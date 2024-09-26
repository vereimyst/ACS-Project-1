#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define CACHE_LINE_SIZE 64  // Cache line size is 64B for L1, L2, and L3 caches
#define L1D_CACHE_SIZE (48 * 1024)  // 48 KiB per core for L1 data cache
#define L2_CACHE_SIZE (1.25 * 1024 * 1024)  // 1.25 MiB per core for L2 cache
#define L3_CACHE_SIZE (24 * 1024 * 1024)  // 24 MiB shared L3 cache
#define MULTIPLICATION_CONSTANT 3  // Lightweight multiplication constant
#define REPEAT 1000  // Number of times to repeat the operation

// Function to get the current CPU cycle count
static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Function to perform computation and simulate cache misses
void compute_with_cache_pressure(int total_size, int num_elements, double *latency) {
    struct timespec start, end;
    
    // Allocate memory for the test
    int *array = (int *)malloc(total_size);
    if (array == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize the array with some values
    for (int i = 0; i < num_elements; i++) {
        array[i] = i;
    }

    // Start measuring time and CPU cycles
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Perform lightweight multiplication
    for (int r = 0; r < REPEAT; r++) {
        for (int i = 0; i < num_elements; i++) {
            array[i] *= MULTIPLICATION_CONSTANT;
        }
    }

    // Stop measuring time
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds
    *latency = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Free the allocated memory after computation
    free(array);
}

int main() {
    // Define the different cache levels and their sizes
    int cache_levels[] = {L1D_CACHE_SIZE, L2_CACHE_SIZE, L3_CACHE_SIZE};
    const char *cache_names[] = {"L1 Cache", "L2 Cache", "L3 Cache"};
    int num_cache_levels = sizeof(cache_levels) / sizeof(cache_levels[0]);

    // Variables to store latency
    double latency;

    // Open a CSV file to write the data
    FILE *csv_file = fopen("cache_miss_vs_latency.csv", "w");
    if (csv_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Write CSV header
    fprintf(csv_file, "Cache Level, Total Size (bytes), Latency (seconds)\n");

    // Loop over each cache level to test performance
    for (int i = 0; i < num_cache_levels; i++) {
        int total_size = cache_levels[i];  // Size of the memory to access

        printf("Testing %s:\n", cache_names[i]);
        for (int size = total_size / 4; size <= total_size * 2; size += total_size / 4) {
            int num_elements = size / sizeof(int);  // Recalculate num_elements for the current size
            compute_with_cache_pressure(size, num_elements, &latency);
            // Write number of cache misses (size) and latency to the CSV file
            fprintf(csv_file, "%s, %d, %.9f\n", cache_names[i], size, latency);
        }
    }

    // Close the CSV file
    fclose(csv_file);

    printf("Data has been saved to cache_miss_vs_latency.csv\n");
    return 0;
}
