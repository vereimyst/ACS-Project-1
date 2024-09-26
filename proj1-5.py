import subprocess
import time
import numpy as np

# Function to perform light computations
def light_computation(num_operations):
    result = 1.0
    for _ in range(num_operations):
        result *= 1.0001  # Light computation (multiplication)
    return result

# Function to run perf command to measure TLB miss performance
def measure_perf(command, num_operations):
    # Use the available TLB miss events from TMA
    perf_command = [
        'perf', 'stat',
        '-e', 'tma_itlb_misses,tma_dtlb_load,tma_dtlb_store,tma_load_stlb_miss,tma_store_stlb_miss',
        command, str(num_operations)
    ]
    
    # Run perf command and capture output
    result = subprocess.run(perf_command, stderr=subprocess.PIPE, text=True)
    return result.stderr  # perf output goes to stderr

# Function to safely extract integer or float values from perf output
def extract_int_value(line, label):
    try:
        # Split the line and take the first part as the numeric value
        value_str = line.split()[0].replace(',', '')
        return int(value_str)
    except (IndexError, ValueError):
        print(f"Could not parse {label} from line: {line}")
        return 0  # Return 0 if the event is not available or cannot be parsed

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
        "itlb-misses": 0,
        "dtlb-load-misses": 0,
        "dtlb-store-misses": 0,
        "stlb-load-misses": 0,
        "stlb-store-misses": 0,
        "execution_time": 0.0
    }
    
    # Parse perf output
    for line in perf_output.splitlines():
        if "tma_itlb_misses" in line:
            perf_data["itlb-misses"] = extract_int_value(line, "ITLB Misses")
        elif "tma_dtlb_load" in line:
            perf_data["dtlb-load-misses"] = extract_int_value(line, "DTLB Load Misses")
        elif "tma_dtlb_store" in line:
            perf_data["dtlb-store-misses"] = extract_int_value(line, "DTLB Store Misses")
        elif "tma_load_stlb_miss" in line:
            perf_data["stlb-load-misses"] = extract_int_value(line, "STLB Load Misses")
        elif "tma_store_stlb_miss" in line:
            perf_data["stlb-store-misses"] = extract_int_value(line, "STLB Store Misses")
    
    # Calculate the time taken for the computation
    perf_data["execution_time"] = end_time - start_time
    
    return perf_data

# Run the experiment for different sizes of operations
num_experiments = 5
num_operations = 10**6  # Number of operations for light computation

# Collect average measurements
itlb_misses = []
dtlb_load_misses = []
dtlb_store_misses = []
stlb_load_misses = []
stlb_store_misses = []
execution_times = []

for _ in range(num_experiments):
    perf_data = run_experiment(num_operations)
    
    # Append TLB miss counts
    itlb_misses.append(perf_data["itlb-misses"])
    dtlb_load_misses.append(perf_data["dtlb-load-misses"])
    dtlb_store_misses.append(perf_data["dtlb-store-misses"])
    stlb_load_misses.append(perf_data["stlb-load-misses"])
    stlb_store_misses.append(perf_data["stlb-store-misses"])
    execution_times.append(perf_data["execution_time"])

# Output average measurements
print(f"Average ITLB Misses: {np.mean(itlb_misses)}")
print(f"Average DTLB Load Misses: {np.mean(dtlb_load_misses)}")
print(f"Average DTLB Store Misses: {np.mean(dtlb_store_misses)}")
print(f"Average STLB Load Misses: {np.mean(stlb_load_misses)}")
print(f"Average STLB Store Misses: {np.mean(stlb_store_misses)}")
print(f"Average Execution Time: {np.mean(execution_times)} seconds")
