#ifndef _users_h_
#define _users_h_

#include <stdio.h>
#include <list>
#include "user.h"

using namespace std;

class Users
{
	private:
		list<User*> users;

		static Users* instance;
		Users();

	public:
		static Users* getInstance();
		~Users();

		void add(User* user);
		void process();
};

#endif
