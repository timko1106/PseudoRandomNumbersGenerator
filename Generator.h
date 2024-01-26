#ifndef GENERATOR_H
#define GENERATOR_H

#include <random>
#include <cstring>
#include <memory>
#include <ctime>
using std::size_t;
using std::unique_ptr;
using std::move;
const size_t SIZE_T_SIZE = sizeof(size_t);

//Просто блок памяти. указатель+размер
struct binary_t {
	char* buffer = nullptr;
	size_t size = 0;
	binary_t() : buffer(nullptr), size(0) {}
	void allocate(size_t newsize) {
		if (buffer != nullptr && size >= newsize) {
			size = newsize;
			return;
		}
		delete[] buffer;
		buffer = new char[newsize];
		size = newsize;
	}
	void destroy() {
		delete[] buffer;
		buffer = nullptr;
		size = 0;
	}
	void clear() {
		if (buffer) {
			memset(buffer, 0, size);
		}
	}
	~binary_t() { destroy (); }
};
//Абстрактный генератор произвольной последовательности из n байтов.
class BaseGenerator {
public:
	BaseGenerator() {}
	BaseGenerator(const BaseGenerator&) = delete;
	BaseGenerator(const BaseGenerator&&) = delete;
	virtual int generate(size_t bytes, binary_t& buffer) = 0;//всегда должен вернуть 0.
	virtual ~BaseGenerator () {}
};

//Энтропийный генератор. Из системы.
class BaseEntropySource : public BaseGenerator {
public:
	virtual int generate(size_t bytes, binary_t& buffer) = 0;
	virtual ~BaseEntropySource() {}
};

//генерирует используя rand()
class FastEntropy : public BaseEntropySource {
public:
	~FastEntropy() {}
	int generate(size_t bytes, binary_t& buffer);
};

//Пользовательский генератор последовательностей, использующий энтропию
class BaseSequenceGenerator : public BaseGenerator {
protected:
	std::unique_ptr<BaseEntropySource> entropy = nullptr;
public:
	BaseSequenceGenerator(unique_ptr<BaseEntropySource>&& e = nullptr) 
		: entropy(e ? std::move(e) : unique_ptr<BaseEntropySource> (new FastEntropy())) {}
	virtual ~BaseSequenceGenerator() {}
	virtual int generate(size_t bytes, binary_t& buffer) = 0;
};
//Генератор чисел на основе BaseGenerator
template<typename T>
class NumbersGenerator {
	std::unique_ptr<BaseGenerator> gen = nullptr;
	size_t block_size, index;
	binary_t block{};
public:
	const size_t NUMBER_SIZE = sizeof(T);
	NumbersGenerator(std::unique_ptr<BaseGenerator>&& e = nullptr, size_t block_size = 256) : 
		gen(e ? std::move(e) : unique_ptr<BaseGenerator>(new FastEntropy ())) {
		this->index = 0;
		this->block_size = std::max((size_t)10, block_size) * NUMBER_SIZE;//Как минимум по умолчанию выделяет на 10 чисел.
		block.allocate(this->block_size);
	}
	~NumbersGenerator() {}
	size_t generate() {
		if (index * NUMBER_SIZE % block_size == 0) {
			gen->generate(block_size, block);
			index = 0;
		}
		return ((T*)block.buffer)[index++];
	}
	BaseGenerator* return_back() {
		BaseGenerator* ptr = gen.release();
		gen = unique_ptr<BaseGenerator>(new FastEntropy());
		return ptr;
	}
};

#endif
