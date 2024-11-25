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
int current_time = 0;

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
    char line[256];
    int pid;
    int memory_size;
    int arrival_time;
    int total_burst_time;
    int io_frequency;
    int io_duration;
    int line_number = 0;

    // No valid file
    if (input_data_file == NULL) {
        printf("Error opening trace.txt\n");
        exit(EXIT_FAILURE);
    }

    // Looping through each line in the "input_data.txt" file:
    while (fgets(line, sizeof(line), input_data_file)) {
		
        // Parse the process data from each line:
        // Checking if the current line matches "%d, %d, %d, %d, %d, %d":
        if (sscanf(line, "%d, %d, %d, %d, %d, %d", &pid, &memory_size, &arrival_time, &total_burst_time, &io_frequency, &io_duration) == 6) {
            // Validate that none of the values is less than 0:
            if (pid >= 0 && memory_size >= 0 && arrival_time >= 0 && total_burst_time >= 0 && io_frequency >= 0 && io_duration >= 0) {
                
                // Storing valid process' data in PCB:
                pcb_fcfs_entries[process_counter].pid = pid;
                pcb_fcfs_entries[process_counter].memory_size = memory_size;
                pcb_fcfs_entries[process_counter].arrival_time = arrival_time;
                pcb_fcfs_entries[process_counter].total_burst_time = total_burst_time;
                pcb_fcfs_entries[process_counter].remaining_burst_time = total_burst_time;
                pcb_fcfs_entries[process_counter].io_frequency = io_frequency;
                pcb_fcfs_entries[process_counter].io_duration = io_duration;
                // Initially setting "process_status" to "NEW" for all processes:
                strcpy(pcb_fcfs_entries[process_counter].process_status, "NEW");
                
                // Incrementing process_counter ONLY for valid process' data:
                process_counter++;
            }
        } else {
            fprintf(stderr, "WARNING: Invalid data %d: %s\n", process_counter, line);
            continue;
        }
        line_number++;
    }

    //print_pcb_entries();
    fcfs_simulator();

    fclose(input_data_file);
}

void print_pcb_entries(void) {
	printf("\nCURRENT ENTRIES IN PCB.\n");
    for(int i = 0; i < process_counter; i++) {
        printf("PID: %d\n", pcb_fcfs_entries[i].pid);
		printf("memory size: %d\n", pcb_fcfs_entries[i].memory_size);
		printf("arrival time: %d\n", pcb_fcfs_entries[i].arrival_time);
		printf("total_cpu_time: %d\n", pcb_fcfs_entries[i].total_burst_time);
		printf("io frequency: %d\n", pcb_fcfs_entries[i].io_frequency);
		printf("io duration: %d\n", pcb_fcfs_entries[i].io_duration);
		printf("process' status: %s\n", pcb_fcfs_entries[i].process_status);
		printf("\n");
    }
}

void print_ready_queue_entries(void) {
	printf("\nCURRENT ENTRIES IN READY QUEUE.\n");
    for(int i = 0; i < ready_queue_size; i++) {
        printf("PID: %d\n", ready_queue[i].pid);
		printf("memory size: %d\n", ready_queue[i].memory_size);
		printf("arrival time: %d\n", ready_queue[i].arrival_time);
		printf("total_cpu_time: %d\n", ready_queue[i].total_burst_time);
		printf("io frequency: %d\n", ready_queue[i].io_frequency);
		printf("io duration: %d\n", ready_queue[i].io_duration);
		printf("process' status: %s\n", ready_queue[i].process_status);
		printf("Partition Allocated: %d\n", ready_queue[i].allocated_partition_number);
		printf("\n");
    }
}

void fcfs_simulator() {
    int completed_processes = 0;
    bool partition_allocated;

    while (completed_processes < process_counter)  {
    	
    	for (int i = 0; i < process_counter; i++) {
    	    if ((pcb_fcfs_entries[i].arrival_time <= current_time) && strcmp(pcb_fcfs_entries[i].process_status, "NEW") == 0) {
    	        printf("FLAG2\n");
	        	// Allocate memory partition for process:
    	    	partition_allocated = allocate_partition(i);
				
    	    	// If partition was successfully allocated:
    	    	if (partition_allocated) {
    	    		completed_processes++;
    	            // Mark process status as "READY" and add it to "ready_queue":
    	            strcmp(pcb_fcfs_entries[i].process_status, "READY");
                    // Storing current process that got memory allocated in a Process struct:
                    Process current_process = pcb_fcfs_entries[i];
    	    	    enqueue_ready_queue(current_process); // Enqueue current process
    	    	}
    	    	// Process was not allocated memory:
    	    	else {
    	    		// Process' status will stay marked as "NEW"
    	    		printf("Time %d: Could not allocate memory for process %d.\n", current_time, pcb_fcfs_entries[i].pid);
    	    	}
    	    }
        }
        // Calling scheudler to run processes in "ready_queue":
        schedule_fcfs_ready_queue();
    }
}

