#include <cstdint>
#include <cmath>

#include "Utils.h"

#define BYTE_LENGTH 8

int Utils::GetByteLengthForData(int num_requested, size_t size_of_unit)
{
	int byteLength = (uint8_t)ceil((double) (num_requested * size_of_unit) / BYTE_LENGTH);

	return byteLength;
}
