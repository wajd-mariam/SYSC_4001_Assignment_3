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
Process* ready_queue[MAX_READY_QUEUE_ENTRIES]; // Ready queue will store pointers to processes stored in "pcb_fcfs_entries" array of structs
MemoryPartition memory_partitions[NUM_PARTITIONS];

// Global Variables
char scheduler_mode[30];
bool process_currently_running = false; // initially no processes are running in CPU


// Files:
FILE *execution_output_file;
FILE *memory_output_file;
FILE *vector_table_txt_file;


// Initializing counters:
static unsigned int current_time = 0;
static unsigned int process_counter = 0;
static unsigned int ready_queue_size = 0;
static unsigned int completed_processes = 0;
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
    print_pcb_entries();
    printf("Finishing reading file\n");
    fcfs_simulator();

    fclose(input_data_file);
}


void print_pcb_entries(void) {
	printf("\nCURRENT ENTRIES IN PCB.\n");
    printf("Process counter: %d\n", process_counter);
    for(int i = 0; i < process_counter; i++) {
        Process *current_process = &pcb_fcfs_entries[i];
        printf("PID: %d\n", current_process->pid);
		printf("memory size: %d\n", current_process->memory_size);
		printf("arrival time: %d\n", current_process->arrival_time);
		printf("total_cpu_time: %d\n", current_process->total_burst_time);
		printf("io frequency: %d\n", current_process->io_frequency);
		printf("io duration: %d\n", current_process->io_duration);
		printf("process' status: %s\n", current_process->process_status);
		printf("Partition Allocated: %d\n", current_process->allocated_partition_number);
		printf("\n");
    }
}


void print_ready_queue_entries(void) {
	printf("\nCURRENT ENTRIES IN READY QUEUE.\n");
    for(int i = 0; i < ready_queue_size; i++) {
        Process *current_process = ready_queue[i];
        printf("PID: %d\n", current_process->pid);
		printf("memory size: %d\n", current_process->memory_size);
		printf("arrival time: %d\n", current_process->arrival_time);
		printf("total_cpu_time: %d\n", current_process->total_burst_time);
		printf("io frequency: %d\n", current_process->io_frequency);
		printf("io duration: %d\n", current_process->io_duration);
		printf("process' status: %s\n", current_process->process_status);
		printf("Partition Allocated: %d\n", current_process->allocated_partition_number);
		printf("\n");
    }
}


