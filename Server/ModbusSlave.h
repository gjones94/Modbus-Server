#pragma once
#include "BaseServer.h"
#include "ModbusADU.h"
#include <cmath>

#define START_ADDRESS 1
#define DATA_BLOCK_SIZE 4096
#define BITS_PER_BYTE 8
#define BYTES_PER_REG 2

enum REQUEST : int
{
	ADDR_HI = 0,
	ADDR_LO = 1,
	NUM_REQ_HI = 2,
	NUM_REQ_LO = 3
};

enum RegisterType : uint16_t
{
	COIL = 00001,
	INPUT_STATUS = 10001,
	INPUT_REGISTER = 30001,
	HOLDING_REGISTER = 40001
};

class ModbusSlave : public BaseServer<ModbusADU>
{
	public:
		ModbusSlave(int port);
		void Start();

	private:
		/* Memory Blocks */
		uint16_t* Coils; //4096
		uint16_t* InputStatuses; //4096
		uint16_t* InputRegisters; //4096
		uint16_t* HoldingRegisters; //4096

		/* Options */
		bool ZeroBasedAddressing;

		/* Initialization */
		void InitializeMemory();
		void EnableZeroBasedAddressing(bool enabled);

		/* General Methods */
		ModbusADU GenerateResponse(ModbusADU data) override;
		uint8_t Read(uint16_t registerType, uint8_t *requestData);

		/* Helpers */
		string GetFunctionName(uint8_t functionCode);
		uint16_t GetRawStartAddress(uint8_t *data);
		uint16_t GetNumberRequested(uint8_t *data);
		uint16_t GetModbusAddress(uint8_t functionCode, uint16_t rawAddress);

		/* Diagnostics */
		void PrintHeader(ModbusADU packet);
		void PrintBinary(uint16_t value);
};

