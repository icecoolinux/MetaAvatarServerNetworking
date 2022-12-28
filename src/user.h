#ifndef _user_h_
class User;
#define _user_h_

#include "config.h"
#include "semaphore.h"
#include <pthread.h>
#include <string>
#include <list>
#include "room.h"
#include "tcp.h"
#include "handleRoom.h"
#include <cstdlib>

using namespace std;

class User
{
	private:
		pthread_t idThread;

		int sock;
		char username[MAX_USERNAME+1];
		Room* room;

		unsigned long long avatarUserID;

		bool mustExit;
		bool exited;

		TCP* tcp;

		Semaphore* semToSend;
		list<Packet*> tosend;

		static void* run_thread(void* user);

		static void getString(unsigned char* buf, int &posBuf, int LEN_BUF, char* dst, int MAX_DST, char end);
		static unsigned long long getNumber(unsigned char* buf, int &posBuf, int LEN_BUF, char end);

	public:

		User(int sock);
		~User();

		void getName(char* username);
		unsigned long long getAvatarUserID();

		void setRoom(Room* room);
		void clearRoom();
		Room* getRoom();

		void setMustExit();
		bool isExited();

		void sendPacket(Packet* p);
		void newUser(char* name, unsigned long long remoteAvatarUserID);
		void leaveUser(char* name);

		void run();

};

#endif
