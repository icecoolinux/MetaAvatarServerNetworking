

#include "user.h"

User::User(int sock)
{
	tcp = TCP::getInstance();

	this->idThread = -1;

	this->mustExit = false;
	this->exited = false;

	this->sock = sock;
	strcpy(this->username, "");
	this->room = NULL;

	semToSend = new Semaphore(1);
}

User::~User()
{
	if(this->sock >= 0)
		tcp->closeSock(this->sock);
	delete semToSend;

	while(tosend.size() > 0)
	{
		Packet* p = tosend.front();
		tosend.pop_front();
		Packet::releasePacket(p);
	}
}

void User::getName(char* username)
{
	strcpy(username, this->username);
}

unsigned long long User::getAvatarUserID()
{
	return avatarUserID;
}

void User::setRoom(Room* room)
{
	this->room = room;
}

void User::clearRoom()
{
	room = NULL;
}

Room* User::getRoom()
{
	return room;
}

void User::setMustExit()
{
	this->mustExit = true;
}

bool User::isExited()
{
	return this->exited;
}

void User::run()
{
	pthread_create(&idThread, NULL, &(User::run_thread), this);
	pthread_detach(idThread);
}

void* User::run_thread(void* user_)
{
	User* user = (User*)user_;

	while(!user->mustExit)
	{
		// Get data to send to the user.
		if(user->room != NULL && user->tosend.size() > 0)
		{
			while(true)
			{
				user->semToSend->P();
				Packet* p = user->tosend.front();
				user->tosend.pop_front();
				user->semToSend->V();

				if(!user->tcp->sendPacket(user->sock, p, &user->mustExit))
				{
					Packet::releasePacket(p);
					break;
				}
				Packet::releasePacket(p);

				if(user->tosend.size() == 0)
					break;
			}
		}

		// Get packet.
		Packet* packet;
		int retTCP = user->tcp->receive(user->sock, packet);

		// There's a packet
		if(retTCP == 1)
		{
			unsigned char* buf = packet->getData();
			int len = packet->getLen();
//printf("c %d %d\n", buf[0], len);
			PACKET_TYPE packetType = static_cast<PACKET_TYPE>(buf[0]);

			// Get username
			if(packetType == USERNAME)
			{
				int posBuf = 1;

				// Copy the userid
				user->avatarUserID = User::getNumber(buf, posBuf, len, ';');

				// Copy the username in a security way.
				User::getString(buf, posBuf, len, user->username, MAX_USERNAME, '\0');
			}

			// The user wants to enter a room.
			else if(packetType == ENTER_ROOM)
			{
				if(user->username[0] != '\0')
				{
					int posBuf = 1;

					// First copy the appname in a security way
					char appname[MAX_APP_NAME+1];
					User::getString(buf, posBuf, len, appname, MAX_APP_NAME, ';');

					// Now get the room number
					int idRoom = User::getNumber(buf, posBuf, len, '\0');

					HandleRoom::getInstance()->userEnter(user, appname, idRoom);
				}
			}

			// The user wants to leave from room.
			else if(packetType == LEAVE_ROOM)
			{
				HandleRoom::getInstance()->userLeave(user);
			}

			// It's a packet data.
			else if(packetType == PACKET_DATA)
			{
				if(user->room != NULL)
				{
					user->room->send(user, packet, 255);
				}

				// If it's a postion packet then I extract the position
				//TODO
			}

			Packet::releasePacket(packet);
		}
		// The connection was closed.
		else if(retTCP == -1)
		{
			break;
		}
		else
			Time::sleep(SLEEP_THREAD_USER_READ_SOCK, 0);
	}
printf("%s mefui\n", user->username);
	// Close sock
	user->tcp->closeSock(user->sock);
	user->sock = -1;

	user->exited = true;
}

void User::sendPacket(Packet* p)
{
printf("11\n");
printf("11 %s\n", username);
	if(sock < 0)
		return;

	semToSend->P();
	tosend.push_back(p);
	semToSend->V();
}

void User::newUser(char* name)
{
printf("22\n");
printf("22 %s\n", username);
	if(sock < 0)
		return;

	Packet* p = Packet::getPacket();

	char* buf = (char*)p->getData();
	buf[0] = ENTER_ROOM;

	int posBuf = 1;

	// Put avatar id
	posBuf += sprintf(&(buf[posBuf]), "%llu;", avatarUserID);

	// Put username
	strcpy(&(buf[posBuf]), name);

	p->setLen(posBuf + strlen(name) + 1);

	semToSend->P();
	tosend.push_back(p);
	semToSend->V();
}

void User::leaveUser(char* name)
{
printf("33\n");
printf("33 %s\n", username);
	if(sock < 0)
		return;

	Packet* p = Packet::getPacket();

	char* buf = (char*)p->getData();
	buf[0] = LEAVE_ROOM;
	strcpy(&(buf[1]), name);

	p->setLen(1 + strlen(name)+1);

	semToSend->P();
	tosend.push_back(p);
	semToSend->V();
}


void User::getString(unsigned char* buf, int &posBuf, int LEN_BUF, char* dst, int MAX_DST, char end)
{
	int pos=0;

	while(pos < MAX_DST && posBuf < LEN_BUF)
	{
		// Reach the end of the name.
		if(buf[posBuf] == end)
			break;

		// Check if the string is correct.
		if( !isalnum(buf[posBuf]) && buf[posBuf] != '@' && buf[posBuf] != '.' && buf[posBuf] != ' ')
		{
			pos = 0;
			dst[0] = '\0';
			break;
		}

		dst[pos] = buf[posBuf];

		posBuf++;
		pos++;
	}
	dst[pos] = '\0';

	posBuf++;
}

unsigned long long User::getNumber(unsigned char* buf, int &posBuf, int LEN_BUF, char end)
{
	int pos=0;

	char numberString[100+1];

	while(pos < 100 && posBuf < LEN_BUF)
	{
		// Reach the end of the name.
		if(buf[posBuf] == end)
			break;

		// Check if the name is correct.
		if( !isdigit(buf[posBuf]) )
		{
			pos = 0;
			numberString[0] = '\0';
			break;
		}

		numberString[pos] = buf[posBuf];

		posBuf++;
		pos++;
	}
	numberString[pos] = '\0';

	return strtoull(numberString, NULL, 10);
}
