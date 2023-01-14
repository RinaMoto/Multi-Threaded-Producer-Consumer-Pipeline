#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

// Size of the buffers
#define SIZE 1000


// Number of items that will be produced. This number is less than the size of the buffer. Hence, we can model the buffer as being unbounded.
#define NUM_ITEMS 50 


// Buffer 1, shared resource between input thread and line separator thread
char buffer_1[NUM_ITEMS][SIZE];
// Number of items in the buffer
int count_1 = 0;
// Index where the input thread will put the next item
int prod_idx_1 = 0;
// Index where the line-separator thread will pick up the next item
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

// Buffer 2, shared resource between line separator thread and plus sign thread
char buffer_2[NUM_ITEMS][SIZE];
// Number of items in the buffer
int count_2 = 0;
// Index where the line-separator thread will put the next item
int prod_idx_2 = 0;
// Index where the plus sign thread will pick up the next item
int con_idx_2 = 0;
// Initialize the mutex for buffer 2
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 2
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;


// Buffer 3, shared resource between plus sign thread and output thread
char buffer_3[NUM_ITEMS][SIZE];
// Number of items in the buffer
int count_3 = 0;
// Index where the plus sign thread will put the next item
int prod_idx_3 = 0;
// Index where the output thread will pick up the next item
int con_idx_3 = 0;
// Initialize the mutex for buffer 2=3
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 3
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

// Program is adapted from the example program provided for this assignment

/* 
 * Get the user input 
*/
char get_user_input(){
	char input[SIZE];
	printf("Enter a positive integer: ");
	fgets(input, SIZE, stdin);
        return *input;
}

/*
 *  Put an item in buff_1
*/
void put_buff_1(const char *item){
	// Lock the mutex before putting the item in the buffer
	pthread_mutex_lock(&mutex_1);
	// Put the item in the buffer
        strcpy(buffer_1[prod_idx_1], item);
	// Increment the index where the next item will be put.
        prod_idx_1 = prod_idx_1 + 1;
	count_1++;
	// Signal to the consumer that the buffer is no longer empty
        pthread_cond_signal(&full_1);
        // Unlock the mutex
	pthread_mutex_unlock(&mutex_1);
}

/*
 *  Function that the input thread will run.
 *  Get input from the user.
 *  Put the user input string in the buffer shared with the line separator thread.
*/
void *get_input(void *args)
{
	char thread_buffer1[SIZE];
	for (int i = 0; i < NUM_ITEMS; i++){
		// Get the user input
		memset(thread_buffer1, 0, SIZE);
		fgets(thread_buffer1, SIZE, stdin);
		fflush(stdin);
		put_buff_1(thread_buffer1);
		if (strncmp(thread_buffer1, "STOP\n", 5) == 0) {
			break;
		}
	}
	return NULL;
}

/* 
 *  Function that the line separator thread will run
 *  Replace newline character with a space
 *  Put the new string in the buffer shared with the plus sign thread
 */
void *separate_line_thread(void *args){
	char thread_buffer2[SIZE];
	for (int i = 0; i < NUM_ITEMS; i++){
				
		memset(thread_buffer2, '\0', SIZE);
		// Lock the mutex before checking if the buffer has data
		pthread_mutex_lock(&mutex_1);
		while (count_1 == 0){
			// Buffer is empty. Wait for the producer to signal that the buffer has data
			pthread_cond_wait(&full_1, &mutex_1);
		}
		strcpy(thread_buffer2, buffer_1[con_idx_1]);
		// Increment the index from which the item will be picked up
		con_idx_1 = con_idx_1 + 1;
		count_1--;
		// Unlock the mutex
		pthread_mutex_unlock(&mutex_1);
		// replace \n with space
		char space = ' ';

		if (strncmp(thread_buffer2, "STOP\n", 5) != 0) {
			thread_buffer2[strlen(thread_buffer2)-1] = space;
		}
		
		// lock the mutex before putting data into buffer
		pthread_mutex_lock(&mutex_2);
		// put into buffer_2
		strcpy(buffer_2[prod_idx_2], thread_buffer2);
		// increment the index from which the item will be picked up
		prod_idx_2++;
		count_2++;
		// signal that the buffer is no longer empty
		pthread_cond_signal(&full_2);
		// unlock mutex
		pthread_mutex_unlock(&mutex_2);

		// check for stop
		if (strncmp(thread_buffer2, "STOP\n", 5) == 0) {
			break;
		}
	}
	return NULL;
}

