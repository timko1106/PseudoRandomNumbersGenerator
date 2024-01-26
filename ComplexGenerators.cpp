#ifndef COMPLEX_GENERATORS_CPP
#define COMPLEX_GENERATORS_CPP

#include "ComplexGenerators.h"

void BoxGenerator::xor_rows(char* what, char* other, size_t len) {
	size_t cnt_shorted = len / SIZE_T_SIZE;
	size_t* first = (size_t*)what, *second = (size_t*)other;//Быстрее нельзя проксорить.
	while (cnt_shorted--) {
		*(first++) ^= *(second++);
	}
	size_t jmp = len - len % SIZE_T_SIZE;
	what += jmp;
	other += jmp;
	len -= jmp;
	while (len--) {
		*(what++) ^= *(other++);
	}
}

void BoxGenerator::new_state () {
	if (current_block % period == 0) {
		binary_t block{};
		block.buffer = tmp;//Меняем указатель
		block.size = full;
		block.clear();
		entropy->generate(full, block);
		block.buffer = nullptr;
		current_block = 0;
	}
	//Фактически получаем 8 независимых битовых матриц (8 бит=1Байт).
	for (size_t i = 0; i < rows; ++i) {
		char* cur_row = matrix + cols * i;
		char* cur_row_t = tmp + cols * i;
		memcpy (cur_row, cur_row_t, cols);
		//Ксорим с соседями.
		if (i > 0) {
			char* prev_row = cur_row_t - cols;
			xor_rows(cur_row + 1, prev_row, cols - 1);
			xor_rows(cur_row, prev_row, cols);
			xor_rows(cur_row, prev_row + 1, cols - 1);
		}
		xor_rows(cur_row + 1, cur_row_t, cols - 1);
		xor_rows(cur_row, cur_row_t + 1, cols - 1);
		if (i + 1 < rows) {
			char* next_row = cur_row_t + cols;
			xor_rows(cur_row + 1, next_row, cols - 1);
			xor_rows(cur_row, next_row, cols);
			xor_rows(cur_row, next_row + 1, cols - 1);
		}
	}
	++current_block;
	location = 0;
}

int BoxGenerator::generate (size_t count, binary_t& buff) {
	buff.allocate (count);
	if (count <= full - location) {
		memcpy(buff.buffer, matrix + location, count);
		location += count;
		if (location == full) {
			new_state();
		}
		return 0;
	}
	for (size_t i = 0; i < count / full; ++i) {
		new_state();
		memcpy(buff.buffer + i * full, matrix, full);
	}
	new_state();
	if (count % full) {
		memcpy(buff.buffer + count - count % full, matrix, count % full);
		location = count % full;
	}
	return 0;
}

#endif
