#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <time.h>

#define NUM_TA 5
#define NUM_STUDENTS 20
#define MAX_MARK 10

// Shared memory structure
typedef struct {
    int student_list[NUM_STUDENTS]; // List of student IDs
    int current_index;              // Shared index
} SharedData;

// Function to simulate marking delay
void random_delay(int min, int max) {
    sleep(rand() % (max - min + 1) + min);
}

// Marking task for each process
void marking_task(int ta_id, SharedData *shared_data, sem_t *sem) {
    srand(time(NULL) + ta_id); // Seed random number generator

    int marked_count = 0;
    while (marked_count < NUM_STUDENTS * 3) { // Each process marks all students 3 times
        sem_wait(sem); // Lock semaphore for shared memory access

        // Access the shared student list
        int student_id = shared_data->student_list[shared_data->current_index];
        shared_data->current_index = (shared_data->current_index + 1) % NUM_STUDENTS;

        sem_post(sem); // Release semaphore

        printf("TA %d is marking student %d\n", ta_id + 1, student_id);
        random_delay(1, 4); // Simulate marking time

        int mark = rand() % (MAX_MARK + 1); // Generate random mark
        printf("TA %d assigned mark %d to student %d\n", ta_id + 1, mark, student_id);

        marked_count++;
        random_delay(1, 10); // Simulate processing time
        printf ("TA %d count is: %d\n", ta_id+1, marked_count);
    }

    printf("TA %d finished marking.\n", ta_id + 1);
}

int main() {
    srand(time(NULL));

    // Create shared memory
    int shm_id = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        exit(1);
    }

    // Attach shared memory
    SharedData *shared_data = (SharedData *)shmat(shm_id, NULL, 0);
    if (shared_data == (void *)-1) {
        perror("Failed to attach shared memory");
        exit(1);
    }

    // Initialize the student list
    for (int i = 0; i < NUM_STUDENTS; i++) {
        shared_data->student_list[i] = 1000 + i; // Assign student IDs starting from 1000
    }
    shared_data->current_index = 0;

    // Create semaphore
    sem_t *sem = sem_open("/shared_sem", O_CREAT, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("Failed to create semaphore");
        shmctl(shm_id, IPC_RMID, NULL); // Clean up shared memory
        exit(1);
    }

    // Create child processes
    pid_t pids[NUM_TA];
    for (int i = 0; i < NUM_TA; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Failed to fork process");
            shmctl(shm_id, IPC_RMID, NULL); // Clean up shared memory
            sem_unlink("/shared_sem");
            sem_close(sem);
            exit(1);
        }

        if (pid == 0) {
            // Child process
            marking_task(i, shared_data, sem);
            exit(0); // Exit child process
        } else {
            pids[i] = pid; // Save child PID
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_TA; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Clean up shared memory and semaphore
    shmctl(shm_id, IPC_RMID, NULL); // Remove shared memory
    sem_unlink("/shared_sem");      // Unlink semaphore
    sem_close(sem);                 // Close semaphore

    printf("All processes finished marking.\n");
    return 0;
}

