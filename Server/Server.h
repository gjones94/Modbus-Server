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

typedef unique_lock<mutex> Lock;

class Server
{
	public:
		Server(unsigned short port);
	 	void Start();
		static void SignalClose(int);

	private:
		WSAData WSAData;
		WORD Version;
		SOCKET ServerSocket;
		unsigned short Port;

		vector<thread> ClientThreads;
		atomic<int> ClientCount;
		static bool Cancel;

		bool InitializeSocket();
		bool BindSocket();
		void Listen();
		void HandleClient(SOCKET);
};

