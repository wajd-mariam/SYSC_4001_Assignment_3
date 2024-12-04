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
#include "interrupts_101225633_101205030.h"


// Array of Structs:
Process pcb_fcfs_entries[MAX_PCB_ENTRIES];
Process* ready_queue[MAX_READY_QUEUE_ENTRIES]; // Ready queue will store pointers to processes stored in "pcb_fcfs_entries" array of structs
MemoryPartition memory_partitions[NUM_PARTITIONS];

// Global Variables
char scheduler_mode[30];
bool process_currently_running = false; // initially no processes are running in CPU


// Files:
FILE *execution_output_file;
FILE *memory_status_output_file;
FILE *vector_table_txt_file;


// Initializing counters:
static unsigned int current_time = 0;
static unsigned int process_counter = 0;
static unsigned int ready_queue_size = 0;
static unsigned int completed_processes = 0;
static unsigned int total_available_memory = 0;
static unsigned int memory_used = 0;
static unsigned int total_free_memory = 100;
static unsigned int usable_free_memory = 100;
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

// Defining "partitions_state" array of integers (used in "log_memory_status_transition"):
int partitions_state[6] = {-1, -1, -1, -1, -1, -1};


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
                pcb_fcfs_entries[process_counter].process_last_cpu_burst = false; // initially set to false for all processes
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
    printf("Finishing reading file\n");

    if (strcmp(scheduler_mode, "FCFS")== 0) {
        printf("Starting FCFS Scheduling...\n");
        fcfs_simulator();
    } else if (strcmp(scheduler_mode, "EP") == 0) {
	    printf("Starting SJF Scheduling...\n");
        sjf_simulator();
    } else {
        printf("INVALID SCHEDULER TYPE; CHOOSE EITHER FCFS OR EP");
    }

    fclose(input_data_file);
}


void fcfs_simulator() {
    while (completed_processes < process_counter)  {

    	for (int i = 0; i < process_counter; i++) {
            Process *current_process = &pcb_fcfs_entries[i];

            // Checking arrival times for each process in ready queue:
    	    if ((current_process->arrival_time <= current_time) && strcmp(current_process->process_status, "NEW") == 0) {
	        	// Allocate memory partition for process:
    	    	bool partition_allocated = allocate_partition(i);

    	    	// If a memory partition was successfully allocated:
    	    	if (partition_allocated) {
                    // No processs are being executed by CPU -> Enqueue to ready queue and call scheduler: 
                    if (!process_currently_running) {
                        enqueue_ready_queue(current_process); // Enqueue current process
                        // Mark process status as "READY":
                        strcpy(current_process->process_status, "READY");
                    } else { // There is a process running in the CPU -> Enqueue to ready queue only:
                        enqueue_ready_queue(current_process);
                    }
                    log_execution_transition(current_time, current_process, "NEW", "READY");
    	    	}
    	    	// Process was not allocated memory:
    	    	else {
    	    		// Process' status will stay marked as "NEW" until a memory partition is available to store the process in it:
    	    		printf("Time %d: Could not allocate memory for process %d.\n", current_time, pcb_fcfs_entries[i].pid);
    	    	}
            // Currently running process needs an I/O:
    	    } else if ((strcmp(current_process->process_status, "RUNNING") == 0) && current_process->next_io_event == current_time && !current_process->process_last_cpu_burst) {
                process_currently_running = false;
                strcpy(current_process->process_status, "WAITING");
                log_execution_transition(current_time, current_process, "RUNNING", "WAITING");             
            } 
            // Enqueing process after it finished waiting for I/O:
            else if ((strcmp(current_process->process_status, "WAITING") == 0) && current_process->io_event_finished == current_time) {
                if (!(current_process->process_last_cpu_burst)) {
                    log_execution_transition(current_time, current_process, "WAITING", "READY");
                    enqueue_ready_queue(current_process);
                }
            }
            else if ((strcmp(current_process->process_status, "RUNNING") == 0) && current_time == current_process->process_execution_end_time) {
                if (current_process->process_last_cpu_burst) {
                    log_execution_transition(current_time, current_process, "RUNNING", "TERMINATED");
                    strcpy(current_process->process_status, "TERMINATED");
                    free_memory_partition(current_process);
                    process_currently_running = false;
                    completed_processes++;
                }
            }
        }

        if (!process_currently_running && ready_queue_size > 0) {
            schedule_fcfs_ready_queue();
        }
        
        current_time++;
    }

}


