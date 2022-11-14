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

using namespace std;

class User
{
	private:
		int sock;
		char username[MAX_USERNAME+1];
		Room* room;

		bool mustExit;
		bool exited;

		TCP* tcp;

		Semaphore* semToSend;
		list<Packet*> tosend;

		static void* run_thread(void* user);

	public:

		User(int sock);
		~User();

		void getName(char* username);

		void setRoom(Room* room);
		void clearRoom();
		Room* getRoom();

		void setMustExit();
		bool isExited();

		void sendPacket(Packet* p);
		void newUser(char* name);
		void leaveUser(char* name);

		void run();

};

#endif
