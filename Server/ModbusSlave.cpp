#include "ModbusSlave.h"
#include "Utils.h"

ModbusSlave::ModbusSlave(int port)
{
	SetPort(port);

	InitializeMemory();
	EnableZeroBasedAddressing(true);
}

void ModbusSlave::InitializeMemory()
{
	//TODO use constructors instead
	CoilRegisters = (bool*) calloc(DATA_BLOCK_SIZE * BITS_PER_REG, sizeof(bool));
	StatusRegisters = (bool*) calloc(DATA_BLOCK_SIZE * BITS_PER_REG, sizeof(bool));
	InputRegisters = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
	HoldingRegisters = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));

	StatusRegisters[0] = true;
	StatusRegisters[1] = true;
	StatusRegisters[2] = true;
	StatusRegisters[3] = true;

	StatusRegisters[4] = false;
	StatusRegisters[5] = true;
	StatusRegisters[6] = false;
	StatusRegisters[7] = true;

	StatusRegisters[8] = false;
	StatusRegisters[9] = true;
	StatusRegisters[10] = false;
	StatusRegisters[11] = true;

	StatusRegisters[12] = false;
	StatusRegisters[13] = true;
	StatusRegisters[14] = false;
	StatusRegisters[15] = true;

	StatusRegisters[16] = true;
	StatusRegisters[17] = true;
	StatusRegisters[18] = true;
	StatusRegisters[19] = false;
}

void ModbusSlave::Start()
{
	BaseServer::Start();
}

/*
	Response
	====================================================================
	1) MBAP Header
	2) Function Code
	3) Data Requested (Functions 1-4) // Data Value Written (Functions 5, 6) // # Written (Functions 15, 16)
	====================================================================
*/
ModbusPacket ModbusSlave::GetResponse(char *requestData)
{
	ModbusPacket request;
	request.ParseRawRequest(requestData);

	ModbusPacket response;

	switch (request.function)
	{
		case READ_COILS:
			response = ReadCoilStatusRegisters(CoilRegisters, request);
			break;
		case READ_INPUTS:
			response = ReadCoilStatusRegisters(StatusRegisters, request);
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
	}

	response.transaction_id = htons(request.transaction_id);
	response.protocol_id = htons(request.protocol_id);
	response.unit_id = request.unit_id;
	response.function = request.function;

	//for (int i = 0; i < response.message_length - 2; i++)
	//{
	//	Utils::PrintBinary<uint8_t>(response.data[i]);
	//}

	response.message_length = htons(response.message_length);
	
	return response;
}

bool ModbusSlave::Send(SOCKET socket, const ModbusPacket* sendData)
{
	char* serializedData = ModbusPacket::Serialize(sendData);

	return true;
}

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
ModbusPacket ModbusSlave::ReadCoilStatusRegisters(bool* registers, const ModbusPacket& request)
{

	int start_address = request.GetStartAddress(ZeroBasedAddressing);
	int request_size = request.GetRequestSize(); 

	ModbusPacket response;

	int byte_count = Utils::NumBytesNeeded(request_size, BITS_PER_COIL);

	int data_block_size = RES_READ_INFO_SZ + byte_count;
	response.data = new uint8_t[data_block_size];
	response.data[RES_READ_BYTE_COUNT] = byte_count;
	response.message_length = BASE_MESSAGE_LENGTH + data_block_size;

	bool boolArray[SIZE_OF_BYTE];
	bool needsPadding = false;
	int current_byte = 1;
	int end_address = start_address + request_size;

	for (int i = start_address; i < end_address; i += SIZE_OF_BYTE)
	{
		int numberOfValues = SIZE_OF_BYTE;

		//if next coil block exceeds total size requested, minimize acquisition to the request size
		if ((i + SIZE_OF_BYTE) > end_address)
		{
			numberOfValues = end_address - i;
			needsPadding = true;
		}

		//store the values from this register block
		memcpy(boolArray, registers + i, numberOfValues);

		//add 0/false padding to remainder of byte if needed
		if (needsPadding)
		{
			for (int j = numberOfValues; j < SIZE_OF_BYTE; j++)
			{
				boolArray[j] = false;
			}
		}

		//iterating through array gives us MSB. We need LSB, so we will reverse the array
		Utils::Reverse<bool>(boolArray, SIZE_OF_BYTE);

		if (response.data != nullptr)
		{
			response.data[current_byte] = Utils::GetByte(boolArray); 
			current_byte++;
		}
	}

	return response;
}

void ModbusSlave::EnableZeroBasedAddressing(bool enabled)
{
	ZeroBasedAddressing = enabled;
}