void sjf_simulator() {
    while (completed_processes < process_counter)  {

    	for (int i = 0; i < process_counter; i++) {
            Process *current_process = &pcb_fcfs_entries[i];

            // Checking arrival times for each process in ready queue:
    	    if ((current_process->arrival_time <= current_time) && strcmp(current_process->process_status, "NEW") == 0) {
	        	// Allocate memory partition for process:
    	    	bool partition_allocated = allocate_partition(i);

    	    	// If a memory partition was successfully allocated:
    	    	if (partition_allocated) {
                    // No processs are being executed by CPU -> Enqueue to ready queue and call scheduler: 
                    if (!process_currently_running) {
                        enqueue_ready_queue(current_process); // Enqueue current process
                        // Mark process status as "READY":
                        strcpy(current_process->process_status, "READY");
                    } else { // There is a process running in the CPU -> Enqueue to ready queue only:
                        enqueue_ready_queue(current_process);
                    }
                    log_execution_transition(current_time, current_process, "NEW", "READY");
    	    	}
    	    	// Process was not allocated memory:
    	    	else {
    	    		// Process' status will stay marked as "NEW" until a memory partition is available to store the process in it:
    	    		printf("Time %d: Could not allocate memory for process %d.\n", current_time, pcb_fcfs_entries[i].pid);
    	    	}
            // Currently running process needs an I/O:
    	    } else if ((strcmp(current_process->process_status, "RUNNING") == 0) && current_process->next_io_event == current_time && !current_process->process_last_cpu_burst) {
                process_currently_running = false;
                strcpy(current_process->process_status, "WAITING");
                log_execution_transition(current_time, current_process, "RUNNING", "WAITING");             
            } 
            // Enqueing process after it finished waiting for I/O:
            else if ((strcmp(current_process->process_status, "WAITING") == 0) && current_process->io_event_finished == current_time) {
                if (!(current_process->process_last_cpu_burst)) {
                    log_execution_transition(current_time, current_process, "WAITING", "READY");
                    enqueue_ready_queue(current_process);
                }
            }

            // Process is currently running in its last CPU burst (it does not need i/o) -> it will finish executing and terminates:
            else if ((strcmp(current_process->process_status, "RUNNING") == 0) && current_time == current_process->process_execution_end_time) {
                if (current_process->process_last_cpu_burst) {
                    log_execution_transition(current_time, current_process, "RUNNING", "TERMINATED");
                    strcpy(current_process->process_status, "TERMINATED");
                    // Freeing up memory partition after process is terminated:
                    free_memory_partition(current_process);
                    process_currently_running = false;
                    completed_processes++;
                }
            }
        }

        if (!process_currently_running && ready_queue_size > 0) {
            schedule_sjf_ready_queue();
        }
        
        current_time++;
    }
}


void enqueue_ready_queue(Process *process_to_enqueue) {
    if (ready_queue_size >= MAX_READY_QUEUE_ENTRIES) {
        printf("Error: Ready queue is full! Cannot enqueue PID %d.\n", process_to_enqueue->pid);
        return;
    }
    ready_queue[ready_queue_size++] = process_to_enqueue;
    // Changing process status' to READY:
    strcpy(process_to_enqueue->process_status, "READY");
}


// Removes the first element in "ready_queue":
Process* dequeue_ready_queue(void) {
    printf("ready_queue_size before dequeuing: %d\n", ready_queue_size);
	if (ready_queue_size <= 0) {
		printf("Ready queue is empty. Cannot dequeue.\n");
		return NULL;
	}
	
	// Save the first element to return:
	Process *dequeued_process = ready_queue[0];
	
	// Shift back all the processes after the first process:
	for (int i = 1; i < ready_queue_size; i++) {
	     ready_queue[i - 1] = ready_queue[i];
	}
	
	// Decrease ready_queue size;
	ready_queue_size--;

	return dequeued_process; 
}


// Removes a specific element in "ready_queue":
void dequeue_specific_element_ready_queue(Process* process_to_dequeue) {
    if (ready_queue_size <= 0) {
		printf("Ready queue is empty. Cannot dequeue.\n");
		return;
	}

    // Find the position of the process in the "ready_queue":
    int process_position = -1;
    for (int i = 0; i < ready_queue_size; i++) {
        if(ready_queue[i] == process_to_dequeue) {
            process_position = i;
            break;
        }
    }

    
    // Checking if process was not found in "ready_queue":
    if (process_position == -1) {
        printf("Process was not found in ready_queue. Cannot dequeue.");
        return;
    }

    
    // If process was found successfully -> need to shift up all processes after it in ready_queue
    for (int i = process_position + 1; i <ready_queue_size; i++ ){
        ready_queue[i - 1] = ready_queue[i];
    }

    // Decrement ready_queue size after dequeuing an element:
    ready_queue_size--;
}


