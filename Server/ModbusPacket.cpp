#include "ModbusPacket.h"

void ModbusPacket::SetHostByteOrder()
{
	transaction_id = ntohs(transaction_id);
	protocol_id = ntohs(protocol_id);
	message_length = ntohs(message_length);

	byte_order = LSB;
}

void ModbusPacket::SetNetworkByteOrder()
{
	transaction_id = htons(transaction_id);
	protocol_id = htons(protocol_id);
	message_length = htons(message_length);

	byte_order = MSB;
}

ModbusPacket ModbusPacket::Deserialize(const char* in_buffer)
{
	ModbusPacket request;

	//copy over header info
	request.transaction_id = ntohs(MAKEWORD(in_buffer[TID_HI], in_buffer[TID_LO]));
	request.protocol_id = ntohs(MAKEWORD(in_buffer[PID_HI], in_buffer[PID_LO]));
	request.message_length = ntohs(MAKEWORD(in_buffer[LEN_HI], in_buffer[LEN_LO]));
	request.unit_id = in_buffer[UID];
	request.function = in_buffer[FCODE];

	//allocate and copy over data section
	int data_size = request.GetSizeOfDataSection();
	request.data = new byte[data_size];
	memcpy(request.data, in_buffer + INDEX_OF_FIRST_DATA_BYTE, data_size);

	request.byte_order = LSB; //This is default, but explicitly noting this

	return request;
}

byte* ModbusPacket::Serialize(const ModbusPacket& in_packet)
{
	int base_size = HEADER_LENGTH + 1;
	int data_size = in_packet.GetSizeOfDataSection();
	int total_size = base_size + data_size;

	byte* serialized_buffer = new byte[total_size];

	if (serialized_buffer != nullptr)
	{
		memcpy(serialized_buffer, &in_packet, HEADER_LENGTH);
		memcpy(serialized_buffer + INDEX_OF_FIRST_DATA_BYTE, in_packet.data, data_size);
	}

	return serialized_buffer;
}

unsigned short ModbusPacket::GetRequestStartAddress() const
{
	unsigned short address = ntohs(MAKEWORD(data[RQ_ADDR_HI], data[RQ_ADDR_LO]));

	return address;
}

unsigned short ModbusPacket::GetRequestSize() const
{
	unsigned short size = ntohs(MAKEWORD(data[RQ_SIZE_HI], data[RQ_SIZE_LO]));
	return size;
}

unsigned short ModbusPacket::GetRequestWriteValue()
{
	unsigned short value = ntohs(MAKEWORD(data[RQ_WRITE_VALUE_HI], data[RQ_WRITE_VALUE_LO]));

	return value;
}

int ModbusPacket::GetPacketSize() const
{
	int t_size = sizeof(transaction_id);
	int p_size = sizeof(protocol_id);
	int m_size = sizeof(message_length);
	int data_size = GetSizeOfDataSection();

	return t_size + p_size + m_size + data_size;
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


	int data_size = GetSizeOfDataSection();

	for (int i = 0; i < data_size; i++)
	{
		Utils::PrintBinary(data[i]);
		cout << " ";
	}
	cout << endl;
	cout << endl << "===================================" << endl;
}

int ModbusPacket::GetSizeOfDataSection() const
{
	int data_size;

	if (byte_order == MSB)
	{
		//swap byte order for host machine
		data_size = ntohs(message_length) - BASE_MESSAGE_LENGTH;
	}
	else
	{
		data_size = message_length - BASE_MESSAGE_LENGTH;
	}

	return data_size;
}


