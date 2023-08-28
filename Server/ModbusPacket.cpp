#include "ModbusPacket.h"

void ModbusPacket::SetNetworkByteOrder()
{
	transaction_id = ntohs(transaction_id);
	protocol_id = ntohs(protocol_id);
	message_length = ntohs(message_length);
}

void ModbusPacket::SetHostByteOrder()
{
	transaction_id = htons(transaction_id);
	protocol_id = htons(protocol_id);
	message_length = htons(message_length);
}

void ModbusPacket::ParseRawRequest(const char* requestData)
{
	//copy over header info
	transaction_id = ntohs(MAKEWORD(requestData[TID_HI], requestData[TID_LO]));
	protocol_id = ntohs(MAKEWORD(requestData[PID_HI], requestData[PID_LO]));
	message_length = ntohs(MAKEWORD(requestData[LEN_HI], requestData[LEN_LO]));
	unit_id = requestData[UID];
	function = requestData[FCODE];
	
	//allocate and copy over data section
	int data_size = message_length - BASE_MESSAGE_LENGTH;
	data = new uint8_t[data_size];
	memcpy(data, requestData + DATA_START, data_size);
}

uint16_t ModbusPacket::GetStartAddress(bool zeroBasedAddressing) const
{
	uint16_t address = ntohs(MAKEWORD(data[RQ_ADDR_HI], data[RQ_ADDR_LO]));

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

uint16_t ModbusPacket::GetRequestSize() const
{
	uint16_t size = ntohs(MAKEWORD(data[RQ_SIZE_HI], data[RQ_SIZE_LO]));
	return size;
}

uint16_t ModbusPacket::GetSingleWriteValue()
{
	uint16_t value = ntohs(MAKEWORD(data[RQ_WRITE_VALUE_HI], data[RQ_WRITE_VALUE_LO]));

	return value;
}

void ModbusPacket::PrintHeader()
{
	cout << endl;
	cout << "=======Request Header Data=======" << endl;
	cout << "TransactionId: " << transaction_id << endl;
	cout << "ProtocolId: " << protocol_id << endl;
	cout << "MessageLength: " << message_length << endl;
	cout << "UnitId: " << (int) unit_id << endl;
	cout << "=================================" << endl;
	cout << endl;
}

