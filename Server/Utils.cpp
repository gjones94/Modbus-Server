#include <cstdint>
#include <cmath>
#include <iostream>

#include "Utils.h"

#define SIZE_OF_BYTE 8

using namespace std;

int Utils::NumBytesNeeded(int num_requested, size_t size_of_unit)
{
	int byteLength = (uint8_t)ceil((double) (num_requested * size_of_unit) / SIZE_OF_BYTE);

	return byteLength;
}

/*
	GetByte
	====================================================================
	Algorithm
	[F] T T T F T -> (array), start index 0
	 1  0 0 0 0 0 -> IF (array[index] = True)  { OR with 1 bit on far left }
	 0  1 0 0 0 0 -> IF (array[index] = True)  { OR with 1 bit on end shifted right by index count }
	 0  0 1 0 0 0 -> IF (array[index] = True)  { OR with 1 bit on end shifted right by index count }
	 ------------
	 0  1 1 1 0 1 RESULTING BINARY
	====================================================================
*/
uint8_t Utils::GetByte(bool* array)
{
	uint8_t byte = 0;

	int farLeftBit = 1 << (SIZE_OF_BYTE - 1);

	for (int i = 0; i < SIZE_OF_BYTE; i++) //move right
	{
		if (array[i] == true)
		{
			byte |= (farLeftBit >> i); //shift right
		}
	}

	return byte;
}

template<typename T>
void Utils::PrintArray(const T* array, int size)
{
	cout << endl;
	cout << "Array Values";
	cout << "====================================================================" << endl;
	for (int i = 0; i < size; i++)
	{
		cout << "i: " << i << " value: " << (T)array[i] << endl;
	}
	cout << "====================================================================" << endl;
	cout << endl;
}

template <typename T>
void Utils::PrintBinary(T value)
{
	int dataSize = (sizeof(T) * SIZE_OF_BYTE);
	for (int i = 1 << (dataSize - 1); i > 0; i >>= 1)
	{
		if (i & value)
		{
			printf("1");
		}
		else
		{
			printf("0");
		}
	}
}

/*
	Reverse: Reverse the the order of elements in an array
	====================================================================
	Algorithm
	swapping values beginning and end values until we reach halfway point
	i-> 	<-j
	1 2 3 4 5 6
	====================================================================
*/
template <typename T>
void Utils::Reverse(T* array, int size)
{
	for (int i = 0; i < (size / 2); i++)
	{
		int j = size - i - 1;
		T swap = array[j];
		array[j] = array[i];
		array[i] = swap;
	}
}

// Explicitly instantiate the Reverse<bool> function specialization
template void Utils::PrintBinary<uint8_t>(uint8_t);
template void Utils::PrintBinary<uint16_t>(uint16_t);
template void Utils::PrintBinary<char>(char);
template void Utils::Reverse<bool>(bool* array, int size);
