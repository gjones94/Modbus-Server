#include "Server.h"

using namespace std;

Server::Server()
{
    Port = PORT;
    ClientCount = 0;
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

    int result = bind(ServerSocket, (SOCKADDR*) &serverIPAddress, sizeof(serverIPAddress));
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
    int result = listen(ServerSocket, SOMAXCONN);

    if (result == SOCKET_ERROR)
    {
        cout << "Failed to enable listening on socket" << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return;
    }

	cout << "Listening for incoming connections on port " << Port << " ..." << endl;

    while (true)
    {
        sockaddr_in clientAddress;
        int szClientAddress = sizeof(clientAddress);

        SOCKET clientSocket = accept(ServerSocket, (SOCKADDR*) &clientAddress, &szClientAddress);

        if (clientSocket == INVALID_SOCKET)
        {
            cout << "Failed to accept client request" << endl;
            cout << "Error: " << WSAGetLastError() << endl;
            closesocket(ServerSocket);
            WSACleanup();
            ExitProcess(EXIT_FAILURE);
        }

        int clientCount = ClientCount.load(); //obtain the current value of the thread-safe client counter
        if (clientCount < MAX_CLIENTS)
        {
            /*
				Start new client communication on separate thread
                ---------------------------------------------------
                Since this is a member function that we are spinning off in a 
                thread, we have to use & and this in the thread call
                otherwise, we could just do: thread(HandleClient, clientSocket)
            */
            ClientConnectionData data { clientCount + 1, clientSocket, clientAddress };
            thread new_connection(&Server::HandleClient, this, data);
            new_connection.detach(); //Detach so that the client data doesn't get invalidated on next loop

        }
        else 
        {
            cout << "\nMax Clients Exceeded. Closing last attempted connection...\n";
            //TODO send response to client about this
            closesocket(clientSocket);
        }
    }

    closesocket(ServerSocket);
    WSACleanup();
}


void Server::HandleClient(ClientConnectionData data)
{
    int monitorResult;
	char recvBuffer[1024];
    char sendBuffer[1024];
    char IPAddress[MAX_IP_LENGTH];

    //Increase the count of clients in use
    int clientCount = ClientCount.load();
    ClientCount.store(clientCount + 1);

    strcpy_s(sendBuffer, "CONNECTED");
    send(data.clientSocket, sendBuffer, sizeof(sendBuffer), 0);

    if (inet_ntop(AF_INET, &(data.clientAddress.sin_addr), IPAddress, MAX_IP_LENGTH) != nullptr)
    {
        cout << "\nCONNECTED: Client #" << ClientCount.load() << " - IP address: " << IPAddress << endl;
    }

	while (true)
	{
        fd_set readset; //data structure that represents a set of socket descriptors (array of integers)
        FD_ZERO(&readset); //clear out the set of sockets
        FD_SET(data.clientSocket, &readset); //add specific socket to to the set

        timeval timeout;
        timeout.tv_sec = SOCKET_TIMEOUT; 
        timeout.tv_usec = 0;
        monitorResult = select(0, &readset, nullptr, nullptr, &timeout);

        if (monitorResult == SOCKET_ERROR)
        {
            cout << "\nERROR: Error occured while waiting for socket select monitoring\n";
            return;
        }
        else if (monitorResult == RESULT_TIMEOUT)
        {
            cout << "\nTIMEOUT: Client #" << data.threadId << "\n";
            break;
        }

		if (FD_ISSET(data.clientSocket, &readset)) //socket is ready to read data
		{
			int bytesReceived = recv(data.clientSocket, recvBuffer, sizeof(recvBuffer), 0);

			if (bytesReceived <= SOCKET_ERROR)
			{
				cout << "\nFAILED READ: client #" << data.threadId << endl;
				break;
			}
			else
			{
                if (strcmp(CLIENT_DISCONNECT, recvBuffer) == 0)
                {
                    char response[MAX_BUFFER_SIZE];
                    sprintf_s(response, "DISCONNECT: Closing connection\n");
                    send(data.clientSocket, response, sizeof(response), 0);
                    break;
                }
                else 
                {
					cout << "\nMESSAGE: Client #" << data.threadId << endl;
					cout << "=========================================================\n";
					cout << "Message: " << recvBuffer << "\n";
					cout << "=========================================================\n";

                    char response[MAX_BUFFER_SIZE];
                    sprintf_s(response, "Message Received\n");
                    send(data.clientSocket, response, sizeof(response), 0);
                }
			}
		}
	}
	closesocket(data.clientSocket);

    cout << "\nDISCONNECTED: Client #" << data.threadId << "\n";

	int currentCount = ClientCount.load();
	ClientCount.store(currentCount - 1);
}