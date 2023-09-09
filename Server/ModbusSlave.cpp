#include "ModbusSlave.h"
#include "Utils.h"

ModbusSlave::ModbusSlave(int port)
{
	SetPort(port);

	InitializeRegisters();
	EnableZeroBasedAddressing(true);
}

void ModbusSlave::InitializeRegisters()
{
	coil_registers = new bool[MODBUS_REGISTER_CAPACITY];
	status_registers = new bool[MODBUS_REGISTER_CAPACITY];
	input_registers = new unsigned short[MODBUS_REGISTER_CAPACITY];
	holding_registers = new unsigned short[MODBUS_REGISTER_CAPACITY];

	memset(coil_registers, 0, MODBUS_REGISTER_CAPACITY);
	memset(status_registers, 0, MODBUS_REGISTER_CAPACITY);
	memset(input_registers, 0, MODBUS_REGISTER_CAPACITY);
	memset(holding_registers, 0, MODBUS_REGISTER_CAPACITY);

	status_registers[0] = true;
	status_registers[1] = true;
	status_registers[2] = true;
	status_registers[3] = true;
	status_registers[4] = true;
	status_registers[5] = true;
	status_registers[6] = true;
	status_registers[7] = true;

	status_registers[8] = false;
	status_registers[9] = false;
	status_registers[10] = false;
	status_registers[11] = false;
	status_registers[12] = false;
	status_registers[13] = false;
	status_registers[14] = false;
	status_registers[15] = false;

	status_registers[16] = true;
	status_registers[17] = true;
	status_registers[18] = true;
	status_registers[19] = true;
	status_registers[19] = true;
}

void ModbusSlave::Start()
{
	BaseServer::Start();
}

bool ModbusSlave::ReceiveAndRespond(SOCKET socket)
{
	char buffer[BUFFER_SIZE];

	int bytesReceived = recv(socket, (char*)buffer, BUFFER_SIZE, 0);

	if (bytesReceived <= 0)
	{
		return false;
	}

	ModbusPacket request = ModbusPacket::Deserialize(buffer);

	ModbusPacket response = GetResponse(request);

	response.SetNetworkByteOrder();
	int serialize_buffer_sz = 0;
	char *serialized_buffer = ModbusPacket::Serialize(response, &serialize_buffer_sz);

	int bytes_sent = send(socket, serialized_buffer, serialize_buffer_sz, 0);

	if (bytes_sent <= 0)
	{
		return false;
	}

	return true;
}

/*
	Response
	====================================================================
	1) MBAP Header
	2) Function Code
	3) Data Requested (Functions 1-4) // Data Value Written (Functions 5, 6) // # Written (Functions 15, 16)
	====================================================================
*/
ModbusPacket ModbusSlave::GetResponse(const ModbusPacket request)
{
	ModbusPacket response;

	switch (request.function)
	{
		case READ_COILS:
			response = ReadCoilStatusRegisters(coil_registers, request);
			break;
		case READ_INPUTS:
			response = ReadCoilStatusRegisters(status_registers, request);
			break;
		case READ_HOLDING_REGISTERS:
			//responseData = Read(HOLDING_REGISTER, &request);
			break;
		case READ_INPUT_REGISTERS:
			//responseData = Read(INPUT_REGISTER, &request);
			break;
		case WRITE_SINGLE_COIL:
			break;
		case WRITE_SINGLE_REGISTER:
			break;
		case WRITE_MULTIPLE_COILS:
			break;
		case WRITE_MULTIPLE_REGISTERS:
			break;
		default:
			//TODO Exception code 01 Unsupported function
			break;
	}

	return response;
}

//bool ModbusSlave::Send(SOCKET socket, const ModbusPacket* sendData)
//{
//	char* serializedData = ModbusPacket::Serialize(sendData);
//
//	return true;
//}

size_t ModbusSlave::GetSendBufferSize(const ModbusPacket* sendData)
{
	size_t t_size = sizeof(sendData->transaction_id);
	size_t p_size = sizeof(sendData->protocol_id);
	size_t l_size = sizeof(sendData->message_length);
	size_t m_size = ntohs(sendData->message_length);

	return t_size + p_size + l_size + m_size;
}

