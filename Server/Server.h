#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>

//Inform compiler to link the winsock2 library
#pragma comment(lib, "ws2_32.lib") 

#pragma once

#define MAX_IP_LENGTH 16

class Server
{
	public:
		Server();
		Server(const char *ipAddress, unsigned short port);
	 	void Start();

	private:
		WSAData WSAData;
		WORD Version;
		SOCKET ServerSocket;
		char *IpAddress;
		unsigned short Port;

		bool InitializeSocket();
		bool BindSocket();
		void Listen();
};

