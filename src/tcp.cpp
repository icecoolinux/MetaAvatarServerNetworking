#include "tcp.h"

Connection::Connection(int sock)
{
	this->sock = sock;
	this->lenHeader = 0;
	this->packet = NULL;
}

TCP* TCP::instance = NULL;


TCP* TCP::getInstance(){
	if(instance == NULL)
		instance = new TCP();
	return instance;
}

TCP::TCP()
{
	this->sem = new Semaphore(1);
}

TCP::~TCP()
{
	// Close the connections
	list<Connection*>::iterator it = connections.begin();
	while(it != connections.end())
	{
		close( (*it)->sock);
		it++;
	}

	delete sem;
}


int TCP::mountServer(int port, int maxClients)
{
	int fd;
	struct sockaddr_in server;

	if ((fd=socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		printf("error on socket()\n");
		return -1;
	}

	// As non block.
	fcntl (fd, F_SETFL, O_NONBLOCK);

	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;
	memset(&(server.sin_zero),'\0',8);

	int yes = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		printf("setsockopt\n");
		return -1;
	}

	if(bind(fd,(struct sockaddr*)&server, sizeof(struct sockaddr))==-1)
	{
	  printf("error on bind(): %s\n", strerror(errno));
	  return -1;
	}

	if(listen(fd, maxClients) == -1)
	{
	  printf("error on listen()\n");
	  return -1;
	}

	// Add to connections.
	sem->P();
	connections.push_back( new Connection(fd) );
	sem->V();

	return fd;
}

int TCP::acceptClient(int sockServer, char* ip, bool* exit)
{
	int sock;
	int sin_size = sizeof(struct sockaddr_in);
	struct sockaddr_in client;

	do{
		// Accepts new client.
		sock = accept(sockServer,(struct sockaddr *)&client, (socklen_t *)&sin_size);

		if(exit == NULL && sock == -1)
			return -2;

		// Wants to exit.
		if( (exit != NULL) && (*exit) && (sock == -1) )
			return -1;

		Time::sleep(WAITING_BLOCK_MS, 0);

	}while(sock == -1);

	// As non block
	fcntl (sock, F_SETFL, O_NONBLOCK);

	// Return the client ip.
	strcpy(ip, inet_ntoa(client.sin_addr));

	// Save the connection
	sem->P();
	connections.push_back( new Connection(sock) );
	sem->V();

	// Return the socket descriptor of the new client.
	return sock;
}

int TCP::connectToServer(char* hostname, int port)
{
	int sock, connfd;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("socket creation failed...\n");
        return -1;
    }

	// As non block
	//fcntl (sock, F_SETFL, O_NONBLOCK);

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(hostname);
    servaddr.sin_port = htons(port);

    // connect the client socket to server socket
    if (connect(sock, (struct sockaddr*)&servaddr, sizeof(servaddr) ) != 0)
	{
        printf("connection with the server failed...\n");
        return -1;
    }

	// Save the connection
	sem->P();
	connections.push_back( new Connection(sock) );
	sem->V();

	return sock;
}

// Return 1 if it there's a packet, 0 if there's not packet
// Return -1 if there's no connection.
int TCP::receive(int sock, Packet* &p)
{
	p = NULL;

	// Get connection
	Connection* c = getConnection(sock);

	if(c == NULL)
		return -1;

	// If I didnt read two bytes that define the len packet.
	if(c->lenHeader < BYTES_LEN_HEADER)
	{
		// Read 2 bytes.
		int bytes_read = recv(sock, &c->header[c->lenHeader], BYTES_LEN_HEADER - c->lenHeader, 0);

		// Grow len and position if I read something.
		if(bytes_read > 0)
			c->lenHeader += bytes_read;

		// There's no connection
		else if( bytes_read == 0)
			return -1;

		// I read an error
		else
		{
			// Common error, non-blocking.
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// There's no packet yet.
				return 0;
			}
			// Fatal error.
			else
			{
				return -1;
			}
		}
	}

	// I already have the bytes that define the packet len.
	if(c->lenHeader >= BYTES_LEN_HEADER)
	{
		// Packet len
		int packetLen = 0;
		for(int i=0; i<BYTES_LEN_HEADER; i++)
		{
			packetLen *= 256;
			packetLen += c->header[i];
		}

		// The packet is bigger than it should be, return error.
		if(packetLen > MAX_DATA_RECEVIED_BY_PACKET)
		{
			c->lenHeader = 0;
			return -1;
		}

		// Get a new packet id I dont have one
		if(c->packet == NULL)
			c->packet = Packet::getPacket();

		unsigned char* data = c->packet->getData();
		int len = c->packet->getLen();

		// Read the complete packet.
		int bytes_read = recv(sock, &data[len], packetLen - len, 0);

		// Grow len and position.
		if(bytes_read > 0)
		{
			len += bytes_read;
			c->packet->setLen(len);

			// It already read a complete packet.
			if( packetLen == len )
			{
				// Return the packet and clean.
				p = c->packet;
				c->packet = NULL;
				c->lenHeader = 0;

				return 1;
			}

			// I didnt read a complete packet.
			else
			{
				return 0;
			}
		}

		// There's no connection.
		else if( bytes_read == 0)
		{
			Packet::releasePacket(c->packet);
			c->packet = NULL;
			c->lenHeader = 0;
			return -1;
		}

		// I read an error
		else
		{
			// Common error, non-blocking.
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// There's no packet yet.
				return 0;
			}
			// Fatal error.
			else
			{
				return -1;
			}
		}
	}

	return 0;
}