/*  
 *  Function that the plus sign thread will run
 *  Replace all instances of ++ with ^ in the string
 *  Put the new string in the buffer shared with the output thread
 */
void *plus_sign_thread(void *args) {
	char thread_buffer3[SIZE]; 
	for (int i = 0; i < NUM_ITEMS; i++) {
		memset(thread_buffer3, '\0', SIZE);
		// Lock the mutex before checking if the buffer has data
		pthread_mutex_lock(&mutex_2);
		while (count_2 == 0) {
			// buffer is empty. Wait for the producer to signal that the buffer has data
			pthread_cond_wait(&full_2, &mutex_2);
		}

		strcpy(thread_buffer3, buffer_2[con_idx_2]);
		// Increment the index from which the item will be picked up
		con_idx_2 = con_idx_2 + 1;
		count_2--;
		// Unlock the mutex
		pthread_mutex_unlock(&mutex_2);

		char new_string[SIZE];
		char *ptr = thread_buffer3;	
		int k = 0;
		memset(new_string, '\0', SIZE);

		while (*ptr) {
			if (strstr(ptr, "++") == ptr) {
				strcpy(&new_string[k], "^");
				k += strlen("^");
				ptr += strlen("++");
			}
			else {
				new_string[k++] = *ptr++;
			}

		}
		
		int new_string_len = strlen(new_string);
		
		// Copy the new_string into thread_buffer3
		strncpy(thread_buffer3, new_string, new_string_len); 
		// null terminate the end of the string
		thread_buffer3[new_string_len] = '\0';

		// Lock the mutex before putting data into the buffer
		pthread_mutex_lock(&mutex_3);
		// Put into buffer_3
		strcpy(buffer_3[prod_idx_3], thread_buffer3);
		// Increment the index from which the item will be picked up
		prod_idx_3++;
		count_3++;
		// Signal that the buffer is no longer empty  
		pthread_cond_signal(&full_3);
		// Unlock the mutex
		pthread_mutex_unlock(&mutex_3);

		// Check for stop
		if (strncmp(thread_buffer3, "STOP\n", 5) == 0) {
			break;
		}
	}
	return NULL;
}

/*
 *  Function that the output thread will run
 *  Output 80 character lines to stdout
 */
void *write_output(void *args) {
	char thread_buffer4[SIZE];
	char output[81];
	int char_count = 0;
	memset(output, '\0', 81);
	while (1) {
		memset(thread_buffer4, '\0', SIZE);
		// Lock the mutex before checking if the buffer has data
		pthread_mutex_lock(&mutex_3);
		while (count_3 == 0) {
		// Buffer is empty. Wait for the producer to signal that the buffer has data
			pthread_cond_wait(&full_3, &mutex_3);
		}

		strcpy(thread_buffer4, buffer_3[con_idx_3]);
		// Increment the index from which the item will be picked up 
		con_idx_3++;
		count_3--;
		// Unlock the mutex
		pthread_mutex_unlock(&mutex_3);
		
		// Check for stop
		if (strncmp(thread_buffer4, "STOP\n", 5) == 0) {
			break;
		}

		// print out 80 characters at a time to stdout
		for (int i = 0; i < strlen(thread_buffer4); i++) {
			if (char_count == 80) {
				output[80] = '\0';
				fprintf(stdout, "%s\n", output);
				fflush(stdout);

				memset(output, '\0', 80);
				char_count = 0;
			}
			output[char_count] = thread_buffer4[i];
			char_count++;
		}
	}
	return NULL;
}

int main()
{
	srand(time(0));
	pthread_t input_t, line_separator_t, plus_sign_t, output_t;

	// Create the threads	
	pthread_create(&input_t, NULL, get_input, NULL);
	pthread_create(&line_separator_t, NULL, separate_line_thread, NULL);
	pthread_create(&plus_sign_t, NULL, plus_sign_thread, NULL);
	pthread_create(&output_t, NULL, write_output, NULL);
	// Wait for the threads to terminate
	pthread_join(input_t, NULL);
	pthread_join(line_separator_t, NULL);
	pthread_join(plus_sign_t, NULL);
	pthread_join(output_t, NULL);
	
	return EXIT_SUCCESS;
}
