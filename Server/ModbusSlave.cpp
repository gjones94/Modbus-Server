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
	ModbusPacket request = ParseRequest(requestData);
	request.PrintHeader();
	uint16_t startAddress = request.StartAddress;
	uint16_t size = request.GetRequestSize();

	ModbusPacket response;
	response.TransactionId = htons(request.TransactionId);
	response.ProtocolId = htons(request.ProtocolId);
	response.UnitId = request.UnitId;
	response.FunctionCode = request.FunctionCode;

	switch (request.FunctionCode)
	{
		case ReadCoilStatus:
			ReadCoilStatusRegisters(CoilRegisters, startAddress, size, &response);
			break;
		case ReadInputStatus:
			ReadCoilStatusRegisters(StatusRegisters, startAddress, size, &response);
			for (int i = 1; i < (response.Data[0] + 1); i++)
			{
				PrintBinary(response.Data[i]);
			}
			break;
		case ReadHoldingRegister:
			//responseData = Read(HOLDING_REGISTER, &request);
			break;
		case ReadInputRegister:
			//responseData = Read(INPUT_REGISTER, &request);
			break;
		case ForceSingleCoil:
			break;
		case PresetSingleRegister:
			break;
		case ForceMultipleCoils:
			break;
		case PresetMultipleRegisters:
			break;
	}

	response.MessageLength = htons(response.MessageLength);
	/*response.Data[RESP_SIZE] = response_data->data_size;

	PrintBinary(response.Data[0]);
	for (int i = RESP_DATA_START; i < (response_data->data_size + RESP_DATA_START); i++)
	{
		response.Data[i] = response_data->data[i - 1];
		PrintBinary(response.Data[i]);
	}*/
	//memcpy(response.Data + RESPONSE_VALUES, response_data->data, response_data->data_size);
	
	return response;
}

size_t ModbusSlave::GetDataSize(ModbusPacket sendData)
{
	size_t size = sizeof(sendData) + sendData.Data[RESP_SIZE];
	return size;
}

ModbusPacket ModbusSlave::ParseRequest(char* requestData)
{
	ModbusPacket request;
	request.TransactionId = ntohs(MAKEWORD(requestData[TID_HI], requestData[TID_LO]));
	request.ProtocolId = ntohs(MAKEWORD(requestData[PID_HI], requestData[PID_LO]));
	request.MessageLength = ntohs(MAKEWORD(requestData[LEN_HI], requestData[LEN_LO]));
	request.UnitId = requestData[UID];
	request.FunctionCode = requestData[FCODE];
	request.StartAddress = ntohs(MAKEWORD(requestData[ADDR_HI], requestData[ADDR_LO]));

	int requestDataLength = request.MessageLength - sizeof(request.UnitId) - sizeof(request.FunctionCode) - sizeof(request.StartAddress);
	
	request.Data = new uint8_t[requestDataLength];

	memcpy(request.Data, (requestData + DATA), requestDataLength);

	return request;
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

/* TODO: Removing responseData object, and just passing the response in here as a pointer to be filled */
void ModbusSlave::ReadCoilStatusRegisters(bool* registers, uint16_t address, uint16_t size, ModbusPacket *response)
{
	bool boolArray[BYTE_LENGTH];
	bool needsPadding = false;
	int numBytes = Utils::GetByteLengthForData(size, BITS_PER_COIL);
	int currentByte = RESP_DATA_START;

	response->MessageLength = 3 + numBytes;
	response->Data = new uint8_t[(numBytes + 1)]; // 1 is for the first part of data that tells how many data bytes follow;
	response->Data[RESP_SIZE] = numBytes;
	//response->MessageLength = sizeof(response->UnitId) + sizeof(response->FunctionCode) + numBytes + 1; /* 1 is the start of the data that specifies the number of data bytes retrieved*/


	for (int i = address; i < address + size; i += BYTE_LENGTH)
	{
		int numberOfValues = BYTE_LENGTH;

		//if next coil block exceeds total size requested, minimize acquisition to the request size
		if ((i + BYTE_LENGTH) > size)
		{
			numberOfValues = size - i;
			needsPadding = true;
		}

		//store the values from this register block
		memcpy(boolArray, registers + i, numberOfValues);

		if (needsPadding)
		{
			for (int j = numberOfValues; j < BYTE_LENGTH; j++)
			{
				boolArray[j] = false;
			}
		}

		//iterating through array gives us MSB. We need LSB, so we will reverse the array
		Reverse<bool>(boolArray, BYTE_LENGTH);

		if (response->Data != nullptr)
		{
			response->Data[currentByte] = GetByte(boolArray); //convert boolean array to a uint8_t byte
			currentByte++;
		}
	}
}

void ModbusSlave::EnableZeroBasedAddressing(bool enabled)
{
	ZeroBasedAddressing = enabled;
}

/*
	Reverse: Reverse the the order of elements in an array
	====================================================================
	Algorithm
	swapping values beginning and end values until we reach halfway point
	i-> 	<-j
	1 2 3 4 5 6
	====================================================================
*/
template <typename T>
void Reverse(T* array, int size)
{

	for (int i = 0; i < (size / 2); i++)
	{
		int j = size - i - 1;
		T swap = array[j];
		array[j] = array[i];
		array[i] = swap;
	}
}


/*
	GetByte
	====================================================================
	Algorithm
	[F] T T T F T -> (array), start index 0
	 1  0 0 0 0 0 -> IF (array[index] = True)  { OR with 1 bit on far left }
	 0  1 0 0 0 0 -> IF (array[index] = True)  { OR with 1 bit on end shifted right by index count }
	 0  0 1 0 0 0 -> IF (array[index] = True)  { OR with 1 bit on end shifted right by index count }
	 ------------
	 0  1 1 1 0 1 RESULTING BINARY
	====================================================================
*/
uint8_t GetByte(bool* array)
{
	uint8_t byte = 0;

	int farLeftBit = 1 << (BYTE_LENGTH - 1);

	for (int i = 0; i < BYTE_LENGTH; i++) //move right
	{
		if (array[i] == true)
		{
			byte |= (farLeftBit >> i); //shift right
		}
	}

	return byte;
}

template<typename T>
void PrintArray(const T* array, int size)
{
	cout << endl;
	cout << "Array Values";
	cout << "====================================================================" << endl;
	for (int i = 0; i < size; i++)
	{
		cout << "i: " << i << " value: " << (T)array[i] << endl;
	}
	cout << "====================================================================" << endl;
	cout << endl;
}

template <typename T>
void PrintBinary(T value)
{
	cout << endl;
	cout << "Binary" << endl;
	cout << "====================================================================" << endl;
	int dataSize = (sizeof(T) * BYTE_LENGTH);
	for (int i = 1 << (dataSize - 1); i > 0; i >>= 1)
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
	cout << "====================================================================" << endl;
}

