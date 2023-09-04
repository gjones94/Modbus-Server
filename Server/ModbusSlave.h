#pragma once
#include "BaseServer.h"
#include "ModbusPacket.h"
#include <cmath>

#define MODBUS_REGISTER_CAPACITY 100000
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
		bool* coil_registers; //65536
		bool* status_registers; //65536
		unsigned short* input_registers; //4096
		unsigned short* holding_registers; //4096

		/* Options */
		bool ZeroBasedAddressing;

		/* Initialization */
		void InitializeRegisters();
		void EnableZeroBasedAddressing(bool enabled);

		/* Response Methods */
		size_t GetSendBufferSize(const ModbusPacket* sendData);

		/* Read/Write Methods */
		ModbusPacket ReadCoilStatusRegisters(bool* registers, const ModbusPacket& request);
		
		/* Base Server override */
		bool ReceiveAndRespond(SOCKET socket) override;
		ModbusPacket GetResponse(const ModbusPacket requestData);
};

