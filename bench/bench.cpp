
#include <iron/json.h>

#include <chrono>
#include <fstream>
#include <iostream>

namespace bench {

using fe::json;
using clock = std::chrono::steady_clock;

std::string read_file(const char* path) {
    if(std::ifstream is{path, std::ios::binary | std::ios::ate}) {
        auto size = is.tellg();
        std::string str(size, '\0'); // construct string to stream size
        is.seekg(0);
        if(is.read(&str[0], size)) {
            return str;
        }
    }
    return {};
}

// https://github.com/google/benchmark/blob/main/include/benchmark/benchmark.h#L439
inline void clobber_memory() {
  std::atomic_signal_fence(std::memory_order_acq_rel);
}

// https://github.com/google/benchmark/blob/main/include/benchmark/benchmark.h#L450
template <class Tp>
inline void do_not_optimize(Tp const& value) {
  asm volatile("" : : "r,m"(value) : "memory");
}

struct timer {
    inline void start() { 
        start_time = clock::now();
    }

    inline void stop() {
        auto end_time = clock::now();
        std::chrono::duration<double> time_s = end_time - start_time;
        accumulated_seconds += time_s.count();
    }

    clock::time_point start_time;
    double accumulated_seconds = 0.0;
};

static void bench_parse() {
    std::string file = read_file("data/github_events.json");
    constexpr int32_t iterations = 500;
    timer t;
    for (int32_t i = 0; i < iterations; i++) {
        t.start();
        json::parse(file);
        t.stop();
    }
    double avg = t.accumulated_seconds / iterations; 

    std::cout << std::left << std::setw(20) << "benchmark" << std::setw(20) << "time (s)" << std::setw(20) << "iterations";
    std::cout << "\n__________________________________________________\n";
    std::cout << std::left << std::setw(20) << "bench_parse " << std::setw(20) << avg  << std::setw(20) << iterations << "\n"; 
}

}

int main() {
    bench::bench_parse();
}
