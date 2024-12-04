// interrupts_101225633_101205030_H
#include <stdio.h>
#include <stdbool.h>
#ifndef INTERRUPTS_101225633_101205030_H
#define INTERRUPTS_101225633_101205030_H

// Define macros
#define MAX_EVENTS 1000
#define MAX_EXTERNAL_FILES 100
#define MAX_READY_QUEUE_ENTRIES 100
#define MAX_PCB_ENTRIES 20
#define NUM_PARTITIONS 6
#define TOTAL_MEMORY_AVAILABLE 100

// Declare global variables
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
    unsigned int process_execution_end_time;
    bool process_last_cpu_burst;
    unsigned int io_frequency;
    unsigned int io_duration;
    unsigned int next_io_event; // Time when next i/o event will occur
    unsigned int io_event_finished; // Time when i/o event will finish
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
void print_ready_queue_entries(void);
void fcfs_simulator(void);
void sjf_simulator(void);
void enqueue_ready_queue(Process *process_to_enqueue);
Process* dequeue_ready_queue(void);
void dequeue_specific_element_ready_queue(Process* process_to_dequeue);
bool allocate_partition(unsigned int process_index);
void free_memory_partition(Process* process_to_free);
void schedule_fcfs_ready_queue(void);
void schedule_sjf_ready_queue(void);
Process* find_shortest_job(void);
void log_header(void);
void log_execution_transition(int time, Process *process, const char *old_state, const char *new_state);
void log_memory_status_transition(int current_time, int memory_used, int usable_free_memory, int partition_number, int process_pid, bool allocate_new_process);
#endif  // INTERRUPTS_101225633_101205030_H
