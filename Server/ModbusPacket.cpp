
#include "ModbusPacket.h"

void ModbusPacket::SetNetworkByteOrder()
{
	transaction_id = ntohs(transaction_id);
	protocol_id = ntohs(protocol_id);
	message_length = ntohs(message_length);

	byte_format = LSB;
}

void ModbusPacket::SetHostByteOrder()
{
	transaction_id = htons(transaction_id);
	protocol_id = htons(protocol_id);
	message_length = htons(message_length);

	byte_format = MSB;
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

	byte_format = LSB;
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
 
char* ModbusPacket::Serialize(const ModbusPacket* packet)
{
	//TODO, test and make sure that endianess was maintained upon return
	size_t data_size = ntohs(packet->message_length) - BASE_MESSAGE_LENGTH;

	char* data = new char[HEADER_LENGTH + data_size];

	if (data != nullptr)
	{
		memcpy(data, packet, HEADER_LENGTH);
		memcpy(data + DATA_START, packet->data, data_size);

		packet->PrintPacketBinary();

	/*	cout << endl;
		cout << "TID" << endl;
		Utils::PrintBinary(data[0]);
		cout << " ";
		Utils::PrintBinary(data[1]);

		cout << endl;
		cout << endl;

		cout << "PID" << endl;
		Utils::PrintBinary(data[2]);
		cout << " ";
		Utils::PrintBinary(data[3]);

		cout << endl;
		cout << endl;

		cout << "Message Length" << endl;
		Utils::PrintBinary(data[4]);
		cout << " ";
		Utils::PrintBinary(data[5]);

		cout << endl;
		cout << endl;

		cout << "UID: " << endl;
		Utils::PrintBinary(data[6]);

		cout << endl;
		cout << endl;

		cout << "Function Code:" << endl;
		Utils::PrintBinary(data[7]);
		cout << endl;
		cout << endl;

		cout << "Data" << endl;
		for (int i = DATA_START; i < (HEADER_LENGTH + data_size); i++)
		{
			Utils::PrintBinary(data[i]);
			cout << " ";
		}
		cout << endl;*/

	}

	return (char*) data;
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

void ModbusPacket::PrintPacketBinary() const
{
	cout << "============PACKET DATA============" << endl;
	cout << endl;
	cout << "TID" << endl;
	Utils::PrintBinary(transaction_id);

	cout << endl;
	cout << endl;

	cout << "PID" << endl;
	Utils::PrintBinary(protocol_id);

	cout << endl;
	cout << endl;

	cout << "Message Length" << endl;
	Utils::PrintBinary(message_length);

	cout << endl;
	cout << endl;

	cout << "UID: " << endl;
	Utils::PrintBinary(unit_id);

	cout << endl;
	cout << endl;

	cout << "Function Code:" << endl;
	Utils::PrintBinary(function);
	cout << endl;
	cout << endl;

	cout << "Data" << endl;
	int data_size = ntohs(message_length) - BASE_MESSAGE_LENGTH;
	for (int i = 0; i < data_size; i++)
	{
		Utils::PrintBinary(data[i]);
		cout << " ";
	}
	cout << endl;
	cout << endl << "===================================" << endl;
}


