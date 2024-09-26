#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MEM_SIZE (256 * 1024 * 1024)  // 256MB memory block, larger than typical cache
#define REPEAT 1000000                // Number of iterations

// Function to flush the cache line
void clflush(void *p) {
    asm volatile("clflush (%0)" :: "r"(p));
}

// Function to measure memory bandwidth for a given chunk size and read/write ratio
double measure_bandwidth(size_t chunk_size, double read_ratio) {
    struct timespec start, end;
    char *mem_block;
    size_t i;
    long total_data = MEM_SIZE;        // Total memory to access
    long iterations = total_data / chunk_size;

    // Allocate a large block of memory
    mem_block = (char *) malloc(total_data);
    if (!mem_block) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    // Initialize memory block
    for (i = 0; i < total_data; i++) {
        mem_block[i] = (char) i;
    }

    // Start timing memory access
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Perform read/write operations according to the read_ratio
    for (i = 0; i < iterations; i++) {
        volatile char temp;
        // Flush the cache line for every memory access to avoid cache hits
        clflush(&mem_block[i * chunk_size]);

        if ((double)rand() / RAND_MAX < read_ratio) {
            temp = mem_block[i * chunk_size];  // Perform read
        } else {
            mem_block[i * chunk_size] = (char)i;  // Perform write
        }
    }

    // Stop timing
    clock_gettime(CLOCK_MONOTONIC, &end);

    // Free memory block
    free(mem_block);

    // Calculate elapsed time in seconds
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;

    // Calculate bandwidth in GB/s
    double bandwidth = (double)(total_data) / (time_taken * 1e9);  // GB/s

    return bandwidth;
}

int main() {
    // Array of data chunk sizes to test
    size_t chunk_sizes[] = {64, 256, 512, 1024, 2048, 5096};
    int num_chunks = sizeof(chunk_sizes) / sizeof(chunk_sizes[0]);

    // Array of read-to-write ratios (from 100% read to 100% write)
    double read_ratios[] = {1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0};
    int num_ratios = sizeof(read_ratios) / sizeof(read_ratios[0]);

    // Open CSV file for writing
    FILE *csv_file = fopen("memory_bandwidth_results.csv", "w");
    if (!csv_file) {
        perror("Error opening CSV file");
        return 1;
    }

    // Write CSV headers
    fprintf(csv_file, "Chunk Size (bytes),Read Ratio,Write Ratio,Bandwidth (GB/s)\n");

    printf("Evaluating memory bandwidth for different data access granularities and read/write ratios:\n");
    printf("Memory Block Size: %d MB\n", MEM_SIZE / (1024 * 1024));

    // Iterate over each chunk size and each read/write ratio
    for (int i = 0; i < num_chunks; i++) {
        size_t chunk_size = chunk_sizes[i];
        printf("\nChunk size: %zu bytes\n", chunk_size);
        printf("Read/Write Ratio  |  Bandwidth (GB/s)\n");
        printf("--------------------------------------\n");

        for (int j = 0; j < num_ratios; j++) {
            double read_ratio = read_ratios[j];
            double bandwidth = measure_bandwidth(chunk_size, read_ratio);

            printf("%5.0f%% read, %5.0f%% write  |  %.2f GB/s\n", 
                   read_ratio * 100, (1.0 - read_ratio) * 100, bandwidth);

            // Write data to the CSV file
            fprintf(csv_file, "%zu,%.1f%%,%.1f%%,%.2f\n", 
                    chunk_size, read_ratio * 100, (1.0 - read_ratio) * 100, bandwidth);
        }
    }

    // Close the CSV file
    fclose(csv_file);

    printf("\nBandwidth data has been saved to 'memory_bandwidth_results.csv'\n");

    return 0;
}
