#ifndef _handleroom_h_
#define _handleroom_h_

#include <stdio.h>
#include <map>
#include "room.h"
#include "user.h"

using namespace std;

class HandleRoom
{
	private:

		// Keep the rooms for an app name and an id room.
		map<string, map<int,Room*> > rooms;

		Semaphore* sem;

		static HandleRoom* instance;
		HandleRoom();

		void checkEmptyRoom(Room* room);

	public:

		static HandleRoom* getInstance();
		~HandleRoom();

		bool userEnter(User* user, char* appname, int idRoom);
		void userLeave(User* user);
};

#endif