bool allocate_partition(unsigned int process_index) {
    int best_index = -1;
    int memory_required = pcb_fcfs_entries[process_index].memory_size;
    int pid = pcb_fcfs_entries[process_index].pid;

    // Find the best-fit partition - starting from smallest to largest partition (best-fit):
    for (int i = NUM_PARTITIONS - 1; i >= 0; i--) {
        if(memory_partitions[i].size >= memory_required &&
            memory_partitions[i].status == -1) {
			// Memory partition successfully allocated for process:
			memory_partitions[i].status = pid; // Updating partitions' status with PID
			pcb_fcfs_entries[process_index].allocated_partition_number = i+1;
			memory_partitions[i].status == pcb_fcfs_entries[process_index].pid;

            memory_used += memory_required;
            total_free_memory -= memory_required;
            usable_free_memory -= memory_partitions[i].size;
            log_memory_status_transition(current_time, memory_used, usable_free_memory, i, memory_partitions[i].status, true);
			return true;
        }
    }
    // No sutiable partition found -> return false
    return false;
}


void free_memory_partition(Process* process_to_free) {
    if (process_to_free->allocated_partition_number == -1) {
        printf("Process %d does not have a memory parition allocated.\n", process_to_free->pid);
        return;
    }

    // Get the partition index (partiton number - 1 because arrays are 0-indexed):
    int partition_index = process_to_free->allocated_partition_number - 1;

    // Update memory partition status as available (-1):
    memory_partitions[partition_index].status = -1;

    // 
    memory_used -= process_to_free->memory_size;
    total_free_memory += process_to_free->memory_size;
    usable_free_memory += memory_partitions[partition_index].size;

    log_memory_status_transition(current_time, memory_used, usable_free_memory, partition_index, memory_partitions[partition_index].status, false);
}


void schedule_fcfs_ready_queue() {
	Process *currently_running_process = dequeue_ready_queue();

    // Process is valid; perform operations:
    if (currently_running_process->pid != -1) {
        // Modify currently_running_process status to "RUNNING":
        strcpy(currently_running_process->process_status, "RUNNING");
        // Log transition change:
        log_execution_transition(current_time, currently_running_process,"READY", "RUNNING");

        process_currently_running = true;

        if (currently_running_process->remaining_burst_time <= currently_running_process->io_frequency) {
            currently_running_process->process_execution_end_time = currently_running_process->remaining_burst_time + current_time;
            currently_running_process->process_last_cpu_burst = true;
        }

        // Update process's struct variables:
        currently_running_process->next_io_event = current_time + currently_running_process->io_frequency;
        currently_running_process->io_event_finished = currently_running_process->next_io_event + currently_running_process->io_duration;
        currently_running_process->remaining_burst_time = currently_running_process->remaining_burst_time - currently_running_process->io_frequency;

    } else {
        printf("INVALID PROCESS-EXITING\n");
    }
}


void schedule_sjf_ready_queue() {
    
    // Looking at current ready queue and finding shortest job first:
    Process* shortest_job = find_shortest_job();

    dequeue_specific_element_ready_queue(shortest_job);

    // Process is valid; perform operations:
    if (shortest_job->pid != -1) {
        // Modify currently_running_process status to "RUNNING":
        strcpy(shortest_job->process_status, "RUNNING");
        // Log transition change:
        log_execution_transition(current_time, shortest_job,"READY", "RUNNING");

        process_currently_running = true;

        if (shortest_job->remaining_burst_time <= shortest_job->io_frequency) {
            shortest_job->process_execution_end_time = shortest_job->remaining_burst_time + current_time;
            shortest_job->process_last_cpu_burst = true;
        }

        // Update process's struct variables:
        shortest_job->next_io_event = current_time + shortest_job->io_frequency;
        shortest_job->io_event_finished = shortest_job->next_io_event + shortest_job->io_duration;
        shortest_job->remaining_burst_time = shortest_job->remaining_burst_time - shortest_job->io_frequency;

    } else {
        printf("INVALID PROCESS-EXITING\n");
    }
}


