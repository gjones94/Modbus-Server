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

typedef struct ResponseData
{
	byte* data;
	int size;
	byte status;

} ResponseData;

typedef struct SerializedBuffer
{
	byte* buffer;
	int buffer_sz;

} SerializedBuffer;


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

		/* Helpers */
		unsigned short GetRequestStartAddress(const ModbusPacket &request);

		/* Request */
		/// <summary>
		/// Validates request information
		/// </summary>
		/// <param name="ModbusPacket [request]"></param>
		/// <returns>uint8_t [OK] if valid, [EXCEPTION_CODE] if invalid</returns>
		uint8_t ValidateRequest(ModbusPacket request);

		/// <summary>
		/// Reads coil and input status registers
		/// </summary>
		/// <param name="bool* [registers]"></param>
		/// <param name="ModbusPacket [request]"></param>
		/// <returns></returns>
		ResponseData* ReadCoilStatusRegisters(bool* registers, int startAddress, int size);

		/* Response */
		SerializedBuffer* GetResponse(const char* request);

		void SetException(ModbusPacket &response, byte result);

		/// <summary>
		///	Receive data from client and respond to client
		/// </summary>
		/// <param name="SOCKET [socket]"></param>
		/// <returns>boolean representing success or failure</returns>
		bool ReceiveAndRespond(SOCKET socket) override;
};

