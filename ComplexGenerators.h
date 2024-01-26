#ifndef COMPLEX_GENERATORS_H
#define COMPLEX_GENERATORS_H

#include "Generator.h"
#include "ComplexEntropy.h"
#include <iostream>
//Блочный генератор, ксорящий с соседними 8 байтами
class BoxGenerator : public BaseSequenceGenerator {
	char* matrix = nullptr, *tmp = nullptr;
	size_t rows, cols, period, current_block, location;
	size_t full;
	static void xor_rows(char* what, char* other, size_t len);
public:
	void new_state ();
	BoxGenerator(size_t rows, size_t cols, size_t period, std::unique_ptr<BaseEntropySource>&& e = nullptr) : 
		BaseSequenceGenerator(e ? std::move(e) : unique_ptr<BaseEntropySource>(new FastEntropy())), 
		rows (std::max ((size_t)4, rows)), cols (std::max ((size_t)4, cols)), //меньше 4 неудачно получается
		period (std::max ((size_t)40, period)), current_block (0), location (0) {
		full = this->rows * this->cols;
		matrix = new char[full];
		tmp = new char[full];
		binary_t buffer{};
		buffer.buffer = matrix;
		buffer.size = full;
		entropy->generate(full, buffer);
		buffer.buffer = nullptr;
	}
	~BoxGenerator() {
		delete[] matrix;
		delete[] tmp;
	}
	int generate(size_t count, binary_t& buff);
	friend std::ostream& operator<< (std::ostream& os, const BoxGenerator& generator) {
		for (size_t i = 0; i < generator.rows; ++i) {
			for (size_t j = 0; j < generator.cols; ++j) {
				unsigned char sym = generator.matrix[i * generator.cols + j];
				for (int i = 7; i >= 0; --i) {
					if (sym & (1 << i)) {
						os << "\x1b[97;107m \x1b[0m";//управляющие ANSI. белый пробел
					}
					else {
						os << "\x1b[30;40m \x1b[0m";//чёрный пробел
					}
				}
			}
			os << '\n';
		}
		return os;
	}
};


#endif
