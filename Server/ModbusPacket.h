#pragma once
#include <cstdint> // For datatypes
#include <iostream>
#include <winsock2.h>

using namespace std;

/*
	MODBUS PACKET

	REQUESTS
	--------
	All TCP Request Packets have the following structure to start:
	- uint8_t Slave Address
	- uint8_t Function Code
	- uint16_t Starting Data Address
	
	After this, there are some differences between read, write, and multiple write
	Read (Function 1 - 4):
	- uint16_t Size Requested
	
	Write (single) (Function 5 & 6):
	- uint16_t Value to write

	Write Multiple (Function 15 & 16):
	- uint16_t Number Coils/Registers Requested
	- uint8_t Number of BYTES to follow (Calculated based on 8 coils per byte or 2 bytes per register)
	--------

	RESPONSES
	---------


	---------
*/

enum STATUS : uint8_t
{
	/* Bit masks for function code */
	GOOD = 0xFF, /* &= returns the same value */
	BAD = 0x80, /* |= sets the far left bit for error */
};

enum TCP_REQUST : int
{
	TID_HI,
	TID_LO,
	PID_HI,
	PID_LO,
	LEN_HI,
	LEN_LO,
	UID,
	FCODE,
	ADDR_HI,
	ADDR_LO,
	DATA
};

/* REQUESTS */
enum REQ_TCP_READ : int
{
	REQ_SIZE_HI = 0,
	REQ_SIZE_LO = 1
};

enum REQ_TCP_WRITE_SINGLE : int
{
	REQ_VALUE_HI = 0,
	REQ_VALUE_LO = 1
};

enum REQ_TCP_WRITE_MULTIPLE : int
{
	REQ_SIZE = 1
};

enum RESP_TCP_READ : int
{
	RESP_SIZE = 0,
	RESP_DATA_START = 1
};

enum ErrorCodes : uint8_t
{
	IllegalFunction = 0x01,
	IllegalAddress = 0x02,
	IllegalDataValue = 0x03,
	SlaveDeviceFailure = 0x04,
	Acknowledge = 0x05,
	SlaveDeviceBusy = 0x06,
	NegativeAcknlowedge = 0x07,
	MemoryParityError = 0x08,
	GatewayPathUnavailable = 0x10,
	GatewayFailedToResponse = 0x11
};

enum FunctionCodes : uint8_t
{
	ReadCoilStatus = 0x01,
	ReadInputStatus = 0x02,
	ReadHoldingRegister = 0x03,
	ReadInputRegister = 0x04,
	ForceSingleCoil = 0x05,
	PresetSingleRegister = 0x06,
	ForceMultipleCoils = 0x0F,
	PresetMultipleRegisters = 0x10
};

enum ON_OFF : uint8_t
{
	ON_HI = 0xFF,
	ON_LO = 0x00,
	OFF_HI = 0x00,
	OFF_LO = 0x00
};

class ModbusPacket
{
	public:
		/* MBAP */
		uint16_t TransactionId;
		uint16_t ProtocolId;
		uint16_t MessageLength;
		uint8_t UnitId;

		/* PDU */
		uint8_t FunctionCode;
		uint16_t StartAddress;
		uint8_t *Data;

		/* Request Methods */
		void FixHeaderByteOrder();
		uint16_t GetStartAddress(bool zeroBasedAddressing);
		uint16_t GetRequestSize();
		uint16_t GetWriteValue();

		/* Diagnostics */
		void PrintHeader();

		ModbusPacket() 
		{
			TransactionId = 0;
			ProtocolId = 0;
			MessageLength = 0;
			UnitId = 0;
			FunctionCode = 0;
			StartAddress = 0;
		};

		/* Deconstructor */
		~ModbusPacket()
		{
			cout << "Deconstructor called" << endl;
			if(Data)
				delete[] Data;
		}
};