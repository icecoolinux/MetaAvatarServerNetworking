todo: avatarServer testClient testAvatar testAvatar2

avatarServer: obj/main.o obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o
	g++ -g obj/main.o obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o -o avatarServer -lpthread

testClient: obj/main.o obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o obj/test.o
	g++ -g obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o obj/test.o -o testClient -lpthread

testAvatar: obj/main.o obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o obj/testAvatar.o
	g++ -g obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o obj/testAvatar.o -o testAvatar -lpthread

testAvatar2: obj/main.o obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o obj/testAvatar2.o
	g++ -g obj/handleRoom.o obj/room.o obj/semaphore.o obj/tcp.o obj/time.o obj/user.o obj/users.o obj/packet.o obj/testAvatar2.o -o testAvatar2 -lpthread


obj/main.o: src/main.cpp
	g++ -g -c src/main.cpp -o obj/main.o

obj/test.o: src/test.cpp
	g++ -g -c src/test.cpp -o obj/test.o

obj/testAvatar.o: src/testAvatar.cpp
	g++ -g -c src/testAvatar.cpp -o obj/testAvatar.o

obj/testAvatar2.o: src/testAvatar2.cpp
	g++ -g -c src/testAvatar2.cpp -o obj/testAvatar2.o

obj/handleRoom.o: src/handleRoom.cpp src/handleRoom.h
	g++ -g -c src/handleRoom.cpp -o obj/handleRoom.o

obj/room.o: src/room.cpp src/room.h
	g++ -g -c src/room.cpp -o obj/room.o

obj/semaphore.o: src/semaphore.cpp src/semaphore.h
	g++ -g -c src/semaphore.cpp -o obj/semaphore.o

obj/tcp.o: src/tcp.cpp src/tcp.h
	g++ -g -c src/tcp.cpp -o obj/tcp.o

obj/packet.o: src/packet.cpp src/packet.h
	g++ -g -c src/packet.cpp -o obj/packet.o

obj/time.o: src/time.cpp src/time.h
	g++ -g -c src/time.cpp -o obj/time.o

obj/user.o: src/user.cpp src/user.h
	g++ -g -c src/user.cpp -o obj/user.o

obj/users.o: src/users.cpp src/users.h
	g++ -g -c src/users.cpp -o obj/users.o

clean:
	rm -rf obj/* avatarServer testClient testAvatar testAvatar2
