#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define NUM_TAS 5
#define NUM_STUDENTS 20
#define MAX_MARK 10

// Shared data structures
int student_list[NUM_STUDENTS]; // List of students
int current_index = 0;          // Shared index in the student list
sem_t semaphores[NUM_TAS];      // Semaphores for each TA
pthread_mutex_t index_lock;     // Mutex to protect the shared index

// Function to simulate marking delay
void random_delay(int min, int max) {
    sleep(rand() % (max - min + 1) + min);
}

// TA task function
void *marking_task(void *arg) {
    int ta_id = *((int *)arg);
    char file_name[10];
    snprintf(file_name, sizeof(file_name), "TA%d.txt", ta_id + 1);
    FILE *file = fopen(file_name, "w");
    if (!file) {
        perror("Error opening file");
        pthread_exit(NULL);
    }

    int marked_count = 0;

    while (marked_count < NUM_STUDENTS * 3) { // Each TA marks all students 3 times
        // Lock semaphores
        sem_wait(&semaphores[ta_id]);
        sem_wait(&semaphores[(ta_id + 1) % NUM_TAS]);

        // Access the database
        pthread_mutex_lock(&index_lock);
        int student_id = student_list[current_index];
        current_index = (current_index + 1) % NUM_STUDENTS;
        pthread_mutex_unlock(&index_lock);

        printf("TA %d is accessing the database and picked student %d\n", ta_id + 1, student_id);
        random_delay(1, 4); // Simulate database access time

        // Release semaphores
        sem_post(&semaphores[ta_id]);
        sem_post(&semaphores[(ta_id + 1) % NUM_TAS]);

        // Marking the student
        int mark = rand() % (MAX_MARK + 1);
        fprintf(file, "Student %04d: %d\n", student_id, mark);
        fflush(file);

        printf("TA %d is marking student %d with mark %d\n", ta_id + 1, student_id, mark);
        random_delay(1, 10); // Simulate marking time
	
        marked_count++;
        printf ("TA %d count is: %d\n", ta_id+1, marked_count);
    }

    fclose(file);
    printf("TA %d finished marking.\n", ta_id + 1);
    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    // Initialize the student list
    for (int i = 0; i < NUM_STUDENTS - 1; i++) {
        student_list[i] = 1000 + i; // Assign student IDs from 1000
    }
    student_list[NUM_STUDENTS - 1] = 9999; // End-of-file marker

    // Initialize semaphores and mutex
    for (int i = 0; i < NUM_TAS; i++) {
        sem_init(&semaphores[i], 0, 1);
    }
    pthread_mutex_init(&index_lock, NULL);

    // Create TA threads
    pthread_t ta_threads[NUM_TAS];
    int ta_ids[NUM_TAS];
    for (int i = 0; i < NUM_TAS; i++) {
        ta_ids[i] = i;
        pthread_create(&ta_threads[i], NULL, marking_task, &ta_ids[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < NUM_TAS; i++) {
        pthread_join(ta_threads[i], NULL);
    }

    // Destroy semaphores and mutex
    for (int i = 0; i < NUM_TAS; i++) {
        sem_destroy(&semaphores[i]);
    }
    pthread_mutex_destroy(&index_lock);

    printf("All TAs have finished marking.\n");
    return 0;
}

