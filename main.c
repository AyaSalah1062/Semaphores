#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 2
#define NUM_COUNTER_THREADS 2
//semaphore variables for synchronization
sem_t empty_slots, full_slots, buffer_mutex, counter_mutex;

// Counter problem
int message_counter = 0;

// Buffer and positions
int message_buffer[BUFFER_SIZE];
int write_index = 0;  // Producer index
int read_index = 0;   // Consumer index

// Adds a value to the buffer.
void enqueue(int value) {
    message_buffer[write_index] = value;
    write_index = (write_index + 1) % BUFFER_SIZE;
}

//Removes a value from the buffer and returns it.
int dequeue() {
    int value = message_buffer[read_index];
    read_index = (read_index + 1) % BUFFER_SIZE;
    return value;
}

//Simulates a counter thread that receives messages and increments the message counter.
void *mCounter(void *arg) {
    while (1) {
        sleep(rand() % 5);
        int thread_id = *((int *)arg);
        printf("Counter thread %d: received a message\n", thread_id);

        sem_wait(&counter_mutex);
        sem_wait(&buffer_mutex);
        printf("Counter thread %d: waiting to write\n", thread_id);
        message_counter++;
        printf("Counter thread %d: now adding to counter, counter value = %d\n", thread_id, message_counter);
        sem_post(&buffer_mutex);
        sem_post(&counter_mutex);
    }
}

//Simulates a monitor thread that reads the message counter, writes the count to the buffer, and handles buffer full conditions.
void *mMonitor(void *arg) {
    while (1) {
        sleep(rand() % 5);
        printf("Monitor thread: waiting to read counter\n");
        // Wait for access to the counter_mutex and buffer_mutex

        sem_wait(&counter_mutex);
        sem_wait(&buffer_mutex);
        int count = message_counter;
        message_counter = 0;
        // Release the buffer_mutex and counter_mutex

        sem_post(&buffer_mutex);
        sem_post(&counter_mutex);

        sem_wait(&empty_slots);

        // Write to the buffer at the next available position
        enqueue(count);
        printf("Monitor thread: reading a count value of %d\n", count);
        printf("Monitor thread: writing to buffer at position %d\n", (write_index - 1 + BUFFER_SIZE) % BUFFER_SIZE);
        sem_post(&full_slots);
        
        //Check if the buffer is full
        if (write_index == read_index) {
            printf("Monitor thread: buffer full!!\n");
        }
    }
}

// Simulates a collector thread that reads from the buffer and handles buffer empty conditions.
void *mCollector(void *arg) {
    while (1) {
        sleep(rand() % 5);

        sem_wait(&full_slots);

        // Read from the buffer at the next available position
        int count = dequeue();

        sem_post(&empty_slots);

        if (count == -1) {
            printf("Collector thread: nothing is in the buffer!\n");
        } else {
            printf("Collector thread: reading from buffer at position %d\n", (read_index - 1 + BUFFER_SIZE) % BUFFER_SIZE);
        }
    }
}

int main() {
    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    sem_init(&buffer_mutex, 0, 1);
    sem_init(&counter_mutex, 0, 1);

    pthread_t counter_threads[NUM_COUNTER_THREADS], monitor_thread, collector_thread;

    for (int i = 0; i < NUM_COUNTER_THREADS; i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i + 1;
        pthread_create(&counter_threads[i], NULL, mCounter, arg);
    }
    //ID,Default thread attributes,Function to be executed,Argument to be passed to the fun
    pthread_create(&monitor_thread, NULL, mMonitor, NULL);
    pthread_create(&collector_thread, NULL, mCollector, NULL);

    for (int i = 0; i < NUM_COUNTER_THREADS; i++) {
        //ID thread,Pointer to the loc where the exit status will be stored
        pthread_join(counter_threads[i], NULL);
    }

    pthread_join(monitor_thread, NULL);
    pthread_join(collector_thread, NULL);

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    sem_destroy(&buffer_mutex);
    sem_destroy(&counter_mutex);

    return 0;
}
