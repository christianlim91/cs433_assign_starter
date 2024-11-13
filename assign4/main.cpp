
/**
 * Assignment 4: Producer Consumer Problem
 * @file main.cpp
 * @author Noya Hafiz, Chrisitan Lim
 * @brief The main program for the producer consumer problem.
 * @version 0.1
 */
#include <iostream>
#include "buffer.h"
#include <unistd.h>          // For usleep() function, which allows threads to sleep for a specified period of time (in microseconds).
#include <pthread.h>         // For using POSIX threads (pthreads). This header provides functions to create and manage threads, and for thread synchronization using mutexes and condition variables.
#include <semaphore.h>       // For using semaphores to manage synchronization. This header provides functions for semaphore initialization, wait, and signal operations, allowing threads to coordinate their execution.


using namespace std;

// Global buffer object with size 5
Buffer buffer(5);  // Initialize buffer with size 5
pthread_mutex_t mutex_lock;   // Mutex lock to synchronize access to the buffer
sem_t empty_slots;            // Semaphore to manage empty slots in the buffer
sem_t full_slots;             // Semaphore to manage full slots in the buffer

/**
 * @brief Producer thread function
 * 
 * This function is executed by each producer thread. Each producer will insert its unique
 * ID (passed as a parameter) into the buffer. The producer will wait if the buffer is full
 * and insert the item when there is space available. After inserting an item, the producer 
 * signals the consumer that there is an item to consume.
 * 
 * @param param Pointer to the producer's unique ID
 */
void* producer(void* param) {
    int producer_id = *((int*)param);  // Get producer ID from arguments
    buffer_item item = producer_id;    // Producer will insert its unique ID as an item

    // Infinite loop to continuously produce items
    while (true) {
        /* Sleep for a random period of time to simulate production delay */
        usleep(rand() % 1000000);  

        // Wait for an empty slot in the buffer (if the buffer is full, wait)
        sem_wait(&empty_slots);  

        // Enter critical section (access to the buffer is protected by mutex)
        pthread_mutex_lock(&mutex_lock);  

        // Try to insert the item into the buffer
        if (buffer.insert_item(item)) {  
            cout << "Producer " << producer_id << ": Inserted item " << item << endl;
            buffer.print_buffer();  // Print the current state of the buffer
        }

        // Exit critical section after modifying the buffer
        pthread_mutex_unlock(&mutex_lock);  

        // Signal that the consumer can now consume the item (buffer is no longer empty)
        sem_post(&full_slots);  
    }
}

/**
 * @brief Consumer thread function
 * 
 * This function is executed by each consumer thread. The consumer will remove an item from
 * the buffer. The consumer will wait if the buffer is empty and consume an item when there
 * is one available. After removing an item, the consumer signals the producer that there is
 * space available in the buffer for a new item.
 * 
 * @param param Unused, no arguments are passed to the consumer thread
 */
void* consumer(void* param) {
    buffer_item item;  // Variable to store the item removed from the buffer

    // Infinite loop to continuously consume items
    while (true) {
        /* Sleep for a random period of time to simulate consumption delay */
        usleep(rand() % 1000000);  

        // Wait for a full slot in the buffer (if the buffer is empty, wait)
        sem_wait(&full_slots);  

        // Enter critical section (access to the buffer is protected by mutex)
        pthread_mutex_lock(&mutex_lock);  

        // Try to remove an item from the buffer
        if (buffer.remove_item(&item)) {  
            cout << "Consumer: Removed item " << item << endl;
            buffer.print_buffer();  // Print the current state of the buffer
        }

        // Exit critical section after modifying the buffer
        pthread_mutex_unlock(&mutex_lock);  

        // Signal that space is now available in the buffer for the producer
        sem_post(&empty_slots);  
    }
}

int main(int argc, char* argv[]) {
    // Ensure correct number of arguments
    if (argc != 4) {
        cout << "Usage: ./producer_consumer <sleep_time> <num_producers> <num_consumers>" << endl;
        return 1;
    }

    // Parse command-line arguments
    int sleep_time = atoi(argv[1]);      // Time for which the main thread will sleep
    int num_producers = atoi(argv[2]);   // Number of producer threads
    int num_consumers = atoi(argv[3]);   // Number of consumer threads

    // Initialize semaphores and mutex
    sem_init(&empty_slots, 0, buffer.get_size());  // Set initial empty slots to buffer size
    sem_init(&full_slots, 0, 0);                  // Set initial full slots to 0
    pthread_mutex_init(&mutex_lock, NULL);        // Initialize the mutex lock for synchronization

    // Arrays to store thread IDs for producers and consumers
    pthread_t producers[num_producers], consumers[num_consumers];

    // Create producer threads
    for (int i = 0; i < num_producers; i++) {
        int* producer_id = new int(i + 1);  // Assign a unique ID to each producer
        pthread_create(&producers[i], NULL, producer, (void*)producer_id);  // Create producer thread
    }

    // Create consumer threads
    for (int j = 0; j < num_consumers; j++) {
        pthread_create(&consumers[j], NULL, consumer, NULL);  // Create consumer thread
    }

    // Main thread sleeps for the specified time to allow producers and consumers to run
    sleep(sleep_time);  

    cout << "Main thread: Time's up! Shutting down..." << endl;

    // Clean up and destroy semaphores and mutex before exiting
    sem_destroy(&empty_slots);  // Destroy empty slots semaphore
    sem_destroy(&full_slots);   // Destroy full slots semaphore
    pthread_mutex_destroy(&mutex_lock);  // Destroy the mutex lock

    return 0;  // Exit the program
}