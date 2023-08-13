#include "Server.h"

using namespace std;

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 2

Server::Server(unsigned short port)
{
    Port = port;
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
	

        int clientCount = ClientCount.load(); //obtain the current value of the thread-safe counter
        if (clientCount < MAX_CLIENTS)
        {
            /*
				Start new client communication on separate thread
                ---------------------------------------------------
                Since this is a member function that we are spinning off in a 
                thread, we have to use & and this in the thread call
                otherwise, we could just do: thread(HandleClient, clientSocket)
            */

            ClientConnectionData data { clientCount, clientSocket, false };
            ClientThreads.push_back(thread(&Server::HandleClient, this, &data));
            ClientCount.store(clientCount + 1);

            printf("Server: Cancel address - %p\n", &(data.cancelToken));

            //Update server console with connection info
            char clientIpAddress[MAX_IP_LENGTH];
            if (inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIpAddress, MAX_IP_LENGTH) != nullptr)
            {
                cout << "Established Connection with IP Address: " << clientIpAddress << endl;
            }
            cout << "Active Clients: " << ClientCount.load() << endl;

            this_thread::sleep_for(chrono::milliseconds(10000));
            cout << "Server cancelling client thread..";
            data.cancelToken = true;
            break;
        }
        else 
        {
            printf("Max Clients Exceeded. Closing last attempted connection...\n");
            //TODO send response to client about this
            closesocket(clientSocket);
        }
    }

    //Cleanup
    cout << "Waiting for client threads to join" << endl;
    for (int i = 0; i < ClientThreads.size(); i++) {
        ClientThreads[i].join();
    }
   
    cout << "Client thread joined" << endl;
    closesocket(ServerSocket);
    WSACleanup();
}


void Server::HandleClient(ClientConnectionData *data)
{
    //Increase the count of clients in use
	char exitCommand[] = "exit";
    int threadId = data->threadId;
    SOCKET clientSocket = data->clientSocket;
    int monitorResult;
	char buffer[1024];

    cout << "Client is running on thread: #" << threadId << endl;

	while (data->cancelToken == false)
	{
        fd_set readset; //data structure that represents a set of socket descriptors (array of integers)
        FD_ZERO(&readset); //clear out the set of sockets
        FD_SET(clientSocket, &readset); //add specific socket to to the set

        timeval timeout;
        timeout.tv_sec = 1; 
        timeout.tv_usec = 0;
		//monitor socket for 1 second
        monitorResult = select(0, &readset, nullptr, nullptr, &timeout);

        if (monitorResult == SOCKET_ERROR)
        {
            cout << "Error occured while waiting for socket select monitoring" << endl;
            return;
        }
        else if (monitorResult != RESULT_TIMEOUT) //a non-timeout event hit the socket
        {
            if (FD_ISSET(clientSocket, &readset))
            {
                int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

                if (bytesReceived <= SOCKET_ERROR)
                {
                    cout << "Unable to read data from client" << endl;
                }
                else
                {
                    cout << "Message Received" << endl;
                    cout << "Length: " << strlen(buffer) << endl;
                    cout << "Message: " << buffer << endl;
                    cout << endl << endl;
                }
            }
        }
        else {
            if (data->cancelToken == true)
            {
                cout << "Client #" << threadId << " -> thread has been cancelled..." << endl;
                this_thread::sleep_for(chrono::milliseconds(2000));
            }
        }
	}

    int currentCount = ClientCount.load();
    ClientCount.store(currentCount - 1);
}
