// SYSC 4001 - Assignment 3 - Part I - Lab L6
// Wajd Mariam #101225633
// Abed Qubbaj #101205030

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include "schedulers_101225633_101205030.h"

// Mode set to user mode by default:
int mode = USER_MODE;
int current_mode = USER_MODE; // Default Mode

// Array of Structs:
TraceEvent trace_events[MAX_EVENTS];
ExternalFiles external_files[MAX_EXTERNAL_FILES];
ProgramEvent program_events[MAX_EVENTS];
PCB pcb_fcfs_entries[MAX_PCB_ENTRIES];
MemoryPartition memory_partitions[NUM_PARTITIONS];

// Global Variables
char scheduler_mode[30];
char instructions[25][7];
char instruction[10];
int current_time = 0;

// Files:
FILE *execution_output_file;
FILE *memory_output_file;
FILE *vector_table_txt_file;

// Initializing an array to store the random durations of SYSCALL events:
int syscall_random_durations[3];

// Initializing counters:
static int trace_event_count = 0;
static int program_event_count = 0;
static int external_files_count = 0;
static int pcb_counter = 0;
static int save_time = 0;

// Defining memory_partitions array:
MemoryPartition memory_partitions[NUM_PARTITIONS] = {
    {1, 40, "free"},
    {2, 25, "free"},
    {3, 15, "free"},
    {4, 10, "free"},
    {5, 8, "free"},
    {6, 2, "free"}
};


// Reading "input_data.txt" file and storing valid data in PCB:
void read_input_data_file(const char *filename) {
    FILE *input_data_file = fopen(filename, "r");
    char line[100];
    unsigned process_counter = 0;
    int pid;
    int memory_size;
    int arrival_time;
    int total_cpu_time;
    int io_frequency;
    int io_duration;

    // No valid file
    if (input_data_file == NULL) {
        printf("Error opening trace.txt\n");
        exit(1);
    }

    // Looping through each line in the "input_data.txt" file:
    while (fgets(line, sizeof(line), input_data_file)) {

	process_counter++;
	
        // Parse the process data from each line:
        // Checking if the current line matches "%d, %d, %d, %d, %d, %d":
        if (sscanf(line, "%d, %d, %d, %d, %d, %d", &pid, &memory_size, &arrival_time, &total_cpu_time, &io_frequency, &io_duration) == 6) {
            // Validate that none of the values is less than 0:
            if (pid >= 0 && memory_size >= 0 && arrival_time >= 0 && total_cpu_time >= 0 && io_frequency >= 0 && io_duration >= 0) {
                // Incrementing process_counter ONLY for valid process' data:
                process_counter++;
                
                // Storing valid process' data in PCB:
                pcb_fcfs_entries[process_counter].pid = pid;
                pcb_fcfs_entries[process_counter].memory_size = memory_size;
                pcb_fcfs_entries[process_counter].arrival_time = arrival_time;
                pcb_fcfs_entries[process_counter].total_cpu_time = total_cpu_time;
                pcb_fcfs_entries[process_counter].io_frequency = io_frequency;
                pcb_fcfs_entries[process_counter].io_duration = io_duration;
            }
        } else {
            fprintf(stderr, "WARNING: Invalid data %d: %s\n", process_counter, line);
            continue;
        }
	
	// print contents of array of structs:
	for(int i = 0; i < process_counter; i++) {
	    printf("PID: %d\n", pcb_fcfs_entries[process_counter].pid);
	    printf("memory size: %d\n", pcb_fcfs_entries[process_counter].memory_size);
	    printf("arrival time: %d\n", pcb_fcfs_entries[process_counter].arrival_time);
	    printf("total_cpu_time: %d\n", pcb_fcfs_entries[process_counter].total_cpu_time);
	    printf("io frequency: %d\n", pcb_fcfs_entries[process_counter].io_frequency);
	    printf("io duration: %d\n", pcb_fcfs_entries[process_counter].io_duration);
	}
    }
    fclose(input_data_file);
}