// Return true if there's connection.
bool TCP::sendPacket(int sock, Packet* p, bool* exit)
{
	int len = p->getLen();
	unsigned char* buf = p->getData();
printf("send %d\n", len);
	// Amount of bytes written.
	// Also the data position in the buf.
	int pos = 0;

	// Number of bytes written.
	int b_sent = 0;

	// Make the two len bytes.
	unsigned char header[2];
	header[0] = len/256;
	header[1] = len%256;

	// Loop while send the two bytes
	while (pos < 2)
	{
		// Write bytes.
		b_sent = send(sock, &header[pos], 2 - pos, 0);

		// If some bytes was written.
		if (b_sent > 0)
		{
			// Grow the amount of bytes written
			pos += b_sent;
		}
		// Not bytes written (sure the buffer is full)
		else if(b_sent == 0)
		{
		}
		// Error
		else
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Common error, non-blocking.
			}
			// Fatal error.
			else
			{
				return false;
			}
			
			/*
			// Check if the socket is closed.
			if (b_sent == 0)
			{
				 // There is no connection.
				 return false;
			}
			// Check the error connection.
			else if(b_sent == -1)
			{
				 // Error.

				 // If wants to exit then return false.
				 if( exit == NULL || *exit )
					 return false;

				 Time::sleep(WAITING_BLOCK_MS, 0);
			}
			*/
		}
		
		// If wants to exit then return false.
		if( exit != NULL && *exit )
			return false;
			
		Time::sleep(WAITING_BLOCK_MS, 0);
	}

	pos = 0;

	// Loop while all the bytes are written.
	while (pos < len)
	{
		// Write bytes.
		b_sent = send(sock, &buf[pos], len - pos, 0);

		// If some bytes was written.
		if (b_sent > 0)
		{
			// Grow the amount of bytes written
			pos += b_sent;
		}
		// Not bytes written (sure the buffer is full)
		else if(b_sent == 0)
		{
		}
		// Error
		else
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				// Common error, non-blocking.
			}
			// Fatal error.
			else
			{
				return false;
			}
			
			/*
		else
		{
			// Check if the socket is closed.
			if (b_sent == 0)
			{
			   // There is no connection.
			   return false;
			}
			// Check the error connection.
			else if(b_sent == -1)
			{
			   // Error.

			   // If wants to exit then return false.
			   if( exit == NULL || *exit )
				   return false;

			   Time::sleep(WAITING_BLOCK_MS, 0);
			}
		}
		*/
		}
		
		// If wants to exit then return false.
		if( exit != NULL && *exit )
			return false;
		
		Time::sleep(WAITING_BLOCK_MS, 0);
	}

	// Return there is connection.
	return true;
}

void TCP::closeSock(int sock)
{
	Connection* c = NULL;

	sem->P();

	// Find the connection.
	list<Connection*>::iterator it = connections.begin();
	while(it != connections.end())
	{
		// Found it.
		if( (*it)->sock == sock )
		{
			c = *it;
			connections.erase(it);
			break;
		}

		it++;
	}

	sem->V();

	// Close it.
	close(sock);

	if(c != NULL)
	{
		if(c->packet != NULL)
			Packet::releasePacket(c->packet);
		delete c;
	}
}

Connection* TCP::getConnection(int sock)
{
	sem->P();

	list<Connection*>::iterator it = connections.begin();
	while(it != connections.end())
	{
		if( (*it)->sock == sock )
		{
			Connection* c = *it;
			sem->V();

			return c;
		}
		it++;
	}

	sem->V();

	return NULL;
}
