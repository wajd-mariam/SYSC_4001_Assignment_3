// SYSC 4001 - Assignment 3 - Part I - Lab L6
// Wajd Mariam #101225633
// Abed Qubbaj #101205030

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "schedulers_101225633_101205030.h"


// Array of Structs:
Process pcb_fcfs_entries[MAX_PCB_ENTRIES];
Process ready_queue[MAX_READY_QUEUE_ENTRIES];
MemoryPartition memory_partitions[NUM_PARTITIONS];

// Global Variables
char scheduler_mode[30];

// Files:
FILE *execution_output_file;
FILE *memory_output_file;
FILE *vector_table_txt_file;


// Initializing counters:
static unsigned int process_counter = 0;
static unsigned int ready_queue_size = 0;

static int pcb_counter = 0;
static int save_time = 0;

// Defining memory_partitions array:
MemoryPartition memory_partitions[NUM_PARTITIONS] = {
    {1, 40, -1},
    {2, 25, -1},
    {3, 15, -1},
    {4, 10, -1},
    {5, 8, -1},
    {6, 2, -1}
};


// Reading "input_data.txt" file and storing valid data in PCB:
void read_input_data_file(const char *filename) {
    FILE *input_data_file = fopen(filename, "r");
    char line[100];
    int pid;
    int memory_size;
    int arrival_time;
    int total_burst_time;
    int io_frequency;
    int io_duration;

    // No valid file
    if (input_data_file == NULL) {
        printf("Error opening trace.txt\n");
        exit(1);
    }

    // Looping through each line in the "input_data.txt" file:
    while (fgets(line, sizeof(line), input_data_file)) {

        // Parse the process data from each line:
        // Checking if the current line matches "%d, %d, %d, %d, %d, %d":
        if (sscanf(line, "%d, %d, %d, %d, %d, %d", &pid, &memory_size, &arrival_time, &total_burst_time, &io_frequency, &io_duration) == 6) {
            // Validate that none of the values is less than 0:
            if (pid >= 0 && memory_size >= 0 && arrival_time >= 0 && total_burst_time >= 0 && io_frequency >= 0 && io_duration >= 0) {
                // Incrementing process_counter ONLY for valid process' data:
                process_counter++;
                // Storing valid process' data in PCB:
                pcb_fcfs_entries[process_counter].pid = pid;
                pcb_fcfs_entries[process_counter].memory_size = memory_size;
                pcb_fcfs_entries[process_counter].arrival_time = arrival_time;
                pcb_fcfs_entries[process_counter].total_burst_time = total_burst_time;
                pcb_fcfs_entries[process_counter].io_frequency = io_frequency;
                pcb_fcfs_entries[process_counter].io_duration = io_duration;
                // Initially setting "process_status" to "NEW" for all processes:
                strcpy(pcb_fcfs_entries[process_counter].process_status, "NEW");
            }
        } else {
            fprintf(stderr, "WARNING: Invalid data %d: %s\n", process_counter, line);
            continue;
        }
    }

    //print_pcb_entries();

	fcfs_simulator();

    fclose(input_data_file);
}

void print_pcb_entries(void) {
    for(int i = 0; i < process_counter; i++) {
        printf("PID: %d\n", pcb_fcfs_entries[process_counter].pid);
		printf("memory size: %d\n", pcb_fcfs_entries[process_counter].memory_size);
		printf("arrival time: %d\n", pcb_fcfs_entries[process_counter].arrival_time);
		printf("total_cpu_time: %d\n", pcb_fcfs_entries[process_counter].total_burst_time);
		printf("io frequency: %d\n", pcb_fcfs_entries[process_counter].io_frequency);
		printf("io duration: %d\n", pcb_fcfs_entries[process_counter].io_duration);
		printf("process' status: %s\n", pcb_fcfs_entries[process_counter].process_status);
    }
}

void fcfs_simulator() {
    int current_time = 0;
    int completed_processes = 0;
    bool partition_allocated;

    while (completed_processes < process_counter)  {
    	for (int i = 0; i < process_counter; i++) {
    	    if (pcb_fcfs_entries[i].arrival_time <= current_time &&
    	        strcmp(pcb_fcfs_entries[i].process_status, "NEW")) {
	        	// Allocate memory partition for process:	    	        
    	    	partition_allocated = allocate_partition(i);

    	    	// If partition was successfully allocated:
    	    	if (partition_allocated) {
    	    		completed_processes++;
    	            // Mark process status as "READY" and add it to "ready_queue":
    	            strcmp(pcb_fcfs_entries[i].process_status, "READY");
    	    	    enqueue_ready_queue(i); // Enqueue current process	
    	    	} 
    	    	// Process was not allocated memory:
    	    	else {
    	    		// Process' status will stay marked as "NEW"
    	    	} 
    	    }
        }
    }
    
    

	// Calling scheudler to run processes in "ready_queue":
}	

