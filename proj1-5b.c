#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define L1D_CACHE_SIZE (48 * 1024)  // 48 KiB per instance for L1 data cache
#define L2_CACHE_SIZE (1.25 * 1024 * 1024)  // 1.25 MiB per instance for L2 cache
#define L3_CACHE_SIZE (24 * 1024 * 1024)  // 24 MiB shared L3 cache
#define PAGE_SIZE_4KB (4 * 1024)  // 4KB page size
#define PAGE_SIZE_2MB (2 * 1024 * 1024)  // 2MB page size
#define MULTIPLICATION_CONSTANT 3  // Lightweight multiplication constant
#define REPEAT 5  // Number of times to repeat the operation

// Function to get the current CPU cycle count
static inline uint64_t rdtsc() {
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}

// Function to perform computation with memory accesses that generate TLB misses
void compute_with_tlb_pressure(int page_size, int num_pages, double *latency) {
    struct timespec start, end;
    int **pages = (int **)malloc(num_pages * sizeof(int *));
    uint64_t cycles_start, cycles_end;

    // Allocate memory in page-sized chunks to simulate memory access across multiple pages
    for (int i = 0; i < num_pages; i++) {
        pages[i] = (int *)malloc(page_size);
        for (int j = 0; j < page_size / sizeof(int); j++) {
            pages[i][j] = i + j;  // Initialize memory
        }
    }

    // Start measuring time
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Perform lightweight multiplication across pages
    for (int r = 0; r < REPEAT; r++) {
        for (int i = 0; i < num_pages; i++) {
            for (int j = 0; j < page_size / sizeof(int); j++) {
                pages[i][j] *= MULTIPLICATION_CONSTANT;
            }
        }
    }

    // Stop measuring time
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate elapsed time in seconds
    *latency = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;

    // Free allocated memory
    for (int i = 0; i < num_pages; i++) {
        free(pages[i]);
    }
    free(pages);
}

int main() {
    // Define the page size to use for testing (4KB and 2MB pages)
    int page_size = PAGE_SIZE_4KB;  // Focus on 4KB pages for this example

    // Variables to store latency
    double latency;

    // Total number of pages to test increasing TLB miss ratios
    int max_pages = 100000;  // Adjust as needed to ensure enough TLB misses
    int step_size = 500;  // Step size for increasing TLB pressure

    // Open a CSV file to write the data
    FILE *csv_file = fopen("tlb_miss_vs_latency.csv", "w");
    if (csv_file == NULL) {
        perror("Error opening file");
        return 1;
    }

    // Write CSV header
    fprintf(csv_file, "Number of Pages, Latency (seconds)\n");

    // Loop over increasing numbers of pages to simulate increasing TLB misses
    for (int num_pages = step_size; num_pages <= max_pages; num_pages += step_size) {
        compute_with_tlb_pressure(page_size, num_pages, &latency);
        // Write number of pages and latency to the CSV file
        fprintf(csv_file, "%d, %.9f\n", num_pages, latency);
    }

    // Close the CSV file
    fclose(csv_file);

    printf("Data has been saved to tlb_miss_vs_latency.csv\n");
    return 0;
}
