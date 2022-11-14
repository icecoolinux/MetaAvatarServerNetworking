
#include "packet.h"

list<Packet*> Packet::packets;

Packet::Packet()
{
	len = 0;
}

Packet* Packet::getPacket()
{
	Packet* p = NULL;

	if(packets.size() > 0)
	{
		p = packets.front();
		packets.pop_front();
	}
	else
		p = new Packet();

	return p;
}

void Packet::releasePacket(Packet* p)
{
	p->clear();
	packets.push_back(p);
}

void Packet::clearPackets()
{
	while(packets.size() > 0)
	{
		Packet* p = packets.front();
		packets.pop_front();
		delete p;
	}
}

Packet* Packet::copy()
{
	Packet* p = Packet::getPacket();

	memcpy (p->data, data, sizeof(char) * len );
	p->len = len;

	return p;
}

unsigned char* Packet::getData()
{
	return &(data[0]);
}

void Packet::setLen(int len)
{
	this->len = len;
}

int Packet::getLen()
{
	return len;
}

void Packet::clear()
{
	len = 0;
}
