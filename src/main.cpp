#include <stdio.h>
#include <algorithm>
#include <signal.h>
#include "semaphore.h"
#include "config.h"
#include "time.h"
#include "tcp.h"
#include "users.h"
#include "handleRoom.h"

bool haveExit = false;

void ctrlcHandler(int s)
{
	printf("Exiting...\n");
	haveExit = true;
}

void checkUsersAlive();
void addNewUser(int sock);

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		printf("Use: %s <port>\n", argv[0]);
		return 0;
	}

	Semaphore::init("./semfiles");

	int port = atoi(argv[1]);

	// Set Ctrl-C signal.
	struct sigaction sigIntHandler;
	sigIntHandler.sa_handler = ctrlcHandler;
	sigemptyset(&sigIntHandler.sa_mask);
	sigIntHandler.sa_flags = 0;
	sigaction(SIGINT, &sigIntHandler, NULL);
	sigaction(SIGTERM, &sigIntHandler, NULL);
	sigaction(SIGQUIT, &sigIntHandler, NULL);

	// Ignore the SIGPIPE when write a closed socket
	signal(SIGPIPE, SIG_IGN);


	TCP* tcp = TCP::getInstance();
	int serverSock = tcp->mountServer(port, MAX_CLIENTS);

	Users* users = Users::getInstance();

	// Main loop
	while(!haveExit)
	{
		char ip[50];
		int sockClient = tcp->acceptClient(serverSock, ip, NULL);

		if(sockClient >= 0)
			addNewUser(sockClient);

		users->process();

		Time::sleep(SLEEP_THREAD_ACCEPT_USERS, 0);
	}

	tcp->closeSock(serverSock);
	delete users;
	delete HandleRoom::getInstance();
	delete tcp;
	Packet::clearPackets();
}


void addNewUser(int sock)
{
	User* user = new User(sock);
	user->run();
	Users::getInstance()->add(user);
}