// Reading "program#.txt" file and storing valid events in "program_file" array:
void read_program_file(const char *filename) {

	// Clearing previous variabls used for by other program#.txt file:
	memset(program_events, 0, sizeof(program_events));
	program_event_count = 0;
	
    FILE *program_file = fopen(filename, "r");
    char line[100];
    int line_number = 0;

    if (program_file == NULL) {
        printf("Error opening program#.txt\n");
        exit(1);
    }

    // Looping through each line in the file:
    while (fgets(line, sizeof(line), program_file)) {
        char event[30];
        int vector_number;
        int duration;
	
		line_number++;
	
        // Parse the event type, vector table number (if applicable), and duration from each line:
        // Checking if the current line matches "SYSCALL %d, %d":
        if (sscanf(line, "SYSCALL %d, %d", &vector_number, &duration) == 2) {
            // Format the event type as "SYSCALL <ISR number>"
            sprintf(event, "SYSCALL %d", vector_number);
            program_events[program_event_count].vector_table_position = vector_number;
            program_events[program_event_count].event_duration = duration;
        // Checking if the current line matches "END_IO %d, %d":
        } else if (sscanf(line, "END_IO %d, %d", &vector_number, &duration) == 2) {
            sprintf(event, "END_IO %d", vector_number);
            program_events[program_event_count].vector_table_position = vector_number;
            program_events[program_event_count].event_duration = duration;
        // Checking if the current line matches "CPU %d":
        } else if (sscanf(line, "CPU, %d", &duration) == 1){
            sprintf(event, "CPU");
            // Store only "duration"
            program_events[program_event_count].event_duration = duration; 
        } else {
            fprintf(stderr, "WARNING: Unrecognized form on line %d: %s\n", line_number, line);
            continue;
        }

        // Only store valid events (non-empty event_type and non-zero duration)
        if (strlen(event) > 0 && duration > 0) {
            strcpy(program_events[program_event_count].event_type, event);
            program_events[program_event_count].event_duration = duration;
            program_events[program_event_count].vector_table_position = vector_number;
            program_event_count++;  // Increment the event count only for valid events
        } else {
            printf("Skipping invalid event: %s\n", line);
        }
    }

    fclose(program_file);
}


// Reading "external_files.txt" and storing files information in "external_files" array:
void read_external_files(const char *filename) {
    FILE *external_files_file = fopen(filename, "r");
    
    char line[100];
    int line_number = 0;
    unsigned int program_number;
    unsigned int program_size;
    char full_program_name[30];

    if (external_files_file == NULL) {
        printf("Error opening external_files.txt\n");
        exit(1);
    }
    
    // Looping through each line in the file:
    while (fgets(line, sizeof(line), external_files_file)) {
        if (sscanf(line, "program%d, %d", &program_number, &program_size) == 2) {
            // Formatting full program name:
            snprintf(full_program_name, sizeof(full_program_name), "program%d.txt", program_number);
            strcpy(external_files[external_files_count].program_name, full_program_name);
            external_files[external_files_count].program_size = program_size;
            external_files_count++;
        }
    }
    fclose(external_files_file);
}


// Reading "vector_table.txt" and storing instructions in "instructions" array:
void read_vector_table(const char *filename) {
    FILE *vector_table_file = fopen(filename, "r");

    int i = 0;
    char buffer[100];
    
    if (vector_table_file == NULL) {
        printf("Error opening vector_table.txt\n");
        exit(1);
    }
    
    while (i < 25 && fgets(buffer, sizeof(buffer), vector_table_file) != NULL) {
        // Storing each line initially in "buffer" and then copying it to "instructions[]"
        strncpy(instructions[i], buffer, sizeof(instructions[i]) - 1);
        i++;
    }
    fclose(vector_table_file);
} 


void switch_mode() {
    if (mode == USER_MODE) {
        mode = KERNEL_MODE;
        write_log_file("switch to kernel mode", ONE_MILLISECONDS);
    } else if (mode == KERNEL_MODE){
        mode = USER_MODE;
        write_log_file("IRET", ONE_MILLISECONDS);
        save_time = current_time;
    }
}


// Writing to log file "exeuction#.txt":
void write_log_file(const char *message, int duration) {
	if (log_file == NULL) {
        fprintf(stderr, "Error: log_file is not open.\n");
        return;
    }
    fprintf(log_file, "%d, %d, %s\n", current_time, duration, message);
    current_time += duration;  // Increment current time by the event duration
}

