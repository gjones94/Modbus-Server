#include "ModbusPacket.h"

void ModbusPacket::FixHeaderByteOrder()
{
	TransactionId = ntohs(TransactionId);
	ProtocolId = ntohs(ProtocolId);
	MessageLength = ntohs(MessageLength);
}

uint16_t ModbusPacket::GetStartAddress(bool zeroBasedAddressing)
{
	uint16_t address = ntohs(StartAddress);
	if (zeroBasedAddressing == false)
	{
		
		address -= 1;
	}
	/*
		With zero based addressing client will send 4 to get the 5th value.
		0 1 2 3 [4]
				5th value

		If NOT zero based, client will send 5 to get 5th. But our indexing is still 0 based.
		0 1 2 3 4 [5] - we would get the 6th value, so we need to subtract 1
	*/
	return address;

}

uint16_t ModbusPacket::GetRequestSize()
{
	uint16_t size = ntohs(MAKEWORD(Data[REQ_SIZE_HI], Data[REQ_SIZE_LO]));
	return size;
}

uint16_t ModbusPacket::GetWriteValue()
{
	uint16_t value = ntohs(MAKEWORD(Data[REQ_VALUE_HI], Data[REQ_VALUE_LO]));

	return value;
}

void ModbusPacket::PrintHeader()
{
	cout << endl;
	cout << "=======Request Header Data=======" << endl;
	cout << "TransactionId: " << TransactionId << endl;
	cout << "ProtocolId: " << ProtocolId << endl;
	cout << "MessageLength: " << MessageLength << endl;
	cout << "UnitId: " << (int) UnitId << endl;
	cout << "=================================" << endl;
	cout << endl;

}