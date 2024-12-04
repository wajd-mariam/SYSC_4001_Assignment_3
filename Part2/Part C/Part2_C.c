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

typedef struct {
    int student_list[NUM_STUDENTS]; // List of student IDs
    int current_index;              // Shared index
    int total_marked;               // Global counter for total marked students
} SharedData;

void random_delay(int min, int max) {
    sleep(rand() % (max - min + 1) + min);
}

void marking_task(int ta_id, SharedData *shared_data, sem_t **semaphores) {
    srand(getpid());

    while (1) {
        // Attempt to acquire both semaphores
        int first_semaphore = ta_id;
        int second_semaphore = (ta_id + 1) % NUM_TA;

        if (sem_wait(semaphores[first_semaphore]) == 0) {
            if (sem_wait(semaphores[second_semaphore]) == 0) {
                // Both semaphores acquired, proceed with marking
                if (shared_data->total_marked >= NUM_STUDENTS * 3) {
                    // Release semaphores and break if all students are marked
                    sem_post(semaphores[first_semaphore]);
                    sem_post(semaphores[second_semaphore]);
                    break;
                }

                // Access shared memory
                int student_id = shared_data->student_list[shared_data->current_index];
                shared_data->current_index = (shared_data->current_index + 1) % NUM_STUDENTS;
                shared_data->total_marked++;

                printf("TA %d is marking student %d\n", ta_id + 1, student_id);
                random_delay(1, 4);

                int mark = rand() % (MAX_MARK + 1);
                printf("TA %d assigned mark %d to student %d\n", ta_id + 1, mark, student_id);

                random_delay(1, 10);

                // Release both semaphores
                sem_post(semaphores[second_semaphore]);
                sem_post(semaphores[first_semaphore]);
            } else {
                // Failed to acquire the second semaphore, release the first
                sem_post(semaphores[first_semaphore]);
            }
        }
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
    shared_data->total_marked = 0;

    // Create semaphores for each TA
    sem_t *semaphores[NUM_TA];
    for (int i = 0; i < NUM_TA; i++) {
        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/sem_ta_%d", i);
        semaphores[i] = sem_open(sem_name, O_CREAT, 0644, 1);
        if (semaphores[i] == SEM_FAILED) {
            perror("Failed to create semaphore");
            shmctl(shm_id, IPC_RMID, NULL);
            for (int j = 0; j < i; j++) {
                sem_unlink(sem_name);
            }
            exit(1);
        }
    }

    // Create child processes
    pid_t pids[NUM_TA];
    for (int i = 0; i < NUM_TA; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Failed to fork process");
            shmctl(shm_id, IPC_RMID, NULL);
            for (int j = 0; j < NUM_TA; j++) {
                sem_close(semaphores[j]);
                char sem_name[20];
                snprintf(sem_name, sizeof(sem_name), "/sem_ta_%d", j);
                sem_unlink(sem_name);
            }
            exit(1);
        }

        if (pid == 0) {
            // Child process
            marking_task(i, shared_data, semaphores);
            exit(0);
        } else {
            pids[i] = pid; // Save child PID
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < NUM_TA; i++) {
        waitpid(pids[i], NULL, 0);
    }

    // Clean up shared memory and semaphores
    shmctl(shm_id, IPC_RMID, NULL); // Remove shared memory
    for (int i = 0; i < NUM_TA; i++) {
        sem_close(semaphores[i]);
        char sem_name[20];
        snprintf(sem_name, sizeof(sem_name), "/sem_ta_%d", i);
        sem_unlink(sem_name);
    }

    printf("All processes finished marking.\n");
    return 0;
}