/**
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

// Simulating events from "trace.txt" file:
void trace_events_simulator() {
    int i = 0;
    while (i < trace_event_count) {
        if (strncmp(trace_events[i].event_type, "FORK", 4) == 0) {
            fork_simulator(trace_events[i].event_duration);
        } else if (strncmp(trace_events[i].event_type, "EXEC", 4) == 0) {
            exec_simulator(trace_events[i].external_program, trace_events[i].event_duration);
        } else {
            printf("Unknown event type: %s\n", trace_events[i].event_type);
        }
        i++;
    }
}


// Simulating events from "program#.txt" file:
void program_events_simulator() {
    int i = 0;
    while (i < program_event_count) {
        if (strcmp(program_events[i].event_type, "CPU") == 0) {
            // Log CPU execution event
            write_log_file("CPU", program_events[i].event_duration);
        } else if (strncmp(program_events[i].event_type, "SYSCALL", 7) == 0) {
            // Handle SYSCALL event:
            syscall_interrupt_simulator(program_events[i].event_duration, program_events[i].vector_table_position);
        } else if (strncmp(program_events[i].event_type, "END_IO", 6) == 0) {
            // Handle END_IO event:
            end_io_interrupt_simulator(program_events[i].event_duration, program_events[i].vector_table_position);
        } else {
            printf("Unknown event type: %s\n", program_events[i].event_type);
        }
        i++;
    }
}


// "Context Save" simulator
void context_save() {
    int context_save_duration = rand() % 3 +1;  
    write_log_file("context saved", context_save_duration);
}


// Random duration generator used for syscall events:
void syscall_random_duration_generator(int syscall_random_durations[3], int max_duration) {
    // Randomally generating 3 numbers that add up to "max_duration":
    // And adding the random numbers to "syscall_random_durations" array for usage.
    syscall_random_durations[0] = rand() % (max_duration - 2) + 1;
    syscall_random_durations[1] = rand() % (max_duration - syscall_random_durations[0] - 1) + 1;
    syscall_random_durations[2] = max_duration - (syscall_random_durations[1] + syscall_random_durations[0]);
}


// "Check priority of interrupt" simulator:
void check_interrupt_priority() {
    write_log_file("check priority of interrupt", ONE_MILLISECONDS);
}


// "Check if interrupt is masked" simulator:
void is_interrupt_masked() {
    write_log_file("check if masked", ONE_MILLISECONDS);
}


void syscall_interrupt_handling(int event_duration, int vector_table_position){
	char log_message[300];
    // Finding instructions stored in memory address associated with vector table position:
    strcpy(instruction, instructions[vector_table_position]);

    // Calculating the memory position with the associated vector:
    // Multiplying by 2 because it is 2-bytes:
    int memory_position_decimal = vector_table_position * 2;
    
    // Formatting log message:
    sprintf(log_message, "find vector %d in memory position %#06x", vector_table_position, memory_position_decimal);
    write_log_file(log_message, ONE_MILLISECONDS);
    
    sprintf(log_message, "load address %s into the PC", instruction);
    write_log_file(log_message, ONE_MILLISECONDS);

    // Function to generate random numbers to assign as durations for the tasks below
    syscall_random_duration_generator(syscall_random_durations, event_duration);

    // Remaining of SYSCALL processes, with durations calculated randomally and sum up to total "event_duration" of SYSCALL event:
    write_log_file("SYSCALL: run the ISR", syscall_random_durations[0]); 
    write_log_file("transfer data", syscall_random_durations[1]); 
    write_log_file("Check for errors", syscall_random_durations[2]);
}


void end_io_interrupt_handling(int event_duration, int vector_table_position) {
	char log_message[300];
    // Finding instructions stored in memory address associated with vector table position:
    strcpy(instruction, instructions[vector_table_position]);

    // Calculating the memory position with the associated vector:
    // Multiplying by 2 because it is 2-bytes:
    int memory_position_decimal = vector_table_position * 2;
    
    // Formatting log message:
    sprintf(log_message, "find vector %d in memory position %#06x", vector_table_position, memory_position_decimal);
    write_log_file(log_message, ONE_MILLISECONDS);
    
    sprintf(log_message, "load address %s into the PC", instruction);
    write_log_file(log_message, ONE_MILLISECONDS);

    write_log_file("END_IO", event_duration);
}


void fork_handler(int event_duration) {
	char log_message[300];
	// "copy_init_duration" range is 3 < copy_init_duration < event_duration
	unsigned int copy_init_duration = 3 + rand() % (event_duration - 5 + 1);
    // Finding instructions stored in memory address associated with vector table position:
    strcpy(instruction, instructions[SYSCALL_VECTOR_TABLE_POSITION]);

    // Calculating the memory position with the associated vector:
    // Multiplying by 2 because it is 2-bytes:
    int memory_position_decimal = SYSCALL_VECTOR_TABLE_POSITION * 2;

    // Formatting log message:
    sprintf(log_message, "find vector %d in memory position %#06x", SYSCALL_VECTOR_TABLE_POSITION, memory_position_decimal);
    write_log_file(log_message, ONE_MILLISECONDS);

    sprintf(log_message, "load address %s into the PC", instruction);
    write_log_file(log_message, ONE_MILLISECONDS);

    // Copying parent PCB to child PCB - duplicating parent process "init" with a different PID
    copy_init_process(copy_init_duration);

    // Calling scheduler
    // Passing the remaining time of FORK process (event_duration - copy_init_duration) to scheduler:
    scheduler(event_duration - copy_init_duration);

}


void exec_handler(const char *external_file_name, int event_duration) {

    unsigned int random_number = (rand() % 10) + 1;
    char current_program_name[30];
    unsigned int current_program_size;
    char log_message[300];
    int external_file_size;
    
    // Finding instructions stored in memory address associated with vector table position:
    strcpy(instruction, instructions[EXEC_VECTOR_TABLE_POSITION]);

    // Calculating the memory position with the associated vector:
    // Multiplying by 2 because it is 2-bytes:
    int memory_position_decimal = EXEC_VECTOR_TABLE_POSITION * 2;

    // Formatting log message:
    sprintf(log_message, "find vector %d in memory position %#06x", EXEC_VECTOR_TABLE_POSITION, memory_position_decimal);
    write_log_file(log_message, ONE_MILLISECONDS);

    sprintf(log_message, "load address %s into the PC", instruction);
    write_log_file(log_message, ONE_MILLISECONDS);

    // Get program#.txt size from "external_files.txt":
    for (int i = 0; i < MAX_EXTERNAL_FILES; i++) {
    	if (strcmp(external_files[i].program_name, external_file_name) == 0) {
    	    strcpy(current_program_name, external_files[i].program_name);
    	    current_program_size = external_files[i].program_size;
    	    external_file_size = external_files[i].program_size;
    	    sprintf(log_message, "EXEC: load %s of size %dMB", external_files[i].program_name, external_file_size);
   		}
    }

    // EXEC: loading program#.txt to PCB :
    write_log_file(log_message, 10);
    
    
    // Allocating partition for program#.txt above:
    allocate_partition(current_program_size, current_program_name, -1);
    
    scheduler(10);
    
    switch_mode();
    
    // Executing program#.txt:
    read_program_file(current_program_name);
    // Simulating events from program#.txt file
    program_events_simulator();

}


// Simulation FORK events from "trace.txt" file:
void fork_simulator(int event_duration) {
    switch_mode();
    context_save();
    fork_handler(event_duration);
    switch_mode();
}


void exec_simulator(const char *external_file_name, int event_duration) {
    switch_mode();
    context_save();
    exec_handler(external_file_name, event_duration);
}


// Simulating SYSCALL events from "program#.txt" file:
void syscall_interrupt_simulator(int event_duration, int vector_table_position){
    switch_mode();       // Switch to kernel mode
    context_save();      // Save the context
    syscall_interrupt_handling(event_duration, vector_table_position); // Handles the interrupt's processes
    switch_mode();       // Switch back to user mode after handling
}


// Simulating END_IO events:
void end_io_interrupt_simulator(int event_duration, int vector_table_position) {
    check_interrupt_priority(); // Checking priority of the interrupt
    is_interrupt_masked();  // Check if the interrupt is masked or non-masked
    switch_mode();  // Switch to kernel mode
    context_save();  // Save the context
    end_io_interrupt_handling(event_duration, vector_table_position);  // Handles the interrupt's processes
    switch_mode();       // Switch back to user mode after handling
}


void scheduler(unsigned int duration) {
    write_log_file("scheduler called", duration);
}


void create_init_process() {
    // Allocate memory position for "init" process of size 1 MB
    allocate_partition(1, "init", 0);
}


/**
void allocate_partition(unsigned int program_size, const char *program_name, unsigned int process_type) {
    int best_index = -1;
    char log_message[100];

    int counter;
    counter = pcb_counter;

    // Find the best-fit partition
    for (int i = 0; i < NUM_PARTITIONS; i++) {
        if(strcmp(memory_partitions[i].code, "free") == 0 && // Checking if current partition is marked as "free"
            memory_partitions[i].size >= program_size &&  // Checking if current partition's size is > program size 
            // Checking if best_index == -1 (partition has not be assigned) OR if current partition is less than 
            // memory_partiton @best_index:
            (best_index == -1 || memory_partitions[i].size < memory_partitions[best_index].size)) { 
            
            best_index = i;
        }
    }

	// process_type = 0 is "init" process
	if (process_type == 0) {
    	if (best_index != -1) {
    		strcpy(memory_partitions[best_index].code, program_name);

			// Update the PCB table with the partition information for the process:
			if (pcb_counter < MAX_PCB_ENTRIES) {
    			//write_log_file(log_message2, 6);
			    pcb_entries[pcb_counter].pid = pcb_counter;
			    strcpy(pcb_entries[pcb_counter].program_name, program_name);
			    pcb_entries[pcb_counter].partition_number = memory_partitions[best_index].partition_number;
			    pcb_entries[pcb_counter].program_size = program_size;

			    pcb_counter++;
			    print_pcb_entries(1);
			  
			} else {
				printf("Error: PCB table is full. Cannot add new process.\n");
			}
		} else {
		printf("Error: No suitable partition available for program %s requiring %d MB.\n", program_name, program_size);
		}
	// process_type = 1 is any other process (i.e. program#.txt)
	} else if (process_type == -1) {
		int counter = pcb_counter;
		if (best_index != -1) {
    		strcpy(memory_partitions[best_index].code, program_name);

		sprintf(log_message, "found partition %d with %dMB of space", memory_partitions[best_index].partition_number, memory_partitions[best_index].size);
			write_log_file(log_message, 10);
    		sprintf(log_message, "partition %d marked as occupied", memory_partitions[best_index].partition_number);
    		write_log_file(log_message, 10);

			// Update the PCB table with the partition information for the process:
			if (pcb_counter < MAX_PCB_ENTRIES) {
				write_log_file("updating PCB with new information", 6);
			    pcb_entries[counter].pid = pcb_counter;
			    strcpy(pcb_entries[counter].program_name, program_name);
			    pcb_entries[counter].partition_number = memory_partitions[best_index].partition_number;
			    pcb_entries[counter].program_size = program_size;

			    
			    pcb_counter++;
			    print_pcb_entries(1);
			  
			} else {
				printf("Error: PCB table is full. Cannot add new process.\n");
			}
		} else {
		printf("Error: No suitable partition available for program %s requiring %d MB.\n", program_name, program_size);
		}
	}
}*/


int main(int argc, char *argv[]) {

    srand(time(NULL));  // Seed for random number generation

    // READING FILES
    // Reading arguments from "test#.sh" file
    if (argc < 3) {
        printf("Usage: %s <tracefile> <executionfile>\n", argv[0]);
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
    memory_output_file = fopen(argv[3], "w");
    if (memory_output_file == NULL) {
        printf("Error opening %s\n", argv[3]);
        return 1;
    }
    
    // Use the fourth argument as the scheduler used
    //scheduler_mode = argv[4];

    read_external_files(argv[4]);
    read_vector_table(argv[5]);

    // Initialize the init process (PID 0) with :
    create_init_process();
	

    // Simulating events from trace file: FORK, EXEC
    trace_events_simulator();
    
    // Close the log file
    fclose(execution_output_file);
    fclose(memory_output_file);
    printf("Simulation complete. Check execution files for details.\n");

    return 0;
}
