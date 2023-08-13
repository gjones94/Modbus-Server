#include "Server.h"

using namespace std;

#define MAX_BUFFER_SIZE 1024
#define MAX_CLIENTS 2

//MUST be defined outside of any member functions since this is a static variable
bool Server::Cancel = false;

Server::Server(unsigned short port)
{
    Port = port;
    ClientCount = 0;
    ServerSocket = INVALID_SOCKET;
    Version = MAKEWORD(0, 0);
};

void Server::Start()
{

    signal(SIGINT, SignalClose);

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

    while (Cancel == false)
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
            ClientThreads.push_back(thread(&Server::HandleClient, this, clientSocket));
            ClientCount.store(clientCount + 1);

            //Update server console with connection info
            char clientIpAddress[MAX_IP_LENGTH];
            if (inet_ntop(AF_INET, &(clientAddress.sin_addr), clientIpAddress, MAX_IP_LENGTH) != nullptr)
            {
                cout << "Established Connection with IP Address: " << clientIpAddress << endl;
            }
            cout << "Active Clients: " << ClientCount.load() << endl;
        }
        else 
        {
            printf("Max Clients Exceeded. Closing last attempted connection...\n");
            //TODO send response to client about this
            closesocket(clientSocket);
        }
    }

    cout << "CANCELLED" << endl;

    //Cleanup
    for (int i = 0; i < ClientThreads.size(); i++)
    {
        ClientThreads[i].join();
    }
   
    closesocket(ServerSocket);
    WSACleanup();
}


void Server::HandleClient(SOCKET clientSocket)
{
    //Increase the count of clients in use
	char exitCommand[] = "exit";

	char buffer[1024];
	while (true)
	{
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

    int currentCount = ClientCount.load();
    ClientCount.store(currentCount - 1);
}

void Server::SignalClose(int signum)
{
    cout << "CTRL-C" << endl;
    Cancel = true;
}
