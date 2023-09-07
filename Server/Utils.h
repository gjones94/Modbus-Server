#pragma once


class Utils
{
	public:
		template <typename T> 
		static void Reverse(T* array, int size);

		template <typename T>
		static void PrintArray(const T* array, int size);

		template <typename T>
		static void PrintBinary(T value);

		/*
			num_requested: the number of data requested (e.g 3 coils or 5 registers)
			size_of_unit: the size of a coil or register (1 bit, 16 bits)
		*/
		static int GetNumBytesRequiredForData(int num_requested, size_t size_of_unit);

		static uint8_t GetByte(bool* array);

};