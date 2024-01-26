#ifndef COMPLEX_ENTROPY_H
#define COMPLEX_ENTROPY_H

#include "Generator.h"
#include <random>
#include <cstring>
#include <memory>
#include <type_traits>

//Энтропия из "надёжного" источника
class SystemEntropy : public BaseEntropySource {
	std::random_device random_source;
public:
	const size_t ENTROPY_SIZE = sizeof(std::random_device::result_type);
	SystemEntropy() : random_source() {}
	~SystemEntropy () {}
	int generate(size_t count, binary_t& buff);
};

//Более сложное использование произвольного источника энтропии
class MixedEntropy : public BaseEntropySource {
	std::unique_ptr<BaseEntropySource> real_source;
	size_t period;
public:
	MixedEntropy(std::unique_ptr<BaseEntropySource>&& e = nullptr, size_t period = 1024) : real_source(e.get() ? std::move(e) : std::make_unique<SystemEntropy>()), period (std::max(period, (size_t)10)) {}
	~MixedEntropy () {}
	int generate(size_t count, binary_t& buff);
};

#endif
