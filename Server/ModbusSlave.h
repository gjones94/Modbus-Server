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

enum REGISTER_TYPE : byte
{
	COIL_OUTPUT = 0,
	DISCRETE_INPUT = 1,
	INPUT_REGISTER = 3,
	HOLDING_REGISTER = 4
};

typedef struct ResponseData
{
	byte* data;
	int size;
	byte status;

} ResponseData;

class ModbusSlave : public BaseServer<ModbusPacket>
{
public:
	ModbusSlave(int port);
	void Start();

private:
	/* Memory Blocks */
	bool* coil_outputs; //65536
	bool* input_statuses; //65536
	unsigned short* input_registers; //4096
	unsigned short* holding_registers; //4096

	/* Options */
	bool ZeroBasedAddressing;

	/* Initialization */
	void InitializeRegisters();
	void EnableZeroBasedAddressing(bool enabled);

	/* Helpers */
	unsigned short GetRequestStartAddress(const ModbusPacket& request);
	void* GetRegisterBlock(byte registerType);

	/* Request */
	/// <summary>
	/// Validates request information
	/// </summary>
	/// <param name="ModbusPacket [request]"></param>
	/// <returns>byte [OK] if valid, [EXCEPTION_CODE] if invalid</returns>
	byte ValidateRequest(ModbusPacket request);

	/// <summary>
	/// Reads coil and input status registers
	/// </summary>
	/// <param name="bool* [registers]"></param>
	/// <param name="ModbusPacket [request]"></param>
	/// <returns></returns>
	ResponseData* ReadStatus(byte registerType, int startAddress, int size);

	ResponseData* ReadRegister(byte registerType, int startAddress, int size);

	/* Response */
	SendBuffer* GetResponse(const char* request, int request_size) override;

	void SetException(ModbusPacket& response, byte result);
};
