#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define L1D_CACHE_SIZE (48 * 1024)  // 48 KiB per core for L1 data cache
#define L2_CACHE_SIZE  (1.25 * 1024 * 1024)  // 1.25 MiB per core for L2 cache
#define L3_CACHE_SIZE  (24 * 1024 * 1024)  // 24 MiB shared L3 cache
#define CACHE_LINE_SIZE 64  // Cache line size is 64 bytes
#define MULTIPLICATION_CONSTANT 3  // Lightweight computation constant
#define REPEAT 1000  // Repeat the operation to get average values

// Function to get the current CPU cycle count
static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Function to perform computation and measure latency and CPU cycles
void compute_with_size(int data_size, int num_elements, double *latency, double *cpu_cycles) {
    struct timespec start, end;
    int *array = (int *)malloc(data_size);
    uint64_t cycles_start, cycles_end;

    // Initialize the array
    for (int i = 0; i < num_elements; i++) {
        array[i] = i;
    }

    // Start measuring time and CPU cycles
    clock_gettime(CLOCK_MONOTONIC, &start);
    cycles_start = rdtsc();

    // Perform lightweight multiplication
    for (int j = 0; j < REPEAT; j++) {
        for (int i = 0; i < num_elements; i++) {
            array[i] *= MULTIPLICATION_CONSTANT;
        }
    }

    // Stop measuring time and CPU cycles
    cycles_end = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds and CPU cycles
    *latency = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    *cpu_cycles = (double)(cycles_end - cycles_start);

    free(array);  // Clean up
}

int main() {
    // Array sizes that fit within L1d, L2, L3 cache, and exceed L3 cache for memory access
    int cache_levels[] = {L1D_CACHE_SIZE, L2_CACHE_SIZE, L3_CACHE_SIZE, L3_CACHE_SIZE * 2}; 
    const char *cache_names[] = {"L1d Cache", "L2 Cache", "L3 Cache", "Main Memory"};
    int num_levels = sizeof(cache_levels) / sizeof(cache_levels[0]);

    // Variables to store latency and CPU cycles
    double latency, cpu_cycles;

    // Loop over each cache level and measure performance
    for (int i = 0; i < num_levels; i++) {
        int num_elements = cache_levels[i] / sizeof(int);  // Number of elements in the array

        printf("Testing %s:\n", cache_names[i]);
        compute_with_size(cache_levels[i], num_elements, &latency, &cpu_cycles);
        printf("%s: Size = %d bytes, Average Latency = %.9f seconds, CPU Cycles = %.0f\n\n",
               cache_names[i], cache_levels[i], latency / REPEAT, cpu_cycles / REPEAT);
    }

    return 0;
}
