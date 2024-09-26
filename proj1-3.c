#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MEM_SIZE (256 * 1024 * 1024)  // 256MB memory block, larger than typical cache
#define REPEAT 500  // Number of operations per thread (reduced to capture precise latencies)

typedef struct {
    char *mem_block;          // Memory block to access
    long operations;          // Number of operations to perform
    double *latency;          // Pointer to store latency result
    int is_read;              // 1 for read, 0 for write
} thread_data_t;

// Thread function to perform memory access operations (read or write)
void *memory_access_thread(void *arg) {
    thread_data_t *data = (thread_data_t *)arg;
    char *mem_block = data->mem_block;
    long operations = data->operations;
    int is_read = data->is_read;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Perform memory access (read or write)
    for (long i = 0; i < operations; i++) {
        volatile char temp;
        if (is_read) {
            temp = mem_block[i % MEM_SIZE];  // Simulate read operation
        } else {
            mem_block[i % MEM_SIZE] = temp;  // Simulate write operation
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    // Calculate latency (time per operation)
    double time_taken = (end.tv_sec - start.tv_sec) + 
                        (end.tv_nsec - start.tv_nsec) / 1e9;
    *data->latency = (time_taken / operations) * 1e6;  // Latency in microseconds

    pthread_exit(NULL);
}

// Function to measure latency and throughput for a given number of threads (reads or writes)
void measure_latency_throughput(int num_threads, int is_read, FILE *csv_file) {
    pthread_t threads[num_threads];
    thread_data_t thread_data[num_threads];
    double latencies[num_threads];
    double total_latency = 0;
    long operations_per_thread = REPEAT / num_threads;
    char *mem_block = (char *)malloc(MEM_SIZE);

    // Initialize memory block
    for (long i = 0; i < MEM_SIZE; i++) {
        mem_block[i] = (char)(i % 256);
    }

    // Start threads to simulate memory access (reads or writes)
    for (int i = 0; i < num_threads; i++) {
        thread_data[i].mem_block = mem_block;
        thread_data[i].operations = operations_per_thread;
        thread_data[i].latency = &latencies[i];
        thread_data[i].is_read = is_read;
        pthread_create(&threads[i], NULL, memory_access_thread, (void *)&thread_data[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total_latency += latencies[i];
    }

    // Calculate average latency and throughput
    double avg_latency = total_latency / num_threads;
    double throughput = (double)(REPEAT) / (avg_latency / 1e6);  // Throughput in operations per second

    // Output results
    if (is_read) {
        printf("%d threads (Read): Average Latency = %.4f us, Throughput = %.4f ops/sec\n", 
               num_threads, avg_latency, throughput);
        fprintf(csv_file, "%d,read,%.4f,%.4f\n", num_threads, avg_latency, throughput);
    } else {
        printf("%d threads (Write): Average Latency = %.4f us, Throughput = %.4f ops/sec\n", 
               num_threads, avg_latency, throughput);
        fprintf(csv_file, "%d,write,%.4f,%.4f\n", num_threads, avg_latency, throughput);
    }

    // Free memory block
    free(mem_block);
}

// Function to measure combined read and write operations
void measure_combined_latency_throughput(int num_threads, FILE *csv_file) {
    pthread_t threads[num_threads * 2];
    thread_data_t thread_data[num_threads * 2];
    double latencies[num_threads * 2];
    double total_latency = 0;
    long operations_per_thread = REPEAT / num_threads;
    char *mem_block = (char *)malloc(MEM_SIZE);

    // Initialize memory block
    for (long i = 0; i < MEM_SIZE; i++) {
        mem_block[i] = (char)(i % 256);
    }

    // Start threads to simulate combined memory access (reads and writes)
    for (int i = 0; i < num_threads * 2; i++) {
        thread_data[i].mem_block = mem_block;
        thread_data[i].operations = operations_per_thread / 2;
        thread_data[i].latency = &latencies[i];
        thread_data[i].is_read = (i < num_threads);  // First half reads, second half writes
        pthread_create(&threads[i], NULL, memory_access_thread, (void *)&thread_data[i]);
    }

    // Wait for threads to finish
    for (int i = 0; i < num_threads * 2; i++) {
        pthread_join(threads[i], NULL);
        total_latency += latencies[i];
    }

    // Calculate average latency and throughput
    double avg_latency = total_latency / (num_threads * 2);
    double throughput = (double)(REPEAT) / (avg_latency / 1e6);  // Throughput in operations per second

    // Output results
    printf("%d threads (Combined Read/Write): Average Latency = %.4f us, Throughput = %.4f ops/sec\n", 
           num_threads, avg_latency, throughput);
    fprintf(csv_file, "%d,combined,%.4f,%.4f\n", num_threads, avg_latency, throughput);

    // Free memory block
    free(mem_block);
}

int main() {
    FILE *csv_file = fopen("memory_latency_throughput.csv", "w");
    if (!csv_file) {
        perror("Error opening CSV file");
        return 1;
    }

    // Write CSV headers
    fprintf(csv_file, "Threads,Operation Type,Latency (us),Throughput (ops/sec)\n");

    printf("Demonstrating the trade-off between read/write latency and throughput with increasing threads\n");
    printf("Using memory size = %d MB\n\n", MEM_SIZE / (1024 * 1024));

    // Simulate read latency/throughput from 1 to 16 threads
    for (int num_threads = 1; num_threads <= 16; num_threads++) {
        measure_latency_throughput(num_threads, 1, csv_file);  // Measure read performance
    }

    printf("\n");

    // Simulate write latency/throughput from 1 to 16 threads
    for (int num_threads = 1; num_threads <= 16; num_threads++) {
        measure_latency_throughput(num_threads, 0, csv_file);  // Measure write performance
    }

    printf("\n");

    // Simulate combined read and write latency/throughput from 1 to 16 threads
    for (int num_threads = 1; num_threads <= 16; num_threads++) {
        measure_combined_latency_throughput(num_threads, csv_file);  // Measure combined read/write performance
    }

    fclose(csv_file);
    printf("\nResults saved to 'memory_latency_throughput.csv'\n");

    return 0;
}
