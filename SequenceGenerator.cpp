#include <iostream>
#include <vector>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include "ComplexEntropy.h"
#include "ComplexGenerators.h"

template<typename T>
std::tuple<std::chrono::nanoseconds, long double, long double> long_numbers_generate(NumbersGenerator<T>& gen, std::vector<T>& numbers, size_t SIZE = 10000) {
	numbers.reserve(SIZE);
	auto begin = std::chrono::high_resolution_clock::now();
	while (SIZE--) {
		numbers.push_back(gen.generate());
	}
	auto end = std::chrono::high_resolution_clock::now();
	std::unordered_map<T, size_t> repeated{};
	long double sum{0}, entropy{ 0.0 };
	//p(x)=C/N
	//entropy=-E(p(x)log2p(x))=E(C*(log2(N)-log2(C)))/N
	//log2p(x) = log2(C)-log2(N)
	long double log2N = log2 (numbers.size ());
	for (T& value : numbers) {
		sum += value;
		++(repeated[value]);
	}
	for (auto& vals : repeated) {
		entropy += vals.second * (log2N - log2 (vals.second));
	}
	entropy /= numbers.size ();
	long double stddev_sum{ 0 }, mean = sum / numbers.size ();
	for (T& value : numbers) {
		stddev_sum += (value - mean) * (value - mean);
	}
	long double stddev = stddev_sum;
	stddev /= numbers.size();
	stddev = sqrt(stddev);
	return { std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin), stddev , entropy };
}
template<typename T, bool need_print>
void print(unique_ptr<BaseGenerator>& ref, const char* msg = nullptr, size_t count = 10000) {
	std::vector<T> numbers;
	NumbersGenerator<T> prng{ std::move(ref) };
	auto res = long_numbers_generate<T>(prng, numbers, count);
	ref = unique_ptr<BaseGenerator>(prng.return_back());
	auto nanoseconds = std::get<0>(res).count ();
	auto stddev = std::get<1>(res);
	auto entropy = std::get<2>(res);
	std::cout << "Method: " << (msg ? msg : "unknown") << '\n';
	if (need_print) {
		for (T& val : numbers) {
			std::cout << val << ' ';
		}
		std::cout << '\n';
	}
	std::cout << "Generated " << numbers.size() << " numbers by " << nanoseconds << " ns with standard deviation = " << stddev << " and entropy = " << entropy << ".\n";
}

int main () {
	unique_ptr<BaseGenerator> fast{ new FastEntropy() };
	unique_ptr<BaseGenerator> real{ new SystemEntropy() };
	unique_ptr<BaseGenerator> changed{ new MixedEntropy() };
	BoxGenerator* ptr = new BoxGenerator(10, 10, 10, unique_ptr<BaseEntropySource>(new SystemEntropy()));
	BoxGenerator* complexed = new BoxGenerator(20, 10, 15, unique_ptr<BaseEntropySource>(new MixedEntropy(nullptr, 2048)));
	unique_ptr<BaseGenerator> generator{ ptr };
	unique_ptr<BaseGenerator> complexed_generator{ complexed };
	print<unsigned, true>(fast, "fast rand()", 512);
	print<unsigned, true>(real, "system entropy source", 512);
	print<unsigned, true>(changed, "mt19937 with system entropy", 512);
	print<unsigned, true>(generator, "matrix method", 512);
	print<unsigned, true>(complexed_generator, "matrix method with mt19937", 512);
	const char* strings[] = { "Matrix base method", "Matrix complexed method" };
	BoxGenerator* ptrs[] = { ptr, complexed };
	const size_t NEED_TO_PRINT = 4;
	for (size_t i = 0; i < sizeof(ptrs) / sizeof(*ptrs); ++i) {
		std::cout << strings[i] << '\n';
		for (size_t j = 0; j < NEED_TO_PRINT - 1; ++j) {
			std::cout << *(ptrs[i]) << "\n\n\n";
			ptrs[i]->new_state();
		}
		if (NEED_TO_PRINT) {
			std::cout << *(ptrs[i]) << "\n\n\n";
		}
	}
	print<unsigned, false>(changed, "mt19937 with system entropy", 100000);
	print<unsigned, false>(real, "system entropy source", 100000);
	print<unsigned, false>(fast, "fast rand()", 100000);
	print<unsigned, false>(generator, "matrix method", 1000000);
	print<unsigned, false>(complexed_generator, "matrix method with mt19937", 1000000);
	ptr = nullptr;
	complexed = nullptr;
	return 0;
}