Process* find_shortest_job() {

    // Assuming shortest job is the first element in the ready queue
    Process* shortest_job = ready_queue[0];

    for (int i = 1; i < ready_queue_size; i++) {
        if(ready_queue[i]->remaining_burst_time < shortest_job->remaining_burst_time) {
            shortest_job = ready_queue[i];
        }

        // If remaining bursts are equal -> select the Process with the lower PID:
        else if (ready_queue[i]->remaining_burst_time == shortest_job->remaining_burst_time) {
            if (ready_queue[i]->pid < shortest_job->pid) {
                shortest_job = ready_queue[i];
            }
        }
    }
    
    return shortest_job;
}


void log_execution_header() {
    if (execution_output_file == NULL) {
        fprintf(stderr, "Error: Log file is not open.\n");
        return;
    }

    fprintf(execution_output_file,
            "+--------------------+-----+-------------+------------+---------------+--------------\n"
            "| Time of Transition | PID | Old State   | New State  | REM CPU BURST |Exec End Time \n"
            "+--------------------+-----+-------------+------------+---------------+--------------\n");
}


void log_memory_status_header() {
    if (memory_status_output_file == NULL) {
        fprintf(stderr, "Error: Memory Status file is not open.\n");
        return;
    }

    fprintf(memory_status_output_file,
            "+---------------+-------------+--------------------------+-------------------+--------------------+\n"
            "| Time of Event | Memory Used | Partition State          | Total Free Memory | Usable Free Memory | \n"
            "+---------------+-------------+--------------------------+-------------------+--------------------+\n"
            "| 0             | 0           | -1, -1, -1, -1, -1 ,-1   | 100               | 100                |\n");
}


void log_execution_transition(int time, Process *process, const char *old_state, const char *new_state) {
    if (execution_output_file == NULL) {
        fprintf(stderr, "Error: Log file is not open.\n");
        return;
    }

    if (process == NULL) {
        fprintf(stderr, "Error: Process pointer is NULL");
        return;
    }

    // Print to log file
    fprintf(execution_output_file, "| %-18d | %-3d | %-11s | %-11s | %-11d | %-11d | \n", time, process->pid, old_state, new_state, process->remaining_burst_time, process->process_execution_end_time);
}


void log_memory_status_transition(int current_time, int memory_used, int usable_free_memory, int partition_number, int process_pid, bool allocate_new_process) {

    char partition_state_string_array[100] = "";

    if (execution_output_file == NULL) {
        fprintf(stderr, "Error: Log file is not open.\n");
        return;
    }

    if (allocate_new_process) {
        // Modifying "partitions_state" array of integers to reflect latest changes:
        partitions_state[partition_number] = process_pid;
    } else {
        partitions_state[partition_number] = -1;
    }

    // looping through "partition_state array" of integers and appending elements to "partition_state_string_array":
    for (int i = 0; i < 6; i++ ){
        char temp[10];
        sprintf(temp, "%d", partitions_state[i]);
        strcat(partition_state_string_array, temp);

        if (i < 5) {
            strcat(partition_state_string_array, ", ");
        }
    }

    // Print to log file
    fprintf(memory_status_output_file, "| %-13d | %-11d | %-24s | %-17d | %-18d | \n", current_time, memory_used, partition_state_string_array, total_free_memory, usable_free_memory);
}


int main(int argc, char *argv[]) {

    srand(time(NULL));  // Seed for random number generation

    // READING FILES
    // Reading arguments from "test#.sh" file
    if (argc != 3) {
        printf("Usage: %s <input_file> <scheduler_type>\n", argv[0]);
        printf("Scheduler types: fcfs, sjf\n");
        return 1;
    }
    
    // Use the second argument as the execution.txt file
    execution_output_file = fopen("execution_101225633_101205030.txt", "w");
    if (execution_output_file == NULL) {
        return 1;
    }

    memory_status_output_file = fopen("memory_status_101225633_101205030.txt", "w");
    if (memory_status_output_file == NULL) {
        return 1;
    }

    // Taking argument 4 as input for selecting scheduler type
    // Two valid options fcfs (first come first served) & sjf (shortest job first)
    strcpy(scheduler_mode, argv[2]);

    // Printing log header in "execution_output_file.txt"
    log_execution_header();

    // Printing log header in "memory_status.txt"
    log_memory_status_header();

    // Use the first argument as the input_data_#.txt file and passing it as parameter to "read_input_data_file" method:
    read_input_data_file(argv[1]);    

    // Close the log file
    fclose(execution_output_file);
    fclose(memory_status_output_file);
    //fclose(memory_output_file);
    printf("Simulation complete. Check execution files for details.\n");

    return 0;
}
