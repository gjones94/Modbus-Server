#include "ModbusADU.h"

void ModbusADU::FixHeaderByteOrder()
{
	TransactionId = ntohs(TransactionId);
	ProtocolId = ntohs(ProtocolId);
	MessageLength = ntohs(MessageLength);
}