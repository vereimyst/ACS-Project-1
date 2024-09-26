#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define L1D_CACHE_SIZE (48 * 1024)  // 48 KiB per core for L1 data cache
#define L2_CACHE_SIZE (1.25 * 1024 * 1024)  // 1.25 MiB per core for L2 cache
#define L3_CACHE_SIZE (24 * 1024 * 1024)  // 24 MiB shared L3 cache
#define PAGE_SIZE_4KB (4 * 1024)  // 4KB page size
#define PAGE_SIZE_2MB (2 * 1024 * 1024)  // 2MB page size
#define MULTIPLICATION_CONSTANT 3  // Lightweight multiplication constant
#define REPEAT 1000  // Repeat the operation to get average values

// Function to get the current CPU cycle count
static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Function to perform computation and measure latency and CPU cycles with varying page sizes
void compute_with_page_size(int page_size, int total_size, int num_pages, double *latency, double *cpu_cycles) {
    struct timespec start, end;
    int **pages = (int **)malloc(num_pages * sizeof(int *));
    uint64_t cycles_start, cycles_end;

    // Allocate memory in chunks of 'page_size' to simulate memory accesses across multiple pages
    for (int i = 0; i < num_pages; i++) {
        pages[i] = (int *)malloc(page_size);
        for (int j = 0; j < page_size / sizeof(int); j++) {
            pages[i][j] = i + j;
        }
    }

    // Start measuring time and CPU cycles
    clock_gettime(CLOCK_MONOTONIC, &start);
    cycles_start = rdtsc();

    // Perform multiplication across all pages
    for (int r = 0; r < REPEAT; r++) {
        for (int i = 0; i < num_pages; i++) {
            for (int j = 0; j < page_size / sizeof(int); j++) {
                pages[i][j] *= MULTIPLICATION_CONSTANT;
            }
        }
    }

    // Stop measuring time and CPU cycles
    cycles_end = rdtsc();
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds and CPU cycles
    *latency = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    *cpu_cycles = (double)(cycles_end - cycles_start);

    // Free memory
    for (int i = 0; i < num_pages; i++) {
        free(pages[i]);
    }
    free(pages);
}

int main() {
    // Total data size to simulate accesses across different cache levels
    int total_size = L3_CACHE_SIZE * 2;  // Exceeding L3 cache size to force main memory access

    // Define page sizes to simulate TLB miss impacts (4KB and 2MB pages)
    int page_sizes[] = {PAGE_SIZE_4KB, PAGE_SIZE_2MB};
    const char *page_size_names[] = {"4KB Pages", "2MB Pages"};
    int num_page_sizes = sizeof(page_sizes) / sizeof(page_sizes[0]);

    // Variables to store latency and CPU cycles
    double latency, cpu_cycles;

    // Loop over each page size to simulate different TLB miss ratios
    for (int p = 0; p < num_page_sizes; p++) {
        int num_pages = total_size / page_sizes[p];  // Calculate the number of pages for each size

        printf("Testing with %s:\n", page_size_names[p]);
        compute_with_page_size(page_sizes[p], total_size, num_pages, &latency, &cpu_cycles);
        printf("%s: Total Size = %d bytes, Pages = %d, Average Latency = %.9f seconds, CPU Cycles = %.0f\n\n",
               page_size_names[p], total_size, num_pages, latency / REPEAT, cpu_cycles / REPEAT);
    }

    return 0;
}
