import subprocess
import time
import numpy as np

# Function to perform light computations
def light_computation(num_operations):
    result = 1.0
    for _ in range(num_operations):
        result *= 1.0001  # Light computation (multiplication)
    return result

# Function to run perf command to measure cache latency performance
def measure_perf(command, num_operations):
    # Adjust these events to measure L1, L2, and L3 cache miss latencies
    perf_command = [
        'perf', 'stat',
        '-e', 'tma_info_memory_load_miss_real_latency,tma_info_memory_oro_load_l2_miss_latency,tma_info_memory_oro_load_l3_miss_latency',
        command, str(num_operations)
    ]
    
    # Run perf command and capture output
    result = subprocess.run(perf_command, stderr=subprocess.PIPE, text=True)
    return result.stderr  # perf output goes to stderr

# Function to safely extract integer or float values from perf output
def extract_float_value(line, label):
    try:
        # Split the line and take the first part as the numeric value
        value_str = line.split()[0].replace(',', '')
        return float(value_str)
    except (IndexError, ValueError):
        print(f"Could not parse {label} from line: {line}")
        return 0.0  # Return 0.0 if the event is not available or cannot be parsed

# Main experiment function
def run_experiment(num_operations):
    # Start time
    start_time = time.time()
    
    # Perform the light computation
    light_computation(num_operations)
    
    # End time
    end_time = time.time()
    
    # Measure performance with perf
    perf_output = measure_perf('./computation_program', num_operations)
    
    # Initialize dictionary for performance data
    perf_data = {
        "L1-miss-latency": 0.0,
        "L2-miss-latency": 0.0,
        "L3-miss-latency": 0.0,
        "execution_time": 0.0
    }
    
    # Parse perf output
    for line in perf_output.splitlines():
        if "tma_info_memory_load_miss_real_latency" in line:
            perf_data["L1-miss-latency"] = extract_float_value(line, "L1 Miss Latency")
        elif "tma_info_memory_oro_load_l2_miss_latency" in line:
            perf_data["L2-miss-latency"] = extract_float_value(line, "L2 Miss Latency")
        elif "tma_info_memory_oro_load_l3_miss_latency" in line:
            perf_data["L3-miss-latency"] = extract_float_value(line, "L3 Miss Latency")
    
    # Calculate the time taken for the computation
    perf_data["execution_time"] = end_time - start_time
    
    return perf_data

# Run the experiment for different sizes of operations
num_experiments = 5
num_operations = 10**6  # Number of operations for light computation

# Collect average measurements
L1_latencies = []
L2_latencies = []
L3_latencies = []
execution_times = []

for _ in range(num_experiments):
    perf_data = run_experiment(num_operations)
    
    # Append available latency values
    L1_latencies.append(perf_data["L1-miss-latency"])
    L2_latencies.append(perf_data["L2-miss-latency"])
    L3_latencies.append(perf_data["L3-miss-latency"])
    execution_times.append(perf_data["execution_time"])

# Output average measurements
print(f"Average L1 Miss Latency: {np.mean(L1_latencies)} core cycles")
print(f"Average L2 Miss Latency: {np.mean(L2_latencies)} core cycles")
print(f"Average L3 Miss Latency: {np.mean(L3_latencies)} core cycles")
print(f"Average Execution Time: {np.mean(execution_times)} seconds")
