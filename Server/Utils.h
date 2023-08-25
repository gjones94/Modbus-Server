#pragma once

class Utils
{
	public:
		/*
			num_requested: the number of data requested (e.g 3 coils or 5 registers)
			size_of_unit: the size of a coil or register (1 bit, 16 bits)
		*/
		static int GetByteLengthForData(int num_requested, size_t size_of_unit);
};

