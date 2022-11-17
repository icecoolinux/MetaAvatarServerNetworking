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
	strcpy(&(buf[1]), "UserAvatar");
	p->setLen(1 + strlen("UserAvatar")+1);
	tcp->sendPacket(sock, p, NULL);

	Time::sleep(100,0);

	// User enter room 1
	buf[0] = ENTER_ROOM;
	strcpy(&(buf[1]), "ShoppingVR;1");
	p->setLen(1 + strlen("ShoppingVR;1")+1);
	tcp->sendPacket(sock, p, NULL);
	Packet::releasePacket(p);

	Time::sleep(100,0);

	while(!haveExit)
	{
		// Receive packet
		tcp->receive(sock, p);

		if(p != NULL)
		{
			buf = (char*)p->getData();
			if(buf[0] == PACKET_DATA)
			{
				// It's avatar packet.
				if(buf[1] == PACKET_DATA_AVATAR)
				{
					// Find the info data avater, after the ';' character.
					int pos = 2;
					while(buf[pos] != ';')
						pos++;
					pos++;

					// Send the avatar as me
					Packet* p2 = Packet::getPacket();

					unsigned char* buf2 = p2->getData();
					buf2[0] = PACKET_DATA;
					buf2[1] = PACKET_DATA_AVATAR;
					strcpy((char*)&(buf2[2]), "UserAvatar;");

					// Copy the avatar info.
					int pos2 = 13;

					while(pos < p->getLen())
					{
						buf2[pos2] = buf[pos];
						pos++;
						pos2++;
					}

					p2->setLen(pos2);

					tcp->sendPacket(sock, p2, &haveExit);

					Packet::releasePacket(p2);
				}
			}

			Packet::releasePacket(p);
		}
	}

	tcp->closeSock(sock);

	delete tcp;
	Packet::clearPackets();
}
