import re
from collections import defaultdict

# Sample benchmark data (you can read from file instead)
benchmark_data = """
CPU_Pinned/MortonCode/Small/1/iterations:40              26.0 ms        0.406 ms           40
# ... (rest of your benchmark data)
"""

def parse_benchmark_line(line):
    # Parse a single benchmark line
    pattern = r'CPU_Pinned/(\w+)/(\w+)/(\d+)/iterations:\d+\s+(\d+\.?\d*)\s*ms'
    match = re.match(pattern, line.strip())
    if match:
        algorithm, core_type, threads, time = match.groups()
        return {
            'algorithm': algorithm,
            'core_type': core_type,
            'threads': int(threads),
            'time': float(time)
        }
    return None

def create_table(data):
    # Group data by algorithm
    algorithms = defaultdict(lambda: defaultdict(dict))
    
    for line in data.strip().split('\n'):
        result = parse_benchmark_line(line)
        if result:
            algorithms[result['algorithm']][result['core_type']][result['threads']] = result['time']
    
    # Print table
    print("\n=== Benchmark Results (Time in ms) ===\n")
    
    for algorithm in sorted(algorithms.keys()):
        print(f"\n{algorithm}:")
        print("-" * 60)
        print(f"{'Core Type':<10} | {'1 thread':<10} | {'2 threads':<10} | {'3 threads':<10} | {'4 threads':<10}")
        print("-" * 60)
        
        for core_type in ['Small', 'Medium', 'Big']:
            if core_type in algorithms[algorithm]:
                times = algorithms[algorithm][core_type]
                row = f"{core_type:<10} |"
                
                for thread_count in range(1, 5):
                    if thread_count in times:
                        row += f" {times[thread_count]:<10.2f}|"
                    else:
                        row += " " + "-"*8 + " |"
                        
                print(row)
        print("-" * 60)

# If reading from stdin
import sys
input_data = sys.stdin.read()
create_table(input_data)




