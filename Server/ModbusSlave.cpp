#include "ModbusSlave.h"

ModbusSlave::ModbusSlave(int port)
{
	SetPort(port);
}

void ModbusSlave::Start()
{
	BaseServer::Start();
}



