#pragma once
#include "BaseServer.h"
#include "ModbusPacket.h"
#include <cmath>

#define DATA_BLOCK_SIZE 4096
#define SIZE_OF_BYTE 8
#define BYTES_PER_REG 2
#define BITS_PER_COIL 1
#define BITS_PER_REG 16
#define ERROR_FLAG 0b10000000



class ModbusSlave : public BaseServer<ModbusPacket>
{
	public:
		ModbusSlave(int port);
		void Start();

	private:
		/* Memory Blocks */
		bool* CoilRegisters; //65536
		bool* StatusRegisters; //65536
		uint16_t* InputRegisters; //4096
		uint16_t* HoldingRegisters; //4096

		/* Options */
		bool ZeroBasedAddressing;

		/* Initialization */
		void InitializeMemory();
		void EnableZeroBasedAddressing(bool enabled);

		/* Response Methods */
		size_t GetSendBufferSize(const ModbusPacket* sendData);

		/* Read/Write Methods */
		ModbusPacket ReadCoilStatusRegisters(bool* registers, const ModbusPacket& request);
		
		/* Base Server override */
		ModbusPacket GetResponse(char * requestData) override;
		bool Send(SOCKET clientSocket, const ModbusPacket* sendData) override;
};

