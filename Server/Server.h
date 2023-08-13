#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>
#include <mutex>

using namespace std;

//Inform compiler to link the winsock2 library
#pragma comment(lib, "ws2_32.lib") 

#pragma once

#define MAX_IP_LENGTH 16

class Server
{
	public:
		Server();
		Server(unsigned short port);
	 	void Start();

	private:
		WSAData WSAData;
		WORD Version;
		SOCKET ServerSocket;

		unsigned short Port;

		int ClientCount;
		mutex Mutex_ClientCount;
		unique_lock<mutex> Lock;

		bool InitializeSocket();
		bool BindSocket();
		void Listen();
};

