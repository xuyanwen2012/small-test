import re
import sys
import os
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict

def parse_benchmark_line(line):
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

def create_charts(data):
    # Create data directory if it doesn't exist
    os.makedirs("./data", exist_ok=True)
    
    # Group data by algorithm
    algorithms = defaultdict(lambda: defaultdict(dict))
    
    for line in data.strip().split('\n'):
        result = parse_benchmark_line(line)
        if result:
            algorithms[result['algorithm']][result['core_type']][result['threads']] = result['time']

    # Color scheme for different core types
    colors = {'Small': '#FF9999', 'Medium': '#66B2FF', 'Big': '#99FF99'}
    
    for algorithm in algorithms:
        # Create a figure with two subplots side by side
        fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(15, 6))
        fig.suptitle(f'{algorithm} Performance Analysis', fontsize=16)
        
        # Prepare data for plotting
        core_types = ['Small', 'Medium', 'Big']
        thread_counts = list(range(1, 5))
        
        # Bar Chart
        x = np.arange(len(thread_counts))
        width = 0.25
        multiplier = 0
        
        for core_type in core_types:
            times = []
            for thread_count in thread_counts:
                time = algorithms[algorithm].get(core_type, {}).get(thread_count, 0)
                times.append(time)
            
            offset = width * multiplier
            rects = ax1.bar(x + offset, times, width, label=core_type, color=colors[core_type])
            multiplier += 1
        
        # Customize bar chart
        ax1.set_ylabel('Time (ms)')
        ax1.set_xlabel('Number of Threads')
        ax1.set_title('Performance by Core Type and Thread Count')
        ax1.set_xticks(x + width)
        ax1.set_xticklabels(thread_counts)
        ax1.legend()
        
        # Line Plot
        for core_type in core_types:
            times = []
            valid_threads = []
            for thread_count in thread_counts:
                time = algorithms[algorithm].get(core_type, {}).get(thread_count, None)
                if time is not None and time > 0:
                    times.append(time)
                    valid_threads.append(thread_count)
            
            if times:  # Only plot if we have data
                ax2.plot(valid_threads, times, 'o-', label=core_type, color=colors[core_type], 
                        linewidth=2, markersize=8)
        
        # Customize line plot
        ax2.set_ylabel('Time (ms)')
        ax2.set_xlabel('Number of Threads')
        ax2.set_title('Scaling with Thread Count')
        ax2.grid(True, linestyle='--', alpha=0.7)
        ax2.legend()
        
        # Add minor gridlines to line plot
        ax2.grid(True, which='minor', linestyle=':', alpha=0.4)
        ax2.minorticks_on()
        
        # Adjust layout and save
        plt.tight_layout()
        
        # Save plot in the data directory
        output_path = os.path.join("./data", f'{algorithm}_benchmark_charts.png')
        plt.savefig(output_path, dpi=300, bbox_inches='tight')
        plt.close()

        # Also save the raw data as CSV
        csv_path = os.path.join("./data", f'{algorithm}_data.csv')
        with open(csv_path, 'w') as f:
            f.write("Core Type,Threads,Time (ms)\n")
            for core_type in core_types:
                for thread_count in thread_counts:
                    time = algorithms[algorithm].get(core_type, {}).get(thread_count, '')
                    if time:
                        f.write(f"{core_type},{thread_count},{time}\n")

# Read from stdin
if __name__ == "__main__":
    input_data = sys.stdin.read()
    create_charts(input_data)
