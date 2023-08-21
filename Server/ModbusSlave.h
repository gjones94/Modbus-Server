#pragma once
#include "BaseServer.h"
#include "ModbusPacket.h"
#include <cmath>

#define DATA_BLOCK_SIZE 4096
#define BYTE_LENGTH 8
#define BYTES_PER_REG 2
#define BITS_PER_REG 16
#define ERROR_FLAG 0b10000000

template <typename T>
void Reverse(T* array, int size);

template <typename T>
void PrintArray(const T* array, int size);

template <typename T>
void PrintBinary(T value);

uint8_t GetByte(bool* array);



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

		/* Request Methods */
		uint16_t GetStartAddress(uint8_t *requestData);
		uint16_t GetSizeRequested(uint8_t *requestData);

		/* Response Methods */
		ModbusPacket GetResponse(ModbusPacket request) override;

		/* Read/Write Methods */
		ResponseData* ReadCoilStatusRegisters(bool* registers, uint16_t address, uint16_t size);

		/* Exception Reporting */
		bool Success(uint8_t functionCode);

		/* Diagnostics */
		void PrintHeader(ModbusPacket packet);

};

