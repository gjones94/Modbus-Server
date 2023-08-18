#pragma once
#include <cstdint> // For datatypes
#include <iostream>
#include <winsock2.h>

using namespace std;

#define MAX_DATA_LENGTH 250

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

enum CoilStatus : uint16_t
{
	ON = 0xFF00,
	OFF = 0x0000
};

typedef struct ResponseData
{
	uint8_t response_code;
	uint8_t data_size;
	uint8_t* data;
} ResponseData;

class ModbusADU
{
	public:
		/* MBAP */
		uint16_t TransactionId;
		uint16_t ProtocolId;
		uint16_t MessageLength;
		uint8_t UnitId;

		/* PDU */
		uint8_t FunctionCode;
		uint8_t Data[MAX_DATA_LENGTH];

		void FixHeaderByteOrder();
};

