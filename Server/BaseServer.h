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

/* ======CONFIGURABLE======*/
#define PORT 502
#define SOCKET_TIMEOUT 60
#define MAX_CLIENTS 30
#define MAX_DATA_SIZE_BYTES 1024
/* ====END CONFIGURABLE====*/

#define MAX_IP_LENGTH 16
#define RESULT_TIMEOUT 0

typedef struct ClientConnectionData
{
	int threadId;
	SOCKET clientSocket;
	sockaddr_in clientAddress;

} ClientConnectionData;

template <typename T>
class BaseServer
{
	public:
		BaseServer();
		void SetPort(unsigned short port);
	 	void Start();

	private:
		WSAData WSAData;
		WORD Version;
		SOCKET ServerSocket;
		unsigned short Port;

		/// <summary>
		/// ClientId, Socket, IP address
		/// </summary>
		ClientConnectionData ClientData;
		/// <summary>
		/// Keeps track of the number of active clients
		/// </summary>
		atomic<int> ClientCount;

		/// <summary>
		/// Initializes socket using WSAData
		/// </summary>
		/// <returns></returns>
		bool InitializeSocket();

		/// <summary>
		/// Binds socket to host IP address
		/// </summary>
		/// <returns></returns>
		bool BindSocket();

		/// <summary>
		/// Starts listening for client connections on continuous loop
		/// </summary>
		void Listen();

		/// <summary>
		/// Client communication handler that runs on a separate thread
		/// </summary>
		/// <param name="data"></param>
		void HandleClient(ClientConnectionData data);

		bool Send(SOCKET clientSocket, T sendData);

		bool Receive(SOCKET clientSocket, T *receiveData);

		virtual T GetResponse(T clientRequestData);
};

