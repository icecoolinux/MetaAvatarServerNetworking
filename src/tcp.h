#ifndef _tcp_udp_h_
#define _tcp_udp_h_

#include <stdio.h>

#include "config.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <list>
#include "packet.h"
#include "time.h"
#include "semaphore.h"

using namespace std;

// Represent a conection
class Connection {

	public:

		Connection(int sock);

		// Socket.
		int sock;

		// The first two bytes to know the packet len
		unsigned char header[BYTES_LEN_HEADER];
		int lenHeader;

		// Received data that it doesnt complete a package
		Packet* packet;
};

class TCP{

	private:

		Semaphore* sem;

		// All the active connections.
		list<Connection*> connections;

		Connection* getConnection(int sock);

		static TCP* instance;
		TCP();

	public:

		static TCP* getInstance();
		~TCP();


		int mountServer(int port, int maxClients);

		// Accept a client and return an descriptor.
		// Return -1 on error.
		// If exit is NULL and there's no new clients then return -2.
		// On ip return the ip of the client.
		// The variable exit indicate when the function must return.
		int acceptClient(int sockServer, char* ip, bool* exit);

		int connectToServer(char* hostname, int port);

		// Return 1 if it there's a packet, 0 if there's not packet
		// Return -1 if there's no connection.
		int receive(int sock, Packet* &p);

		// Return true if there's connection.
		bool sendPacket(int sock, Packet* p, bool* exit);

		void closeSock(int sock);
};

#endif
