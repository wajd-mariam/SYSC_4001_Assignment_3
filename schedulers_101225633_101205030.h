// schedulers_101225633_101205030_H.h
#include <stdio.h>
#include <stdbool.h>
#ifndef SCHEDULERS_101225633_101205030_H
#define SCHEDULERS_101225633_101205030_H

// Define macros
#define MAX_EVENTS 1000
#define MAX_EXTERNAL_FILES 100
#define MAX_READY_QUEUE_ENTRIES 100
#define MAX_PCB_ENTRIES 20
#define NUM_PARTITIONS 6

// Declare global variables
extern int mode;
extern int event_count;
extern int current_time;
extern int current_mode;
extern FILE *log_file;
extern FILE *system_status_log_file;
extern FILE *vector_table_file;


// Process struct containting process' information
typedef struct {
    unsigned int pid;
    unsigned int memory_size;
    unsigned int arrival_time;
    unsigned int total_burst_time;
    unsigned int remaining_burst_time;
    unsigned int io_frequency;
    unsigned int io_duration;
    unsigned int allocated_partition_number;
    char process_status[30];
} Process;


// MemoryPartition struct declaration
typedef struct {
    unsigned int partition_number;
    unsigned int size;
    unsigned int status; // -1 if unused, PID if used
} MemoryPartition;

// Declare arrays
extern MemoryPartition memory_partitions[NUM_PARTITIONS]; 

// Method declarations
void read_input_data_file(const char *filename);
void print_pcb_entries(void);
void fcfs_simulator(void);
void enqueue_ready_queue(unsigned int process_index);
Process dequeue_read_queue(void);
bool allocate_partition(unsigned int process_index);




 
#endif  // SCHEDULERS_101225633_101205030_H