/*
	Read the Coil or Input Status Registers
	
	Response:
	====================================================================
	Example:
	We request 20 bits

	1) Function Code (If Success)
	2) # Bytes to follow (containing the data acquired)
    3) Bytes Read **(See note below)
		Byte 1: 0110 000(1) <- First Address
		Byte 2: 1010 0101
		Byte 3: 0000 1001 //pad with 0s after 20th bit finished

	** The value of the coil/status at starting address in the request
	     Is to be stored in the LSB (far right). The last bit will be stored
		 in the MSB (Far left) and remaining space not filled in the byte
		 will be padded with zeros
	====================================================================
*/
ModbusPacket ModbusSlave::ReadCoilStatusRegisters(bool* register_data, const ModbusPacket& request)
{
	ModbusPacket response;

	//Populate response header
	response.transaction_id = request.transaction_id;
	response.protocol_id = request.protocol_id;
	response.unit_id = request.unit_id;
	response.function = request.function;

	//Validate Request
	byte result = ValidateRequest(request);
	if (result == OK)
	{
		//Obtain requested data
		unsigned short start_address = GetRequestStartAddress(request);
		unsigned short request_size = request.GetRequestSize();

		int byte_count = Utils::GetNumBytesRequiredForData(request_size, BITS_PER_COIL);
		int data_block_size = RES_READ_INFO_SZ + byte_count;
		response.data = new byte[data_block_size];

		if (response.data != nullptr) 
		{
			response.data[RES_READ_BYTE_COUNT] = byte_count;
			response.message_length = BASE_MESSAGE_LENGTH + data_block_size;

			bool bool_array[SIZE_OF_BYTE];
			bool current_byte_needs_padding = false;
			int current_byte = 1;
			int end_address = start_address + request_size;

			for (int current_address = start_address; current_address < end_address; current_address += SIZE_OF_BYTE)
			{
				int num_coils = SIZE_OF_BYTE;

				//if next coil block exceeds total size requested, minimize acquisition to the request size
				if ((current_address + SIZE_OF_BYTE) > end_address)
				{
					num_coils = end_address - current_address;
					current_byte_needs_padding = true;
				}

				//store the values from this register block
				memcpy(bool_array, register_data + current_address, num_coils);

				//add 0/false padding to remainder of byte if needed
				if (current_byte_needs_padding)
				{
					for (int padding_index = num_coils; padding_index < SIZE_OF_BYTE; padding_index++)
					{
						bool_array[padding_index] = false;
					}
				}

				//iterating through registers gives us MSB. We need LSB, so we will reverse the array
				Utils::Reverse<bool>(bool_array, SIZE_OF_BYTE);

				if (current_byte <= (data_block_size - 1))
				{
					response.data[current_byte] = Utils::GetByte(bool_array);
				}

				current_byte++;
			}
		}
		else //out of memory
		{
			SetException(response, SLAVE_DEVICE_FAILURE);
		}
	}
	else 
	{
		SetException(response, result);
	}

	return response;
}

byte ModbusSlave::ValidateRequest(ModbusPacket request)
{
	//Check for Illegal address
	unsigned short startAddress = GetRequestStartAddress(request);
	unsigned short size = request.GetRequestSize();

	if (startAddress + size > MODBUS_REGISTER_CAPACITY)
	{
		return ILLEGAL_ADDRESS;
	}

	//check for illegal data value
	//TODO MORE Validation

	return OK;
}

void ModbusSlave::SetException(ModbusPacket &response, byte result)
{
	response.message_length = BASE_MESSAGE_LENGTH + EX_INFO_SZ;
	response.function |= BAD;
	response.data = new byte[EX_INFO_SZ];
	response.data[EXCEPTION_CODE] = result;
}

void ModbusSlave::EnableZeroBasedAddressing(bool enabled)
{
	ZeroBasedAddressing = enabled;
}

unsigned short ModbusSlave::GetRequestStartAddress(const ModbusPacket& request)
{
	/*
	With zero based addressing client will send 4 to get the 5th value.
	0 1 2 3 [4]
			5th value

	If NOT zero based, client will send 5 to get 5th. But our data indexing is still 0 based.
	0 1 2 3 4 [5] - we would get the 6th value, so we need to subtract 1
	*/
	unsigned short start_address = request.GetRequestStartAddress();

	if (ZeroBasedAddressing == false)
	{
		start_address -= 1;
	}

	return start_address;
}