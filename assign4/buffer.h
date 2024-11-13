/**
* Assignment 4: Producer Consumer Problem
 * @file buffer.h
 * @author Noya Hafiz, Christian Lim
 * @brief header file for the buffer class
 * @version 0.1
 */
#ifndef BUFFER_H
#define BUFFER_H

#include <mutex>   // For std::mutex to handle thread synchronization
#include <condition_variable> // For std::condition_variable to handle waiting and signaling

// Define the data type of the buffer items
typedef int buffer_item;

/**
 * @brief The bounded buffer class. The number of items in the buffer cannot exceed the size of the buffer.
 */
class Buffer {
private:
    buffer_item* items;     // Array to store buffer items
    int max_size;          // Maximum size of the buffer
    int current_count;     // Number of items currently in the buffer
    int front_idx;         // Index for the front of the buffer
    int rear_idx;          // Index for the rear of the buffer
    std::mutex mutex;      // Mutex for thread synchronization
    std::condition_variable empty_cond;  // Condition variable for empty buffer
    std::condition_variable full_cond;   // Condition variable for full buffer

public:
   /**
     * @brief Construct a new Buffer object
     * @param size the size of the buffer
     */
    Buffer(int size = 5);   // Constructor with default size

    /**
     * @brief Destroy the Buffer object
     */
    ~Buffer();              // Destructor

    /**
     * @brief Copy constructor for the Buffer class
     * 
     * This constructor is used when creating a new buffer as a copy of an existing buffer.
     * It allocates new memory for the buffer and copies the contents from the other buffer.
     * 
     * @param other The buffer object to copy from
     */
    Buffer(const Buffer& other);  // Copy constructor

    /**
     * @brief Assignment operator for the Buffer class
     * 
     * This operator handles the assignment of one buffer to another. It ensures proper memory
     * management by deleting any existing buffer data and copying the contents from the other buffer.
     * 
     * @param other The buffer object to assign from
     * @return A reference to this buffer object
     */
    Buffer& operator=(const Buffer& other);  // Assignment operator

    /**
     * @brief Insert an item into the buffer
     * @param item the item to insert
     * @return true if successful
     * @return false if not successful
     */
    bool insert_item(buffer_item item);   // Adds item to the buffer

    /**
     * @brief Remove an item from the buffer
     * @param item the item to remove
     * @return true if successful
     * @return false if not successful
     */
    bool remove_item(buffer_item* item);  // Removes item from the buffer

    /**
     * @brief Get the size of the buffer
     * @return the size of the buffer
     */
    int get_size();     // Returns buffer size

    /**
     * @brief Get the number of items in the buffer
     * @return the number of items in the buffer
     */
    int get_count();      // Returns the current item count

    /**
     * @brief Chceck if the buffer is empty
     * @return true if the buffer is empty, else false
     */
    bool is_empty();    // Returns true if buffer is empty

    /**
     * @brief Check if the buffer is full
     * @return true if the buffer is full, else false
     */
    bool is_full();     // Returns true if buffer is full

    /**
     * @brief Print the buffer
     */
    void print_buffer();   // Prints buffer contents
};

#endif // BUFFER_H
