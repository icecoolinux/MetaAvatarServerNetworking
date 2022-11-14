#ifndef _packet_h_
#define _packet_h_

#include <stdio.h>
#include <string.h>
#include <list>
#include "config.h"

using namespace std;

enum PACKET_TYPE
{
	USERNAME = 1,
	ENTER_ROOM,
	LEAVE_ROOM,
	PACKET_DATA
};

class Packet
{
	private:
		int len;
		unsigned char data[MAX_DATA_RECEVIED_BY_PACKET];

		static list<Packet*> packets;
	public:

		Packet();

		static Packet* getPacket();
		static void releasePacket(Packet* p);
		static void clearPackets();

		Packet* copy();

		unsigned char* getData();
		int getLen();
		void setLen(int len);

		void clear();
};

#endif
