#include "Server.h"
#include <string.h>
#include <stdlib.h>
#include <ws2tcpip.h> // Include for inet_ntop
#include <iostream>

using namespace std;


Server::Server()
{

}

Server::Server(const char *ipAddress, unsigned short port)
{
    IpAddress = (char *) malloc(sizeof(char) * MAX_IP_LENGTH);

    if (IpAddress != NULL)
    {
		strcpy_s(IpAddress, MAX_IP_LENGTH, ipAddress);
    }
    else 
    {
        cout << "Failed to Initialize Server " << endl;
    }

    Port = port;
    ServerSocket = INVALID_SOCKET;
    Version = MAKEWORD(0, 0);
};


void Server::Start()
{
    bool success = InitializeSocket();

    if (success)
    {
        success = BindSocket();
    }

    if (success)
    {
		Listen();
    }
}

bool Server::InitializeSocket()
{
    //Version of winsock we want to use (v2.2)

	/*
		MAKEWORD takes two bytes, and smashes them to create a 16 bit word
		(In this case 0x0202)
	*/
    Version = MAKEWORD(2, 2);
    int result = WSAStartup(Version, &WSAData);

    if (result != 0)
    {
        cout << "WSA Startup failed\n";
        cout << "WSA Status: " << WSAData.szSystemStatus << endl;
        return false;
    }

    ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (ServerSocket == INVALID_SOCKET)
    {
        cout << "Socket initialization failed" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return false;
    }

    return true;
}

bool Server::BindSocket()
{
    sockaddr_in serverIPAddress;
    serverIPAddress.sin_family = AF_INET;
    serverIPAddress.sin_addr.s_addr = INADDR_ANY; //Allows connection on EVERY network interface with IP address
    serverIPAddress.sin_port = htons(Port);
    int result = bind(ServerSocket, (SOCKADDR*)& serverIPAddress, sizeof(serverIPAddress));
    if (result == SOCKET_ERROR)
    {
        cout << "Failed to bind socket to IP" << endl;
        cout << "Error: " << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return false;
    }

    return true;
}

void Server::Listen()
{
    char exitCommand[] = "Exit";
    int result = listen(ServerSocket, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        cout << "Failed to enable listening on socket" << endl;
        closesocket(ServerSocket);
        WSACleanup();
    }

    while (true)
    {
        sockaddr_in clientAddress;
        int szClientAddress = sizeof(clientAddress);

        cout << "Listening for incoming connections on port " << Port << " ..." << endl;
        SOCKET clientSocket = accept(ServerSocket, (SOCKADDR*) &clientAddress, &szClientAddress);

        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Failed to accept client request" << endl;
            cout << "Error: " << WSAGetLastError() << endl;
            closesocket(ServerSocket);
            WSACleanup();
            ExitProcess(EXIT_FAILURE);
        }
        else
        {
            char clientIpAddress[MAX_IP_LENGTH];
            if (inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIpAddress, MAX_IP_LENGTH) != nullptr)
            {
                cout << "Connected to Client: " << clientIpAddress << endl;
            }
        }

        cout << "Type \"Exit\" to end session" << endl;
        while (true)
        {
            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= SOCKET_ERROR)
            {
                cout << "Unable to read data from client" << endl;
            }
            else
            {
                if (strcmp(buffer, exitCommand) == 0)
                {
                    cout << "Client closed connection" << endl << endl;
                    closesocket(clientSocket);
                    break;
                }

                cout << "Message Received" << endl;
                cout << "Length: " << strlen(buffer) << endl;
                cout << "Message: " << buffer << endl;
                cout << endl << endl;
            }
        }
    }
   
    closesocket(ServerSocket);
    WSACleanup();
}
