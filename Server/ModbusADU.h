#pragma once
#include <cstdint> // For datatypes
#include <iostream>

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

	public:
		uint16_t TransactionId = 0x0000;
		uint16_t ProtocolId = 0x0000;
		uint16_t MessageLength = 0x0000;
		uint8_t UnitId = 0x00;
		uint8_t FunctionCode = 0x00;
		uint8_t Data[];

		void Print(ModbusADU* packet);
};

