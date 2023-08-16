#pragma once
#include "BaseServer.h"
#include "ModbusADU.h"

class ModbusSlave : public BaseServer<ModbusADU>
{
	public:
		ModbusSlave(int port);
		void Start();
		ModbusADU GenerateResponse(ModbusADU data) override;
};

