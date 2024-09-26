#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define L1D_SIZE (48 * 1024)            // 48KB for L1d cache (per core)
#define L2_SIZE  (1.25 * 1024 * 1024)   // 1.25MB for L2 cache (per core)
#define L3_SIZE  (24 * 1024 * 1024)     // 24MB for L3 cache (shared)
#define MEM_SIZE (64 * 1024 * 1024)     // 64MB for main memory (force access)
#define CACHE_LINE_SIZE (64)            // 64B cache line size
#define REPEAT 100000                   // Number of iterations for latency measurement

// Generate random access indices
void generate_random_indices(int *indices, int num_indices, size_t size)
{
    for (int i = 0; i < num_indices; i++)
    {
        // Generate random index within array bounds
        indices[i] = rand() % (size / sizeof(int));
    }
}

// // Function to measure the overhead of time measurement
// long long measure_overhead()
// {
//     // Measure overhead
//     struct timespec start, end;
//     clock_gettime(CLOCK_MONOTONIC, &start);
//     for (int i = 0; i < REPEAT; i++) {
//         // Empty loop
//     }
//     clock_gettime(CLOCK_MONOTONIC, &end);
//     return (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
// }

// Function to measure the average latency of reads or writes
void measure_latency(int *array, size_t size, const char *label, int is_write)
{    
    // long long overhead_ns = measure_overhead();

    struct timespec start, end;
    long long total_ns = 0;
    volatile int value;
    int random_indices[REPEAT];

    // Generate random indices for access
    generate_random_indices(random_indices, REPEAT, size);

    // Measure read/write latency
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int j = 0; j < REPEAT; j++)
    {
        if (is_write)
            array[random_indices[j]] ^= j << 13; // Pseudo-random write memory access
        else
            value = array[random_indices[j]]; // Random read memory access
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    total_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    // double avg_latency = (double)(total_ns - overhead_ns) / (double)REPEAT;
    double avg_latency = (double)total_ns / (double)REPEAT;

    printf("Avg %s Latency for %s: %.3f ns\n", is_write ? "Write" : "Read", label, avg_latency);
}

int main()
{
    // Allocate memory for L1d, L2, L3 cache, and main memory
    int *array_l1d = (int *) malloc(L1D_SIZE);    // L1d cache
    int *array_l2 = (int *) malloc(L2_SIZE);      // L2 cache
    int *array_l3 = (int *) malloc(L3_SIZE);      // L3 cache
    int *array_mem = (int *) malloc(MEM_SIZE);    // Main memory

    if (!array_l1d || !array_l2 || !array_l3 || !array_mem)
    {
        printf("Memory allocation failed\n");
        return 1;
    }

    // Initialize the arrays to avoid cold cache effects
    for (size_t i = 0; i < L1D_SIZE / sizeof(int); i++)
        array_l1d[i] = rand() % ((size_t)L1D_SIZE / sizeof(int));
    for (size_t i = 0; i < L2_SIZE / sizeof(int); i++)
        array_l2[i] = rand() % ((size_t)L2_SIZE / sizeof(int));
    for (size_t i = 0; i < L3_SIZE / sizeof(int); i++)
        array_l3[i] = rand() % ((size_t)L3_SIZE / sizeof(int));
    for (size_t i = 0; i < MEM_SIZE / sizeof(int); i++)
        array_mem[i] = rand() % ((size_t)MEM_SIZE / sizeof(int));

    // Measure latencies for L1d cache
    measure_latency(array_l1d, L1D_SIZE, "L1d Cache", 0); // Read latency for L1d
    measure_latency(array_l1d, L1D_SIZE, "L1d Cache", 1); // Write latency for L1d
    

    // Measure latencies for L2 cache
    measure_latency(array_l2, L2_SIZE, "L2 Cache", 0); // Read latency for L2
    measure_latency(array_l2, L2_SIZE, "L2 Cache", 1); // Write latency for L2

    // Measure latencies for L3 cache
    measure_latency(array_l3, L3_SIZE, "L3 Cache", 0); // Read latency for L3
    measure_latency(array_l3, L3_SIZE, "L3 Cache", 1); // Write latency for L3

    // Measure latencies for Main Memory
    measure_latency(array_mem, MEM_SIZE, "Main Memory", 0); // Read latency for Main Memory
    measure_latency(array_mem, MEM_SIZE, "Main Memory", 1); // Write latency for Main Memory

    // Free memory
    free(array_l1d);
    free(array_l2);
    free(array_l3);
    free(array_mem);

    return 0;
}