void enqueue_ready_queue(unsigned int process_index) {
    ready_queue[ready_queue_size++] = pcb_fcfs_entries[process_index];
    printf("\nAFTER ENQUEUING ready_queue.\n");
    print_pcb_entries();
}

Process dequeue_read_queue() {
	
	if (ready_queue_size == 0) {
		printf("Ready queue is empty. Cannot dequeue.\n");
		return;
	}
}


bool allocate_partition(unsigned int process_index) {
    int best_index = -1;
    int memory_required = pcb_fcfs_entries[process_index].memory_size;
    int pid = pcb_fcfs_entries[process_index].pid;

    // Find the best-fit partition
    for (int i = 0; i < NUM_PARTITIONS; i++) {
        if(memory_partitions[i].size >= memory_required && 
            memory_partitions[i].status == -1) {
			// Memory partition successfully allocated for process:
			memory_partitions[i].status = pid; // Updating partitions' status with PID
			pcb_fcfs_entries[i].allocated_partition_number = i;
			
			return true;
        }
    }
    
    // No sutiable partition found -> return false
    return false;
}


/**
// Writing to log file "exeuction#.txt":
void write_log_file(const char *message, int duration) {
    if (execution_output_file == NULL) {
        fprintf(stderr, "Error: log_file is not open.\n");
        return;
    }
    fprintf(execution_output_file, "%d, %d, %s\n", current_time, duration, message);
    current_time += duration;  // Increment current time by the event duration
}  


// Writing to log file "system_status.txt":
void print_pcb_entries(unsigned int flag) {

	unsigned int pcb_entries_limit;
	char log_message[100];

	if (flag == 0) {
		pcb_entries_limit = pcb_counter + 1;
	} else if (flag == 1) {
		pcb_entries_limit = pcb_counter;
	}

    fprintf(system_status_log_file, "Contents of PCB Table:\n");
    sprintf(log_message, "Save Time: %d", save_time);
    fprintf(system_status_log_file, "%s\n", log_message);
    fprintf(system_status_log_file, "+-------------------------------------------------+\n");
    fprintf(system_status_log_file, "| PID | Program Name      | Partition | Size (MB) |\n");
    fprintf(system_status_log_file, "+-------------------------------------------------+\n");

    for (int i = 0; i < pcb_entries_limit; i++) {
        fprintf(system_status_log_file, "| %-3d | %-16s | %-9d | %-9d |\n",
        pcb_entries[i].pid,
        pcb_entries[i].program_name,
        pcb_entries[i].partition_number,
        pcb_entries[i].program_size);
    }

    fprintf(system_status_log_file, "-------------------------------------------------\n");
    fprintf(system_status_log_file, "!-------------------------------------------------!\n");
    fprintf(system_status_log_file, "!-------------------------------------------------!\n");
}

*/


int main(int argc, char *argv[]) {

    srand(time(NULL));  // Seed for random number generation

    // READING FILES
    // Reading arguments from "test#.sh" file
    if (argc < 3) {
        printf("Usage: %s <inputDataFile> <executionOutputFile>\n", argv[0]);
        return 1;
    }
    
    // Use the first argument as the input_data.txt file
    read_input_data_file(argv[1]);
    
    // Use the second argument as the execution.txt file
    execution_output_file = fopen(argv[2], "w");
    if (execution_output_file == NULL) {
        printf("Error opening %s\n", argv[2]);
        return 1;
    }
    
    // Use the third argument as the memory_status.txt file
    /**memory_output_file = fopen(argv[3], "w");
    if (memory_output_file == NULL) {
        printf("Error opening %s\n", argv[3]);
        return 1;
    }*/
    
    // Use the fourth argument as the scheduler used
    //scheduler_mode = argv[4];
  
    // Close the log file
    fclose(execution_output_file);
    //fclose(memory_output_file);
    printf("Simulation complete. Check execution files for details.\n");

    return 0;
}
