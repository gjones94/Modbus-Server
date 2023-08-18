#include "ModbusSlave.h"

ModbusSlave::ModbusSlave(int port)
{
	SetPort(port);

	InitializeMemory();
	EnableZeroBasedAddressing(true);
}

void ModbusSlave::InitializeMemory()
{
	Coils = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
	InputStatuses = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
	InputRegisters = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
	HoldingRegisters = (uint16_t*) calloc(DATA_BLOCK_SIZE, sizeof(uint16_t));
	cout << "Memory initialized" << endl;
}

void ModbusSlave::Start()
{
	BaseServer::Start();
}

ModbusADU ModbusSlave::GenerateResponse(ModbusADU requestPacket)
{
	/*
		Response:
		1) MBAP Header
		2) Function Code
		3) Data Requested (1-4) // Data Written (5, 6) // #Written (15, 16)
	*/

	requestPacket.FixHeaderByteOrder();
	PrintHeader(requestPacket);

	cout << endl << "=======Request Data=======" << endl;
	cout << "Function Code: " << GetFunctionName(requestPacket.FunctionCode) << endl;

	ModbusADU responsePacket;
	uint8_t responseCode;

	switch (requestPacket.FunctionCode)
	{
		case ReadCoilStatus:
			responseCode = Read(COIL, requestPacket.Data);
			break;
		case ReadInputStatus:
			responseCode = Read(INPUT_STATUS, requestPacket.Data);
			break;
		case ReadHoldingRegister:
			responseCode = Read(HOLDING_REGISTER, requestPacket.Data);
			break;
		case ReadInputRegister:
			responseCode = Read(INPUT_REGISTER, requestPacket.Data);
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

	cout << endl << "==========================" << endl;

	return responsePacket;
}

uint8_t ModbusSlave::Read(uint16_t registerType, uint8_t *requestData)
{
	uint16_t startAddress = GetRawStartAddress(requestData);
	uint16_t numberRequested = GetNumberRequested(requestData);

	ResponseData response;
	int registersToRead = 0;
	cout << "Number of values requested: " << numberRequested << endl;

	//Allocate memory for response data
	switch (registerType)
	{
		case COIL:
		case INPUT_STATUS:
			response.data_size = (uint8_t) ceil( (double) numberRequested / BITS_PER_BYTE );
			response.data = (uint8_t*) calloc(response.data_size, sizeof(byte));
			break;
		case INPUT_REGISTER:
		case HOLDING_REGISTER:
			response.data_size = (uint8_t) numberRequested * BYTES_PER_REG;
			response.data = (uint8_t*) calloc(response.data_size, sizeof(byte));
			break;
		default:
			return -1;
	}

	registersToRead = ceil((double) response.data_size / 2);
	cout << "Registers to read: " << registersToRead << endl;

	for (uint16_t i = startAddress; i < (startAddress + registersToRead); i += 1)
	{
		uint16_t value = Coils[i];
		cout << "Value of data = ";
		PrintBinary(value);
		cout << endl;
	}

	cout << "Total Bytes Read: " << (int) response.data_size << endl;

	return 1;
}

void ModbusSlave::EnableZeroBasedAddressing(bool enabled)
{
	ZeroBasedAddressing = enabled;
	cout << "Zero Based Addressing" << (enabled ? " enabled" : " disabled") << endl;
}

uint16_t ModbusSlave::GetRawStartAddress(uint8_t *data)
{
	uint16_t address = ntohs(MAKEWORD(data[ADDR_HI], data[ADDR_LO])); //smash registers and reverse MSB to LSB

	if (ZeroBasedAddressing == false)
	{
		address -= 1;
	}

	return address;
}

uint16_t ModbusSlave::GetNumberRequested(uint8_t* data)
{
	return ntohs(MAKEWORD(data[NUM_REQ_HI], data[NUM_REQ_LO]));
}

void ModbusSlave::PrintHeader(ModbusADU packet)
{
	cout << endl << "=======Request Header Data=======" << endl;
	cout << "TransactionId: " << packet.TransactionId << endl;
	cout << "ProtocolId: " << packet.ProtocolId << endl;
	cout << "MessageLength: " << packet.MessageLength << endl;
	cout << "UnitId: " << (int) packet.UnitId << endl;
	cout << "=================================" << endl << endl;

}

uint16_t ModbusSlave::GetModbusAddress(uint8_t functionCode, uint16_t rawAddress)
{
	int registerStartAddress;

	switch (functionCode)
	{
	case ReadCoilStatus:
	case ForceSingleCoil:
	case ForceMultipleCoils:
		registerStartAddress = COIL;
		break;
	case ReadInputStatus:
		registerStartAddress = INPUT_STATUS;
		break;
	case ReadHoldingRegister:
	case PresetSingleRegister:
	case PresetMultipleRegisters:
		registerStartAddress = HOLDING_REGISTER;
		break;
	case ReadInputRegister:
		registerStartAddress = INPUT_REGISTER;
		break;
	default:
		registerStartAddress = -1;
	}

	return registerStartAddress + rawAddress;
}

void ModbusSlave::PrintBinary(uint16_t value)
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

string ModbusSlave::GetFunctionName(uint8_t functionCode)
{
	switch (functionCode)
	{
	case ReadCoilStatus:
		return "Read Coil";
	case ReadInputStatus:
		return "Read Input Status";
	case ReadHoldingRegister:
		return "Read Holding Register";
	case ReadInputRegister:
		return "Read Input Register";
	case ForceSingleCoil:
		return "Force Single Coil";
	case PresetSingleRegister:
		return "Preset Single Register";
	case ForceMultipleCoils:
		return "Force Multiple Coils";
	case PresetMultipleRegisters:
		return "Preset Multiple Registers";
	default:
		return "Unknown Function";
	}
}
