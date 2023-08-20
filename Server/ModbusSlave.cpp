#include "ModbusSlave.h"

ModbusSlave::ModbusSlave(int port)
{
	SetPort(port);

	InitializeMemory();
	EnableZeroBasedAddressing(true);
}

void ModbusSlave::InitializeMemory()
{
	CoilRegisters = (bool*) calloc(DATA_BLOCK_SIZE * BITS_PER_REG, sizeof(bool));
	StatusRegisters = (bool*) calloc(DATA_BLOCK_SIZE * BITS_PER_REG, sizeof(bool));

	StatusRegisters[0] = true;
	StatusRegisters[1] = true;
	StatusRegisters[2] = true;
	StatusRegisters[3] = true;

	StatusRegisters[4] = false;
	StatusRegisters[5] = true;
	StatusRegisters[6] = false;
	StatusRegisters[7] = true;

	InputRegisters = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
	HoldingRegisters = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
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
ModbusPacket ModbusSlave::GetResponse(ModbusPacket request)
{
	request.FixHeaderByteOrder();
	PrintHeader(request);

	ModbusPacket response;
	ResponseData response_data{};

	uint16_t size = GetSizeRequested(request.Data);
	uint16_t address = GetStartAddress(request.Data);

	switch (request.FunctionCode)
	{
		case ReadCoilStatus:
			response_data = ReadCoilStatusRegisters(CoilRegisters, address, size);
			break;
		case ReadInputStatus:
			response_data = ReadCoilStatusRegisters(StatusRegisters, address, size);
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

	response_data.response_code = (response_data.response_code == BAD) ? request.FunctionCode | ERROR_FLAG : request.FunctionCode;
	response.TransactionId = htons(request.TransactionId);
	response.ProtocolId = htons(request.ProtocolId);
	response.MessageLength = htons(2 + response_data.data_size);
	response.UnitId = request.UnitId;
	response.FunctionCode = response_data.response_code;
	response.Data[RESPONSE_SIZE] = response_data.data_size;
	memcpy((response.Data + RESPONSE_VALUES), response_data.data, response_data.data_size);
	
	return response;
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
ResponseData ModbusSlave::ReadCoilStatusRegisters(bool* registers, uint16_t address, uint16_t size)
{
	ResponseData response;
	bool boolArray[BYTE_LENGTH];
	bool needsPadding = false;
	int bytesNeeded = (uint8_t)ceil((double)size / BYTE_LENGTH); //round up # bytes needed. If 17 bits, need 3 bytes (24 bits)

	response.data_size = bytesNeeded;
	response.data = new uint8_t[response.data_size];

	for (int i = address; i < address + size; i ++)
	{
		if (boolArray != nullptr)
		{
			int numberOfValues = BYTE_LENGTH;

			//if next block exceeds total size requested, minimize acquisition to the request size
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

			if (response.data != nullptr)
			{
				response.data[i] = GetByte(boolArray); //convert boolean array to a uint8_t byte
			}
		}
		else
		{
			cout << "Failed to allocate memory for data acquisition" << endl;
		}
	}

	response.response_code = GOOD;

	return response;
}

bool ModbusSlave::Success(uint8_t functionCode)
{
	bool success = (functionCode & ERROR) == 0;
	return success;
}

void ModbusSlave::EnableZeroBasedAddressing(bool enabled)
{
	ZeroBasedAddressing = enabled;
}

uint16_t ModbusSlave::GetStartAddress(uint8_t *requestData)
{
	uint16_t address = ntohs(MAKEWORD(requestData[ADDR_HI], requestData[ADDR_LO])); //smash registers and reverse MSB to LSB

	if (ZeroBasedAddressing == false)
	{
		address -= 1;
	}

	return address;
}

uint16_t ModbusSlave::GetSizeRequested(uint8_t* requestData)
{
	return ntohs(MAKEWORD(requestData[SIZE_HI], requestData[SIZE_LO]));
}

void ModbusSlave::PrintHeader(ModbusPacket packet)
{
	cout << endl << "=======Request Header Data=======" << endl;
	cout << "TransactionId: " << packet.TransactionId << endl;
	cout << "ProtocolId: " << packet.ProtocolId << endl;
	cout << "MessageLength: " << packet.MessageLength << endl;
	cout << "UnitId: " << (int) packet.UnitId << endl;
	cout << "=================================" << endl << endl;

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

