#ifndef _room_h_
class Room;
#define _room_h_

#include <stdio.h>
#include <list>
#include "user.h"
#include "semaphore.h"
#include "packet.h"

using namespace std;

class Room{
	private:

		list<User*> users;

		char appname[MAX_APP_NAME];
		int id;

		Semaphore* sem;

	public:
		Room(char* appname, int id);
		~Room();

		char* getAppName();
		int getId();

		int amountUsers();

		void userLeave(User* user);
		void userEnter(User* user);

		void send(User* user, Packet* p, int range);
};

#endif
