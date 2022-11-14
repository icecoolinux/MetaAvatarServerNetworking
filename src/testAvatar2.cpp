#include <stdio.h>
#include "tcp.h"
#include "time.h"
#include "packet.h"
#include "semaphore.h"
#include <signal.h>

bool haveExit = false;

void ctrlcHandler(int s)
{
	printf("Exiting...\n");
	haveExit = true;
}

enum PacketDataType{
	PACKET_DATA_POSITION=1,
	PACKET_DATA_AVATAR,
	PACKET_DATA_OBJECT_POS_ROT
};

int main(int argc, char** argv)
{
	Semaphore::init("./semfiles");

	// Set Ctrl-C signal.
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = ctrlcHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGTERM, &sigIntHandler, NULL);
	sigaction(SIGQUIT, &sigIntHandler, NULL);

	TCP* tcp = TCP::getInstance();

	int sock = tcp->connectToServer("127.0.0.1", 8888);

	// User enter
	Packet* p = Packet::getPacket();
	char* buf = (char*) p->getData();
	buf[0] = USERNAME;
	strcpy(&(buf[1]), "UserAvatar2");
	p->setLen(1 + strlen("UserAvatar2")+1);
	tcp->sendPacket(sock, p, NULL);

	Time::sleep(100,0);

	// User enter room 1
	buf[0] = ENTER_ROOM;
	strcpy(&(buf[1]), "ShoppingVR;1");
	p->setLen(1 + strlen("ShoppingVR;1")+1);
	tcp->sendPacket(sock, p, NULL);
	Packet::releasePacket(p);

	Time::sleep(100,0);


	// Send avatar info
	p = Packet::getPacket();

	buf = (char*)p->getData();
	buf[0] = PACKET_DATA;
	buf[1] = PACKET_DATA_AVATAR;
	strcpy(&(buf[2]), "UserAvatar2;");

	// Copy the avatar info.
	int pos = 14;
	for(int i=0; i<10000; i++)
	{
		buf[pos] = i%10;
		pos++;
	}

	p->setLen(pos);
	tcp->sendPacket(sock, p, &haveExit);
	Packet::releasePacket(p);

	// Wait to receive the info.
	int times = 0;
	while(true)
	{
		tcp->receive(sock, p);
		if(p != NULL)
		{
			buf = (char*)p->getData();

			// Find the avatar info
			pos = 2;
			while(buf[pos] != ';')
				pos++;
			pos++;

			for(int i=0; i<10000; i++)
			{
				if(buf[pos] != (i%10))
				{
					printf("Error\n");
					break;
				}
				pos++;
			}
			Packet::releasePacket(p);
			break;
		}

		Time::sleep(1,0);
		times++;
	}
	printf("Times: %d\n", times);

	tcp->closeSock(sock);

	delete tcp;
	Packet::clearPackets();
}
