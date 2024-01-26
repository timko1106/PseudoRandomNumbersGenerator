#ifndef ENTROPY_CPP
#define ENTROPY_CPP

#include <ctime>
#include "Generator.h"
#include "ComplexEntropy.h"

//Использует rand ()
int FastEntropy::generate(size_t count, binary_t& buff) {
	using rand_t = unsigned short;
	const size_t RAND_SIZE = sizeof (rand_t);
	srand(time(NULL));
	size_t ceiled = count / RAND_SIZE;
	buff.allocate(count);
	//2 байта гарантированно по стандарту даёт rand
	for (size_t i = 0; i < ceiled; ++i) {
		(rand_t&)(buff.buffer[RAND_SIZE * i]) = (unsigned short)rand();
	}
	if (count % RAND_SIZE) {
		for (size_t i = RAND_SIZE * ceiled; i < count; ++i) {
			buff.buffer[i] = rand();
		}
	}
	return 0;
}

//При помощи random_device
int SystemEntropy::generate(size_t count, binary_t& buff) {
	size_t ceiled = count / ENTROPY_SIZE;
	buff.allocate(count);
	for (size_t i = 0; i < ceiled; ++i) {
		(std::random_device::result_type&)(buff.buffer[ENTROPY_SIZE * i]) = random_source();
	}
	if (count % ENTROPY_SIZE) {
		for (size_t i = ENTROPY_SIZE * ceiled; i < count; ++i) {
			buff.buffer[i] = random_source();
		}
	}
	return 0;
}

int MixedEntropy::generate(size_t count, binary_t& buff) {
	//Используя 64-битный вихрь мерсенна
	const size_t MERSENNE_SIZE = sizeof(std::mt19937_64::result_type);
	std::mt19937_64 generator{};
	binary_t reserve;
	buff.allocate(count);
	real_source->generate(((count / MERSENNE_SIZE + period - 1) / period + count % MERSENNE_SIZE) * MERSENNE_SIZE, reserve);
	size_t c = 0;
	for (size_t i = 0; i < count / MERSENNE_SIZE; ++i) {
		if (i % period == 0) {
			generator.seed(((std::mt19937_64::result_type*)reserve.buffer)[c]);
			++c;
		}
		((std::mt19937_64::result_type*)buff.buffer)[i] = generator();
	}
	if (count % MERSENNE_SIZE) {
		c *= MERSENNE_SIZE;//Переход к байтам
		for (size_t i = count - count % MERSENNE_SIZE; i < count; ++i) {
			buff.buffer[i] = reserve.buffer[c];
			++c;
		}
	}
	return 0;
}

#endif
