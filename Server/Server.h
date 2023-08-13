#pragma once
//Inform compiler to link the winsock2 library
#pragma comment(lib, "ws2_32.lib") 

#include <stdio.h>
#include <WinSock2.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <csignal>
#include <mutex>
#include <Windows.h>
#include <ws2tcpip.h> // Include for inet_ntop

using namespace std;

#define MAX_IP_LENGTH 16
#define SOCKET_WAIT 1
#define RESULT_TIMEOUT 0

typedef unique_lock<mutex> Lock;

typedef struct ClientConnectionData
{
	int threadId;
	SOCKET clientSocket;
	bool cancelToken;

} ClientConnectionData;

class Server
{
	public:
		Server(unsigned short port);
	 	void Start();

	private:
		WSAData WSAData;
		WORD Version;
		SOCKET ServerSocket;
		unsigned short Port;

		vector<thread> ClientThreads;
		ClientConnectionData ClientData;
		atomic<int> ClientCount;

		bool InitializeSocket();
		bool BindSocket();
		void Listen();
		void HandleClient(ClientConnectionData *data);
};

