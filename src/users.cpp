#include "users.h"

Users* Users::instance = NULL;

Users::Users()
{

}


Users* Users::getInstance()
{
	if(instance == NULL)
		instance = new Users();
	return instance;
}
Users::~Users()
{
	// Set notice to all users that have to exit.
	list<User*>::iterator it = users.begin();
	while(it != users.end())
	{
		User* user = *it;
		user->setMustExit();
		it++;
	}

	// Wait for the users.
	while(users.size() > 0)
	{
		User* user = users.front();
		users.pop_front();

		while(!user->isExited())
			Time::sleep(5,0);

		delete user;
	}
}

void Users::add(User* user)
{
	users.push_back(user);
}

void Users::process()
{
	list<User*> deadUsers;

	// Check alive.
	list<User*>::iterator it = users.begin();
	while(it != users.end())
	{
		User* user = *it;
		if(user->isExited())
			deadUsers.push_back(user);
		it++;
	}

	// Clear the dead users
	while(deadUsers.size() > 0)
	{
		User* user = deadUsers.front();

		// If it's in a room leave first.
		if(user->getRoom() != NULL)
			HandleRoom::getInstance()->userLeave(user);

		users.remove(user);
		deadUsers.pop_front();
		delete user;
	}
}
