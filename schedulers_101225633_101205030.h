// schedulers_101225633_101205030_H.h
#include <stdio.h>
#include <stdbool.h>
#ifndef SCHEDULERS_101225633_101205030_H
#define SCHEDULERS_101225633_101205030_H

// Define macros
#define MAX_EVENTS 1000
#define MAX_EXTERNAL_FILES 100
#define MAX_PCB_ENTRIES 20
#define KERNEL_MODE 1
#define USER_MODE 0
#define ONE_MILLISECONDS 1
#define SYSCALL_VECTOR_TABLE_POSITION 2
#define EXEC_VECTOR_TABLE_POSITION 3
#define NUM_PARTITIONS 6

// Declare global variables
extern int mode;
extern int event_count;
extern int current_time;
extern int current_mode;
extern FILE *log_file;
extern FILE *system_status_log_file;
extern FILE *vector_table_file;
extern int vector_table[25];
extern int syscall_random_durations[3];
extern int exec_random_durations[5];

// TraceEvent struct declaration
typedef struct {
    char event_type[10];
    int event_duration;
    char external_program[30];
} TraceEvent;

// MemoryPartition struct declaration
typedef struct {
    unsigned int partition_number;
    unsigned int size;
    char code[20]; 
    bool status; // 0 for unused, 1 for used
} MemoryPartition;

// ProgramEvent struct declaration
typedef struct {
    char event_type[10];
    int event_duration;
    int vector_table_position;
} ProgramEvent;

// PCB struct declaration
typedef struct {
    unsigned int pid;
    unsigned int memory_size;
    unsigned int arrival_time;
    unsigned int total_cpu_time;
    unsigned int io_frequency;
    unsigned int io_duration;
} PCB;

// ExternalFiles struct declaration
typedef struct {
    char program_name[30];
    unsigned int program_size;
} ExternalFiles;

// Declare arrays
extern TraceEvent trace_events[MAX_EVENTS];
extern ProgramEvent program_events[MAX_EVENTS];
extern MemoryPartition memory_partitions[NUM_PARTITIONS]; 

// Method declarations
void read_trace_file(const char *filename);
void read_vector_table(const char *filename);
void read_external_files(const char *filename);
void read_program_file(const char *filename);
void print_events(void);
void switch_mode(void);
void write_log_file(const char *message, int duration);
void trace_events_simulator(void);
void program_events_simulator(void);
void context_save(void);
void check_interrupt_priority(void);
void is_interrupt_masked(void);
void syscall_random_duration_generator(int syscall_random_durations[3], int max_duration);
void syscall_interrupt_handling(int event_duration, int vector_table_position);
void end_io_interrupt_handling(int event_duration, int vector_table_position);
void fork_handler(int event_duration);
void exec_handler(const char *external_file_name, int event_duration);
void fork_simulator(int event_duration);
void exec_simulator(const char *external_file_name, int event_duration);
void syscall_interrupt_simulator(int event_duration, int vector_table_position);
void end_io_interrupt_simulator(int event_duration, int vector_table_position);
void scheduler(unsigned int duration);
void create_init_process(void);
void copy_init_process(int copy_init_duration); 
void allocate_partition(unsigned int program_size, const char *program_name, unsigned int process_type);
void print_pcb_entries(unsigned int flag);
 
#endif  // SCHEDULERS_101225633_101205030_H
