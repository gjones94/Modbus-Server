#include "ModbusADU.h"

void ModbusADU::Print()
{
	cout << endl << "=======Request Packet Data=======" << endl;

	cout << "TransactionId: " << TransactionId << endl;

	cout << "ProtocolId: " << ProtocolId << endl;

	cout << "MessageLength: " << MessageLength << endl;

	cout << "UnitId: " << (int) UnitId << endl;

	cout << "Function Code: " << GetFunction(FunctionCode) << endl;

	cout << "Data: " << "TODO" << endl;

	cout << "=================================" << endl;
}

string ModbusADU::GetFunction(uint8_t code)
{
	switch (code)
	{
	case ReadCoilStatus:
		return "Read Coil";
	case ReadInputStatus:
		return "Read Input Status";
	case ReadHoldingRegister:
		return "Read Holding Register";
	case ReadInputRegister:
		return "Read Input Register";
	default:
		return "Unknown Function";
	}
}

void ModbusADU::FixByteOrder()
{
	TransactionId = ntohs(TransactionId);
	ProtocolId = ntohs(ProtocolId);
	MessageLength = ntohs(MessageLength);
}

void ModbusADU::ToBinary(uint16_t value)
{
	for (uint16_t i = 1 << 15; i > 0; i >>= 1)
	{
		if (i & value)
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
	}
	printf("\n");
}
