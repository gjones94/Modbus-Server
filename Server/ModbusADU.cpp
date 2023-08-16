#include "ModbusADU.h"

void ModbusADU::Print(ModbusADU* packet)
{
	cout << "=======Request Packet Data=======" << endl;

	cout << "TransactionId: " << endl;
	cout << "ProtocolId: " << endl;
	cout << "MessageLength: " << endl;
	cout << "UnitId: " << endl;
	cout << "Function Code: " << endl;
	cout << "Data: " << endl;

	cout << "=================================" << endl;
}