void enqueue_ready_queue(Process process_to_enqueue) {
    ready_queue[ready_queue_size++] = process_to_enqueue;
    // Changing process status' to READY:
    strcpy(process_to_enqueue.process_status, "READY");
    printf("\nAFTER ENQUEUING ready_queue.\n");
    print_ready_queue_entries();
}


Process dequeue_ready_queue() {
	Process empty_process = {-1, -1, -1, -1, -1, -1, "EMPTY"};

	if (ready_queue_size == 0) {
		printf("Ready queue is empty. Cannot dequeue.\n");
		return empty_process;
	}
	
	// Save the first element to return:
	Process dequeued_process = ready_queue[0];
	
	// Shift back all the processes after the first process:
	for (int i = 1; i < ready_queue_size; i++) {
	     ready_queue[i - 1] = ready_queue[i];
	}
	
	// Decrease ready_queue size;
	ready_queue_size--;

	printf("\nAFTER DEQUEUING ready_queue.\n");
	print_ready_queue_entries();

	return dequeued_process; 
}

void print_memory_partitions() {
    //
    for (int i = NUM_PARTITIONS - 1; i >= 0; i--) {	
        printf("PARTITION NUMBERat i=%d is %d\n", i, memory_partitions[i].partition_number); 
        printf("MEMORY PARTITION at i=%d is %d\n", i, memory_partitions[i].size);
        printf("MEMORY PARTITION status at i=%d is %d\n", i, memory_partitions[i].status);

    }
}
bool allocate_partition(unsigned int process_index) {
    int best_index = -1;
    int memory_required = pcb_fcfs_entries[process_index].memory_size;
    int pid = pcb_fcfs_entries[process_index].pid;
	printf("Memory Required-------------:%d\n", memory_required);
    // Find the best-fit partition - starting from smallest to largest:
    for (int i = NUM_PARTITIONS - 1; i >= 0; i--) {
    	printf("MEMORY PARTITION at i=%d is %d\n", i, memory_partitions[i].size);
        if(memory_partitions[i].size >= memory_required &&
            memory_partitions[i].status == -1) {
			// Memory partition successfully allocated for process:
			memory_partitions[i].status = pid; // Updating partitions' status with PID
			pcb_fcfs_entries[process_index].allocated_partition_number = i+1;
			memory_partitions[i].status == pcb_fcfs_entries[process_index].pid;
			return true;
        }
    }

    // No sutiable partition found -> return false
    return false;
}

void schedule_fcfs_ready_queue() {
	
	
	
	
	printf("Starting FCFS Scheduling...\n");
	// Looping through processes in ready_queue:
	while (ready_queue_size > 0) {
		// Call dequeue function to hand the first process in ready_queue to CPU:
		// Store this popped out process in "currently_processsing_process":
		Process	currently_processsing_process = dequeue_ready_queue();

        // Update currently_processing_process' status to "RUNNING":
		strcpy(currently_processsing_process.process_status, "RUNNING");
		
		// Begin CPU execution:
		int time_slice = currently_processsing_process.io_frequency;
		int remaining_cpu_burst = currently_processsing_process.remaining_burst_time;
		ready_queue_size--;
		// Begin CPU execution: 
		while (remaining_cpu_burst > 0) {
			if (remaining_cpu_burst <= time_slice) {
				// Process will finish executing without need to perform I/O

			} else {
				// Process will deal with WAITING for I/O:
                strcpy(currently_processsing_process.process_status, "WAITING");
                remaining_cpu_burst -= time_slice;

                // Modify current_time to run process until the I/O event (time_slice):
                current_time += time_slice;

                // Enqueue currently running process to the end of the ready_queue:

                // Call the fcfs_scheduler to run the first process in the "ready_queue":
			}
            
        }
    }
				
		
			
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
    if (argc < 2) {
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
