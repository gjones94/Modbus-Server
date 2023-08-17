#pragma once
#include <cstdint> // For datatypes
#include <iostream>
#include <winsock2.h>

using namespace std;

class ModbusADU
{
	enum FunctionCode : uint8_t
	{
		ReadCoilStatus = 0x01,
		ReadInputStatus = 0x02,
		ReadHoldingRegister = 0x03,
		ReadInputRegister = 0x04
	};

	enum CoilStatus : uint16_t
	{
		ON = 0xFF00,
		OFF = 0x0000
	};

	public:
		uint16_t TransactionId;
		uint16_t ProtocolId;
		uint16_t MessageLength;
		uint8_t UnitId;
		uint8_t FunctionCode;
		uint8_t Data[1024];

		void FixByteOrder();
		void Print();
		string GetFunction(uint8_t code);
		void ToBinary(uint16_t value);
	
};

