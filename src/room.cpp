#include "room.h"

Room::Room(char* appname, int id)
{
	sem = new Semaphore(1);

	strcpy(this->appname, appname);
	this->id = id;
}

Room::~Room()
{
	delete sem;
}

char* Room::getAppName()
{
	return appname;
}

int Room::getId()
{
	return id;
}

int Room::amountUsers()
{
	return users.size();
}

void Room::userLeave(User* user)
{
	char username[MAX_USERNAME];
	user->getName(username);

	user->clearRoom();

	sem->P();

	users.remove(user);

	list<User*>::iterator it = users.begin();
	while(it != users.end())
	{
		User* other = *it;
		other->leaveUser(username);
		it++;
	}

	sem->V();
}

void Room::userEnter(User* user)
{
	char username[MAX_USERNAME+1];
	user->getName(username);

	char usernameOther[MAX_USERNAME+1];

	user->setRoom(this);

	sem->P();

	list<User*>::iterator it = users.begin();
	while(it != users.end())
	{
		User* other = *it;

		other->getName(usernameOther);

		other->newUser(username);
		user->newUser(usernameOther);

		it++;
	}

	users.push_back(user);

	sem->V();
}


void Room::send(User* user, Packet* p, int range)
{
	sem->P();

	list<User*>::iterator it = users.begin();
	while(it != users.end())
	{
		User* other = *it;

		if(other != user)
		{
			Packet* pCopy = p->copy();
			other->sendPacket(pCopy);
		}

		it++;
	}

	sem->V();
}
