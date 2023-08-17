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
	requestPacket.FixHeaderByteOrder();
	PrintHeader(requestPacket);

	ModbusADU responsePacket = GetResponsePacket(requestPacket.FunctionCode, requestPacket.Data);

	return responsePacket;
}

ModbusADU ModbusSlave::GetResponsePacket(uint8_t functionCode, uint8_t* requestData)
{
	switch (functionCode)
	{
		case ReadCoilStatus:
			return ReadCoil(requestData);
		case ReadInputStatus:
			break;
		case ReadHoldingRegister:
			break;
		case ReadInputRegister:
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

}

ModbusADU ModbusSlave::ReadCoil(uint8_t *requestData)
{
	ModbusADU responsePacket;

	uint16_t startAddress = GetRawStartAddress(requestData);
	uint16_t numberRequested = GetNumberRequested(requestData);
	uint16_t coilValue = Coils[startAddress];

	return coilValue;
}

void ModbusSlave::EnableZeroBasedAddressing(bool enabled)
{
	ZeroBasedAddressing = enabled;
	cout << "Zero Based Addressing" << (enabled ? " enabled" : " disabled") << endl;
}

uint16_t ModbusSlave::GetRawStartAddress(uint8_t *data)
{
	uint16_t addressRaw = MAKEWORD(data[0], data[1]); //smash registers together to get 16 bit address
	uint16_t address = ntohs(addressRaw); //reverse network MSB standard to LSB

	if (ZeroBasedAddressing == false)
	{
		address -= 1;
	}

	return address;
}

uint16_t GetNumberRequested(uint8_t* data)
{

}

void ModbusSlave::PrintHeader(ModbusADU packet)
{
	cout << endl << "=======Request Header Data=======" << endl;
	cout << "TransactionId: " << packet.TransactionId << endl;
	cout << "ProtocolId: " << packet.ProtocolId << endl;
	cout << "MessageLength: " << packet.MessageLength << endl;
	cout << "UnitId: " << (int) packet.UnitId << endl;
	cout << "=================================" << endl << endl;

	cout << endl << "=======Request PDU Data=======" << endl;
	cout << "Function Code: " << GetFunctionName(packet.FunctionCode) << endl;
}

uint16_t ModbusSlave::GetModbusAddress(uint8_t functionCode, uint16_t rawAddress)
{
	int registerStartAddress;

	switch (functionCode)
	{
	case ReadCoilStatus:
	case ForceSingleCoil:
	case ForceMultipleCoils:
		registerStartAddress = RegisterStartAddress::Coil;
		break;
	case ReadInputStatus:
		registerStartAddress = RegisterStartAddress::InputStatus;
		break;
	case ReadHoldingRegister:
	case PresetSingleRegister:
	case PresetMultipleRegisters:
		registerStartAddress = RegisterStartAddress::HoldingRegister;
		break;
	case ReadInputRegister:
		registerStartAddress = RegisterStartAddress::InputRegister;
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