void fcfs_simulator() {
    
    while (completed_processes < process_counter)  {
        //printf("COMPLETED PROCESSES COUNTER VALUE AT FCFS SIM: %d\n", completed_processes);
        //printf("CURRENT TIME VALUE AT FCFS SIM: %d\n", current_time);

    	for (int i = 0; i < process_counter; i++) {
            Process *current_process = &pcb_fcfs_entries[i];
            
            printf("GOING IN FOR LOOP---------------------\n");
            printf("PROCESS PID: %d\n", current_process->pid);
            printf("Process arrival time:%d\n", current_process->arrival_time);
            printf("PROCESS BACK FROM WAITING FROM I/O at: %d\n", current_process->io_event_finished);
            printf("CURRENT PROCESS STATUS: %s\n", current_process->process_status);
            printf("PROCESS REM CPU BURST: %d\n", current_process->remaining_burst_time);
            printf("CURRENT TIME VALUE INSIDE LOOP ################################################# %d\n", current_time);

            // Checking arrival times for each process in ready queue:
    	    if ((current_process->arrival_time <= current_time) && strcmp(current_process->process_status, "NEW") == 0) {
                printf("\nADDING A NEW PROCESS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
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
                    log_transition(current_time, current_process, "NEW", "READY");
    	    	}
    	    	// Process was not allocated memory:
    	    	else {
    	    		// Process' status will stay marked as "NEW"
    	    		printf("Time %d: Could not allocate memory for process %d.\n", current_time, pcb_fcfs_entries[i].pid);
    	    	}
            // Currently running process needs an I/O:
    	    } else if ((strcmp(current_process->process_status, "RUNNING") == 0) && current_process->next_io_event == current_time) {
                printf("\nCURRENT PROCESS %d NEEDS I/O!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", current_process->pid);
                // Updating "io_event_finished" value for this process's struct:
                //current_process->io_event_finished = current_time + current_process->io_frequency;
                process_currently_running = false;
                strcpy(current_process->process_status, "WAITING");
                log_transition(current_time, current_process, "RUNNING", "WAITING");             
            } 
            // Enqueing process after it finished waiting for I/O:
            else if ((strcmp(current_process->process_status, "WAITING") == 0) && current_process->io_event_finished == current_time) {
                printf("\nPROCESS FINISHED I/O!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
                log_transition(current_time, current_process, "WAITING", "READY");
                enqueue_ready_queue(current_process);
            }
            // Checking if process has completed exectuion: 
            else if ((strcmp(current_process->process_status, "RUNNING") == 0) && current_process->remaining_burst_time == 0) {
                printf("\nPROCESS HAS TERMINIATED!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
                log_transition(current_time, current_process, "RUNNING", "TERMINATED");  
                completed_processes++;
            }
        }

        if (!process_currently_running && ready_queue_size > 0) {
            schedule_fcfs_ready_queue();
        }
        
        current_time++;
        // Safety check: Ensure completed_processes is updated:
        printf("Completed processes: %d\n", completed_processes);
    }

}


void enqueue_ready_queue(Process *process_to_enqueue) {
    printf("READY QUEUE SIZE: %d\n", ready_queue_size); 
    if (ready_queue_size >= MAX_READY_QUEUE_ENTRIES) {
        printf("Error: Ready queue is full! Cannot enqueue PID %d.\n", process_to_enqueue->pid);
        return;
    }
    ready_queue[ready_queue_size++] = process_to_enqueue;
    // Changing process status' to READY:
    strcpy(process_to_enqueue->process_status, "READY");

    printf("\nAFTER ENQUEUING ready_queue.\n");
    print_ready_queue_entries();
}


Process* dequeue_ready_queue(void) {
    //Process empty_process = {-1, -1, -1, -1, -1, -1};
    //strcpy(empty_process.process_status, "EMPTY");

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

	printf("\nAFTER DEQUEUING ready_queue.\n");
	print_ready_queue_entries();

	return dequeued_process; 
}


bool allocate_partition(unsigned int process_index) {
    int best_index = -1;
    int memory_required = pcb_fcfs_entries[process_index].memory_size;
    int pid = pcb_fcfs_entries[process_index].pid;
	printf("Memory Required-------------:%d\n", memory_required);
    // Find the best-fit partition - starting from smallest to largest partition:
    for (int i = NUM_PARTITIONS - 1; i >= 0; i--) {
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
	Process *currently_running_process = dequeue_ready_queue();

    // Process is valid; perform operations:
    if (currently_running_process->pid != -1) {
        // Modify currently_running_process status to "RUNNING":
        strcpy(currently_running_process->process_status, "RUNNING");
        // Log transition change:
        log_transition(current_time, currently_running_process,"READY", "RUNNING");

        process_currently_running = true;

        // Update process's struct variables:
        currently_running_process->next_io_event = current_time + currently_running_process->io_frequency;
        currently_running_process->io_event_finished = currently_running_process->next_io_event + currently_running_process->io_duration;
        currently_running_process->remaining_burst_time = currently_running_process->remaining_burst_time - currently_running_process->io_frequency;
        printf("PROCESS ID is: %d+++++++++++++++++++++++++++++++++++++++++++++++++++++\n", currently_running_process->pid);
        printf("Currently running process next_io_event: %d\n", currently_running_process->next_io_event);
        printf("Currently running process finishes waiting for I/O at : %d\n", currently_running_process->io_event_finished);
        printf("Currently running process remaining CPU burst time : %d\n", currently_running_process->remaining_burst_time);

    } else {
        printf("INVALID PROCESS-EXITING\n");
    }
    //completed_processes++;
    
}


void log_header() {
    if (execution_output_file == NULL) {
        fprintf(stderr, "Error: Log file is not open.\n");
        return;
    }

    fprintf(execution_output_file,
            "+--------------------+-----+-------------+------------+---------------+\n"
            "| Time of Transition | PID | Old State   | New State  | REM CPU BURST |\n"
            "+--------------------+-----+-------------+------------+---------------+\n");
}


void log_transition(int time, Process *process, const char *old_state, const char *new_state) {
    if (execution_output_file == NULL) {
        fprintf(stderr, "Error: Log file is not open.\n");
        return;
    }

    if (process == NULL) {
        fprintf(stderr, "Error: Process pointer is NULL");
        return;
    }

    // Print to log file
    fprintf(execution_output_file, "| %-18d | %-3d | %-11s | %-11s | %-11d |\n", time, process->pid, old_state, new_state, process->remaining_burst_time);
}



int main(int argc, char *argv[]) {

    srand(time(NULL));  // Seed for random number generation

    // READING FILES
    // Reading arguments from "test#.sh" file
    if (argc < 2) {
        printf("Usage: %s <inputDataFile> <executionOutputFile>\n", argv[0]);
        return 1;
    }

    // Use the first argument as the input_data.txt file
    
    // Use the second argument as the execution.txt file
    execution_output_file = fopen(argv[2], "w");
    if (execution_output_file == NULL) {
        printf("Error opening %s\n", argv[2]);
        return 1;
    }
    // Printing log header in "execution_output_file"
    log_header();

    read_input_data_file(argv[1]);
    

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
