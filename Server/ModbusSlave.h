#pragma once
#include "BaseServer.h"
#include "ModbusADU.h"

#define START_ADDRESS 1
#define DATA_BLOCK_SIZE 4096

class ModbusSlave : public BaseServer<ModbusADU>
{
	enum DataRead
	{
		ADDR_HI = 0,
		ADDR_LO = 1,
		NUM_REQUESTED_HI = 2,
		NUM_REQUESTED_LO = 3
	};

	enum RegisterStartAddress : int
	{
		COIL = 00001,
		INPUT_STATUS = 10001,
		INPUT_REGISTER = 30001,
		HOLDING_REGISTER = 40001
	};

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
		ModbusADU ReadCoil(uint8_t *requestData);

		/* Helpers */
		string GetFunctionName(uint8_t functionCode);
		ModbusADU GetResponsePacket(uint8_t functionCode, uint8_t* data);
		uint16_t GetRawStartAddress(uint8_t *data);
		uint16_t GetModbusAddress(uint8_t functionCode, uint16_t rawAddress);

		/* Diagnostics */
		void PrintHeader(ModbusADU packet);
		void PrintBinary(uint16_t value);
};

