#pragma once
#include "BaseServer.h"
class ModbusSlave : public BaseServer<char*>
{
	public:
		ModbusSlave(int port);
		void Start();
};

