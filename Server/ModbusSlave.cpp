#include "ModbusSlave.h"

ModbusSlave::ModbusSlave(int port)
{
	SetPort(port);
}

void ModbusSlave::Start()
{
	BaseServer::Start();
}

ModbusADU ModbusSlave::GenerateResponse(ModbusADU input)
{
	input.FixByteOrder();
	input.Print();
	return input;
}


