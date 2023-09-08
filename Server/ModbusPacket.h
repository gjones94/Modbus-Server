#pragma once
#include <cstdint> // For datatypes
#include <iostream>
#include <winsock2.h>
#include "Utils.h"

using namespace std;

/*
	MODBUS PACKET

	REQUESTS
	--------
	All TCP Request Packets have the following structure to start:
	- uint8_t Slave Address
	- uint8_t Function Code
	- uint16_t Starting Data Address
	
	After this, there are some differences between read, write, and multiple write
	Read (Function 1 - 4):
	- uint16_t Size Requested
	
	Write (single) (Function 5 & 6):
	- uint16_t Value to write

	Write Multiple (Function 15 & 16):
	- uint16_t Number Coils/Registers Requested
	- uint8_t Number of BYTES to follow (Calculated based on 8 coils per byte or 2 bytes per register)
	--------

	RESPONSES
	---------


	---------
*/

enum STATUS : uint8_t
{
	/* Bit masks for function code */
	GOOD = 0xFF, /* &= returns the same value */
	BAD = 0x80, /* |= sets the far left bit for error */
};

#define HEADER_LENGTH 8
#define BASE_MESSAGE_LENGTH 2 // UID (1 byte) + FCODE (1 byte)

#define RQ_READ_INFO_SZ 4
#define RQ_WRITE_INFO_SZ 2
#define RQ_WRITES_INFO_SZ 5

#define RES_READ_INFO_SZ 1 // 1 Byte representing count of bytes in response
#define RES_WRITE_INFO_SZ 4
#define RES_WRITES_INFO_SZ 4

#define EX_INFO_SZ 1

enum REQUEST : int
{
	TID_HI,
	TID_LO,
	PID_HI,
	PID_LO,
	LEN_HI,
	LEN_LO,
	UID, //included in message length count
	FCODE, //included in message length count

	INDEX_OF_FIRST_DATA_BYTE
};

enum EXCEPTION : int
{
	EXCEPTION_CODE = 0
};

/* Functions (1 - 4) */
enum RQ_READ
{
	RQ_ADDR_HI,
	RQ_ADDR_LO,
	RQ_SIZE_HI,
	RQ_SIZE_LO
};

/* (Functions 5, 6) */
enum RQ_WRITE_SINGLE : int
{
	RQ_WRITE_ADDR_HI,
	RQ_WRITE_ADDR_LO,
	RQ_WRITE_VALUE_HI,
	RQ_WRITE_VALUE_LO,
};

/* (Functions 15, 16) */
enum RQ_WRITE_MULTIPLE : int
{
	RQ_WRITES_ADDR_HI,
	RQ_WRITES_ADDR_LO,
	RQ_WRITES_SIZE_HI,
	RQ_WRITES_SIZE_LO,
	RQ_WRITE_BYTE_COUNT
};

enum RES_READ : int
{
	RES_READ_BYTE_COUNT //First byte indicates number of data bytes to follow
};

enum RES_WRITE_SINGLE : int
{
	RES_WRITE_ADDR_HI, //first byte indicates Hi byte of the address written to
	RES_WRITE_ADDR_LO, //second byte indiciates the Lo byte of the address written to
	RES_WRITE_VALUE_HI, //first byte indicates the Hi byte of the value written
	RES_WRITE_VALUE_LO //second byte indicates the Lo byte of the value written
};

enum RES_WRITE_MULTIPLE : int
{
	RES_WRITES_ADDR_HI, //first byte indicates Hi byte of the address written to
	RES_WRITES_ADDR_LO, //second byte indiciates the Lo byte of the address written to
	RES_WRITES_SIZE_HI, //first byte indicates the Hi byte of the number of values written
	RES_WRITES_SIZE_LO //second byte indicates the Lo byte of the number of values written
};

enum ErrorCodes : uint8_t
{
	OK = 0x00, //Custom for indicating valid request
	ILLEGAL_FUNCTION = 0x01,
	ILLEGAL_ADDRESS = 0x02,
	ILLEGAL_DATA_VALUE = 0x03,
	SLAVE_DEVICE_FAILURE = 0x04,
	ACK = 0x05,
	SLAVE_DEVICE_BUSY = 0x06,
	NEGATIVE_ACK = 0x07,
	MEMORY_PARITY_ERROR = 0x08,
	GATEWAY_PATH_UNAVAILABLE = 0x10,
	GATEWAY_FAILED_TO_RESPOND = 0x11
};

enum FunctionCodes : uint8_t
{
	READ_COILS = 0x01,
	READ_INPUTS = 0x02,
	READ_HOLDING_REGISTERS = 0x03,
	READ_INPUT_REGISTERS = 0x04,
	WRITE_SINGLE_COIL = 0x05,
	WRITE_SINGLE_REGISTER = 0x06,
	WRITE_MULTIPLE_COILS = 0x0F,
	WRITE_MULTIPLE_REGISTERS = 0x10
};

enum ON_OFF : uint8_t
{
	ON_HI = 0xFF,
	ON_LO = 0x00,
	OFF_HI = 0x00,
	OFF_LO = 0x00
};

enum ENDIAN : uint8_t
{
	MSB,
	LSB
};

class ModbusPacket
{
	public:
		/* MBAP Header */
		unsigned short transaction_id;
		unsigned short protocol_id;
		unsigned short message_length;
		uint8_t unit_id;

		/* PDU Data */
		uint8_t function;
		uint8_t *data;

		/* Format Methods */
		void SetNetworkByteOrder();
		void SetHostByteOrder();

		/* Initializers */
		static ModbusPacket Deserialize(const char *buffer);
		static bool Serialize(const ModbusPacket& in_packet, char* serialized_buffer, int *serialized_buffer_sz);

		/* Data Section Helpers*/

		unsigned short GetRequestStartAddress() const;
		unsigned short GetRequestSize() const;
		unsigned short GetSingleWriteValue();

		/* Diagnostics */
		void PrintHeader();
		void PrintPacketBinary() const;

		ModbusPacket() 
		{
			transaction_id = 0;
			protocol_id = 0;
			message_length = 0;
			unit_id = 0;
			function = 0;
			byte_format = LSB;
			data = nullptr;
		};


		//Copy constructor
		ModbusPacket(const ModbusPacket& other)
		{
			cout << "Copy constructor called" << endl;
			transaction_id = other.transaction_id;
			protocol_id = other.protocol_id;
			message_length = other.message_length;
			unit_id = other.unit_id;
			function = other.function;

			int data_size = message_length - BASE_MESSAGE_LENGTH;
			data = new uint8_t[data_size];

			for (int i = 0; i < data_size; i++)
			{
				data[i] = other.data[i];
			}

			byte_format = other.byte_format;
		}

		//Assignment constructor
		ModbusPacket& operator=(const ModbusPacket& other)
		{
			cout << "Assignment constructor called" << endl;
			if (this != &other)
			{
				delete[] data;

				transaction_id = other.transaction_id;
				protocol_id = other.protocol_id;
				message_length = other.message_length;
				unit_id = other.unit_id;
				function = other.function;

				int data_size = message_length - BASE_MESSAGE_LENGTH;
				data = new uint8_t[data_size];
				for (int i = 0; i < data_size; i++)
				{
					data[i] = other.data[i];
				}
			}

			byte_format = other.byte_format;

			return *this;
		}

		/* Deconstructor */
		~ModbusPacket()
		{
			cout << "Desctructor called" << endl;

			if(data)
				delete[] data;
		}

	private:
		ENDIAN byte_format;

		//helper to obtain data size from message length field
		int GetDataSize() const;

};