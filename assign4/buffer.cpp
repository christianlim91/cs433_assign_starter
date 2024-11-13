
/**
* Assignment 4: Producer Consumer Problem
 * @file buffer.cpp
 * @author Noya Hafiz, Chrisitan Lim
 * @brief Implementation file for the buffer class
 * @version 0.1
 */
#include "buffer.h"
#include <iostream>
#include <mutex>  // For using std::mutex and std::unique_lock for thread synchronization

using namespace std;

/**
 * @brief Constructor for the Buffer class
 * 
 * This constructor initializes the buffer with a specified size. It allocates memory for
 * the buffer array, sets the initial item count to 0, and initializes the front and rear indices.
 * The buffer is initially empty.
 *
 * @param size The size of the buffer (default is 5)
 */
Buffer::Buffer(int size) {
    max_size = size;            // Set the maximum size of the buffer
    current_count = 0;          // Initially, there are no items in the buffer
    front_idx = 0;              // The front index starts at 0 (initial position to remove items from)
    rear_idx = -1;              // The rear index starts at -1 (indicating the buffer is empty)
    items = new buffer_item[max_size];  // Dynamically allocate memory for the buffer array
}

/**
 * @brief Destructor for the Buffer class
 * 
 * The destructor cleans up the dynamically allocated memory for the buffer array
 * to prevent memory leaks when the buffer object is destroyed.
 */
Buffer::~Buffer() {
    delete[] items;  // Free the dynamically allocated buffer array
}

/**
 * @brief Copy constructor for the Buffer class
 * 
 * This constructor is used to create a new buffer as a copy of an existing buffer.
 * It allocates new memory and copies the contents of the other buffer, ensuring both
 * buffers are independent.
 * 
 * @param other The buffer object to copy from
 */
Buffer::Buffer(const Buffer& other) {
    max_size = other.max_size;
    current_count = other.current_count;
    front_idx = other.front_idx;
    rear_idx = other.rear_idx;
    
    items = new buffer_item[max_size];  // Allocate new memory for the buffer array
    for (int i = 0, index = front_idx; i < current_count; i++) {
        items[i] = other.items[index];  // Copy each item from the other buffer
        index = (index + 1) % max_size;  // Handle circular buffer logic to wrap around
    }
}

/**
 * @brief Assignment operator for the Buffer class
 * 
 * This operator handles the assignment of one buffer to another. It first cleans up
 * any existing buffer data and then copies the contents of the other buffer into the 
 * current buffer. It also checks for self-assignment to prevent unnecessary work.
 *
 * @param other The buffer object to assign from
 * @return A reference to the current buffer object
 */
Buffer& Buffer::operator=(const Buffer& other) {
    // Check for self-assignment
    if (this == &other) {
        return *this;
    }

    // Clean up old buffer data
    delete[] items;

    // Copy the values from the other buffer
    max_size = other.max_size;
    current_count = other.current_count;
    front_idx = other.front_idx;
    rear_idx = other.rear_idx;
    
    items = new buffer_item[max_size];  // Allocate new memory for the buffer array
    for (int i = 0, index = front_idx; i < current_count; i++) {
        items[i] = other.items[index];  // Copy each item from the other buffer
        index = (index + 1) % max_size;  // Handle circular buffer logic to wrap around
    }

    return *this;
}

/**
 * @brief Inserts an item into the buffer
 * 
 * This function inserts an item into the buffer if there is space available.
 * It locks the mutex to ensure thread safety, waits if the buffer is full, and signals
 * the consumer thread when an item is successfully added.
 * 
 * @param item The item to insert into the buffer
 * @return true if the item was successfully inserted
 * @return false if the buffer is full and the item could not be inserted
 */
bool Buffer::insert_item(buffer_item item) {
    std::unique_lock<std::mutex> lock(mutex);  // Lock the mutex to ensure thread safety

    // Wait until the buffer is not full (use condition variable to wait)
    while (is_full()) {
        empty_cond.wait(lock);  // Wait for space to become available
    }

    // Insert the item at the rear index and update the rear index
    rear_idx = (rear_idx + 1) % max_size;  // Circular buffer logic: wrap around when full
    items[rear_idx] = item;  // Store the item in the buffer
    current_count++;  // Increment the current item count

    full_cond.notify_one();  // Signal the consumer that an item was added
    return true;
}

/**
 * @brief Removes an item from the buffer
 * 
 * This function removes an item from the buffer if it is not empty. It locks the mutex 
 * to ensure thread safety, waits if the buffer is empty, and signals the producer thread 
 * when space is available after removing an item.
 * 
 * @param item Pointer to a variable to store the removed item
 * @return true if an item was successfully removed
 * @return false if the buffer is empty and no item could be removed
 */
bool Buffer::remove_item(buffer_item* item) {
    std::unique_lock<std::mutex> lock(mutex);  // Lock the mutex to ensure thread safety

    // Wait until the buffer is not empty (use condition variable to wait)
    while (is_empty()) {
        full_cond.wait(lock);  // Wait for an item to become available
    }

    *item = items[front_idx];  // Remove the item from the front of the buffer
    front_idx = (front_idx + 1) % max_size;  // Circular buffer logic: wrap around when empty
    current_count--;  // Decrease the current item count

    empty_cond.notify_one();  // Signal the producer that space was freed
    return true;
}

/**
 * @brief Gets the size of the buffer
 * 
 * This function returns the maximum size (capacity) of the buffer.
 * 
 * @return The maximum size of the buffer
 */
int Buffer::get_size() {
    return max_size;  // Return the maximum size of the buffer
}

/**
 * @brief Gets the current item count in the buffer
 * 
 * This function returns the current number of items in the buffer.
 * 
 * @return The current number of items in the buffer
 */
int Buffer::get_count() {
    return current_count;  // Return the current number of items in the buffer
}

/**
 * @brief Checks if the buffer is empty
 * 
 * This function checks if the buffer is empty by comparing the item count to 0.
 * 
 * @return true if the buffer is empty, false otherwise
 */
bool Buffer::is_empty() {
    return current_count == 0;  // Return true if the buffer is empty
}

/**
 * @brief Checks if the buffer is full
 * 
 * This function checks if the buffer is full by comparing the item count to
 * the maximum size of the buffer.
 * 
 * @return true if the buffer is full, false otherwise
 */
bool Buffer::is_full() {
    return current_count == max_size;  // Return true if the buffer is full
}

/**
 * @brief Prints the contents of the buffer
 * 
 * This function prints the current items in the buffer. If the buffer is empty,
 * it prints a message indicating that the buffer is empty. It handles the circular
 * nature of the buffer when printing the items.
 */
void Buffer::print_buffer() {
    cout << "Buffer: [";
    if (is_empty()) {
        cout << "-- Buffer is empty --";  // If the buffer is empty, print this message
    } else {
        // Print the buffer items, handling the circular buffer logic
        for (int i = 0, index = front_idx; i < current_count; i++) {
            cout << items[index];  // Print each item in the buffer
            if (i < current_count - 1) cout << ", ";  // Add commas between items
            index = (index + 1) % max_size;  // Circular buffer logic: wrap around when empty
        }
    }
    cout << "]\n";  // End the buffer printout
}