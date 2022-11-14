#include "handleRoom.h"

HandleRoom* HandleRoom::instance = NULL;

HandleRoom::HandleRoom()
{
	sem = new Semaphore(1);
}

HandleRoom* HandleRoom::getInstance()
{
	if(instance == NULL)
		instance = new HandleRoom();
	return instance;
}

HandleRoom::~HandleRoom()
{
	delete sem;

	// For each app
	while(rooms.size() > 0)
	{
		map<string, map<int, Room*> >::iterator it = rooms.begin();

		// For each room
		while(it->second.size() > 0)
		{
			map<int, Room*>::iterator it2 = it->second.begin();

			// Remove room
			delete it2->second;

			// Erase room from app
			it->second.erase(it2);
		}

		// Erase app
		rooms.erase(it);
	}
}

bool HandleRoom::userEnter(User* user, char* appname, int idRoom)
{
	#ifdef DEBUG
	char name[MAX_USERNAME+1];
	user->getName(name);
	printf("User %s entering to %s:%d\n", name, appname, idRoom);
	#endif

	bool ret = false;
	Room* roomLeaving = NULL;

	sem->P();

	// First check if the user is in a room
	if(user->getRoom() != NULL)
	{
		roomLeaving = user->getRoom();

		// Quit user from the room.
		roomLeaving->userLeave(user);
	}

	// Find the app
	map<string, map<int, Room*> >::iterator itApp = rooms.find(appname);
	Room* room = NULL;

	// The app doesnt exist
	if(itApp == rooms.end())
	{
		// Create the room and the app.
		room = new Room(appname, idRoom);
		map<int, Room*> mapRoom;
		mapRoom[idRoom] = room;
		rooms[appname] = mapRoom;
	}
	// The app exist.
	else
	{
		map<int, Room*>::iterator itRoom = itApp->second.find(idRoom);

		// The id room doesnt exist.
		if(itRoom == itApp->second.end())
		{
			// Create the room.
			room = new Room(appname, idRoom);
			itApp->second[idRoom] = room;
		}
		else
			room = itRoom->second;
	}

	// Add the user.
	if(room != NULL)
	{
		room->userEnter(user);
		ret = true;
	}

	if(roomLeaving != NULL)
		checkEmptyRoom(roomLeaving);

	sem->V();

	return ret;
}

void HandleRoom::userLeave(User* user)
{
	Room* roomLeaving = NULL;

	sem->P();

	// First check if the user is in a room
	if(user->getRoom() != NULL)
	{
		roomLeaving = user->getRoom();

		// Quit user from the room.
		roomLeaving->userLeave(user);
	}

	if(roomLeaving != NULL)
		checkEmptyRoom(roomLeaving);

	sem->V();
}

void HandleRoom::checkEmptyRoom(Room* room)
{
	if(room->amountUsers() == 0)
	{
		// Remove room
		rooms[room->getAppName()].erase(room->getId());

		// Remove app if it's empty too
		if(rooms[room->getAppName()].size() == 0)
			rooms.erase(room->getAppName());

		delete room;
	}
}
