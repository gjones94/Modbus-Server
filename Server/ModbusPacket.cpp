#include "ModbusPacket.h"

void ModbusPacket::FixHeaderByteOrder()
{
	TransactionId = ntohs(TransactionId);
	ProtocolId = ntohs(ProtocolId);
	MessageLength = ntohs(MessageLength);
}