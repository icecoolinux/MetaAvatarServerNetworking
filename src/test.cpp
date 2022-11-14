#include <stdio.h>
#include "tcp.h"
#include "time.h"
#include "packet.h"
#include "semaphore.h"

void printPacket(Packet* p, char* iam)
{
	if(p != NULL)
	{
		unsigned char* buf = p->getData();
		if(buf[0] == ENTER_ROOM)
			printf("Soy %s, entro %s\n", iam, &(buf[1]));
		if(buf[0] == LEAVE_ROOM)
			printf("Soy %s, se fue %s\n", iam, &(buf[1]));
		if(buf[0] == PACKET_DATA)
		{
			int len = p->getLen();
			printf("Receiving %d\n", len);
			for(int i=0; i<len-1; i++)
			{
				if(buf[i+1] != i%200)
				{
					printf("%d %d\n", i, buf[i+1]);
					printf("Error\n");
					break;
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	Semaphore::init("./semfiles");

	TCP* tcp = TCP::getInstance();

	int sock1 = tcp->connectToServer("127.0.0.1", 8888);
	int sock2 = tcp->connectToServer("127.0.0.1", 8888);
	int sock3 = tcp->connectToServer("127.0.0.1", 8888);
	int sock4 = tcp->connectToServer("127.0.0.1", 8888);
	int sock5 = tcp->connectToServer("127.0.0.1", 8888);
	int sock6 = tcp->connectToServer("127.0.0.1", 8888);
	int sock7 = tcp->connectToServer("127.0.0.1", 8888);
	int sock8 = tcp->connectToServer("127.0.0.1", 8888);

	// User1 enter
	Packet* p = Packet::getPacket();
	char* buf = (char*) p->getData();
	buf[0] = USERNAME;
	strcpy(&(buf[1]), "User1");
	p->setLen(1 + strlen("User1")+1);
	tcp->sendPacket(sock1, p, NULL);

	// User2 enter
	buf[0] = USERNAME;
	strcpy(&(buf[1]), "User2");
	p->setLen(1 + strlen("User2")+1);
	tcp->sendPacket(sock2, p, NULL);

	// User1 enter room 1
	buf[0] = ENTER_ROOM;
	strcpy(&(buf[1]), "ShoppingVR;1");
	p->setLen(1 + strlen("ShoppingVR;1")+1);
	tcp->sendPacket(sock1, p, NULL);

	// User2 enter room 1
	buf[0] = ENTER_ROOM;
	strcpy(&(buf[1]), "ShoppingVR;1");
	p->setLen(1 + strlen("ShoppingVR;1")+1);
	tcp->sendPacket(sock2, p, NULL);
	Packet::releasePacket(p);

	Time::sleep(100,0);

	// Get User1
	tcp->receive(sock1, p);
	printPacket(p, "User1");
	Packet::releasePacket(p);

	// Get User2
	tcp->receive(sock2, p);
	printPacket(p, "User2");
	Packet::releasePacket(p);

	// User1 leave room 1
	p = Packet::getPacket();
	buf[0] = LEAVE_ROOM;
	p->setLen(1);
	tcp->sendPacket(sock1, p, NULL);
	Packet::releasePacket(p);

	Time::sleep(100,0);

	// Get User2
	tcp->receive(sock2, p);
	printPacket(p, "User2");
	Packet::releasePacket(p);

	// User1 enter room 1
	p = Packet::getPacket();
	buf[0] = ENTER_ROOM;
	strcpy(&(buf[1]), "ShoppingVR;1");
	p->setLen(1 + strlen("ShoppingVR;1")+1);
	tcp->sendPacket(sock1, p, NULL);
	Packet::releasePacket(p);

	Time::sleep(100,0);

	// Get User1
	tcp->receive(sock1, p);
	printPacket(p, "User1");
	Packet::releasePacket(p);

	Time::sleep(100,0);

	// Get User2
	tcp->receive(sock2, p);
	printPacket(p, "User2");
	Packet::releasePacket(p);

	// User 1 send 30000 bytes
	p = Packet::getPacket();
	unsigned char* bufU = p->getData();
	bufU[0] = PACKET_DATA;
	for(int i=0; i<30000; i++)
		bufU[i+1] = i%200;
	p->setLen(1 + 30000);
	tcp->sendPacket(sock1, p, NULL);
	Packet::releasePacket(p);

	Time::sleep(100,0);

	// User2 get the packet
	tcp->receive(sock2, p);
	printPacket(p, "User2");
	Packet::releasePacket(p);


	tcp->closeSock(sock1);
	tcp->closeSock(sock2);
	tcp->closeSock(sock3);
	tcp->closeSock(sock4);
	tcp->closeSock(sock5);
	tcp->closeSock(sock6);
	tcp->closeSock(sock7);
	tcp->closeSock(sock8);

}
