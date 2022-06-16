
#include <iron/json.h>

#include <chrono>
#include <fstream>
#include <iostream>

static uint64_t allocations = 0;

void* operator new(size_t size) {
    allocations++;
    return malloc(size);
}

void operator delete(void* p) noexcept {
    free(p);
}

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

static void print_stats(const char* name, double avg_s, double mb_s, int32_t iterations) {
    std::cout << std::left << std::setw(40) << name 
              << std::setw(20) << avg_s  
              << std::setw(20) << mb_s  
              << std::setw(20) << allocations 
              << std::setw(20) << iterations << "\n"; 
}

static void bench_parse_github_events() {
    std::string file = read_file("data/github_events.json");
    constexpr int32_t iterations = 5000;
    timer t;
    allocations = 0;
    for (int32_t i = 0; i < iterations; i++) {
        t.start();
        json::parse(file);
        t.stop();
    }
    double avg = t.accumulated_seconds / iterations; 
    allocations /= iterations;
    print_stats(__FUNCTION__, avg, (file.size() / avg) / (1024*1024), iterations);
}

static void bench_parse_san_fran() {
    std::string file = read_file("large_data/san_fran_parcels.json");
    constexpr int32_t iterations = 10;
    timer t;
    allocations = 0;
    for (int32_t i = 0; i < iterations; i++) {
        t.start();
        json::parse(file);
        t.stop();
    }
    double avg = t.accumulated_seconds / iterations; 
    allocations /= iterations;
    print_stats(__FUNCTION__, avg, (file.size() / avg) / (1024*1024), iterations);
}

static void bench_parse_canada() {
    std::string file = read_file("large_data/canada.json");
    constexpr int32_t iterations = 200;
    timer t;
    allocations = 0;
    for (int32_t i = 0; i < iterations; i++) {
        t.start();
        json::parse(file);
        t.stop();
    }
    double avg = t.accumulated_seconds / iterations; 
    allocations /= iterations;
    print_stats(__FUNCTION__, avg, (file.size() / avg) / (1024*1024), iterations);
}

static void bench_parse_twitter() {
    std::string file = read_file("large_data/twitter.json");
    constexpr int32_t iterations = 1000;
    timer t;
    allocations = 0;
    for (int32_t i = 0; i < iterations; i++) {
        t.start();
        json::parse(file);
        t.stop();
    }
    double avg = t.accumulated_seconds / iterations; 
    allocations /= iterations;
    print_stats(__FUNCTION__, avg, (file.size() / avg) / (1024*1024), iterations);
}
} // namespace bench

int main() {
    std::cout << std::left << std::setw(40) << "benchmark" 
              << std::setw(20) << "time (s)" 
              << std::setw(20) << "MB/s" 
              << std::setw(20) << "allocations" 
              << std::setw(20) << "iterations";
    std::cout << "\n______________________________________________________________________\n";
    bench::bench_parse_github_events();
    bench::bench_parse_san_fran();
    bench::bench_parse_canada();
    bench::bench_parse_twitter();
}
