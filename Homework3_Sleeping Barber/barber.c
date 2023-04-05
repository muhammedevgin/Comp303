#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

//Waiting time
long long int total_waiting_time = 0;
// Number of customers
int num_customers;
// Maximum arrival time for customers
int max_arrival_time;
// Maximum haircut duration
int max_haircut_duration;
// Number of times each customer gets a haircut
int haircut_repetition;

// Mutex for accessing the number of customers waiting
pthread_mutex_t num_waiting_mutex = PTHREAD_MUTEX_INITIALIZER;
// Condition variable for customers waiting for a haircut
pthread_cond_t customer_waiting = PTHREAD_COND_INITIALIZER;
// Condition variable for the barber waiting for customers
pthread_cond_t barber_waiting = PTHREAD_COND_INITIALIZER;

pthread_mutex_t waiting_time_mutex = PTHREAD_MUTEX_INITIALIZER;

// Number of customers currently waiting for a haircut
int num_waiting = 0;
// Flag to indicate if the barber is currently cutting hair
int barber_cutting = 0;
// Number of seats
int num_seat=0;

// Function for the customer thread
void *customer(void *arg) {
    int id = (int)arg;
    int haircuts = 0;

    while (haircuts < haircut_repetition) {
        // Wait for a random amount of time before arriving
        usleep(rand() % max_arrival_time);

        // Acquire the mutex for accessing num_waiting
        pthread_mutex_lock(&num_waiting_mutex);

        // Check if there is a free seat in the waiting room
        if (num_waiting == num_seat) {
            printf("Customer %d: Waiting room is full, leaving.\n", id);
            pthread_mutex_unlock(&num_waiting_mutex);
            continue;
        }
        struct timeval start_time;
        gettimeofday(&start_time, NULL);
        // Increment the number of customers waiting
        num_waiting++;
        printf("Customer %d: Waiting for a haircut. %d customers waiting.\n", id, num_waiting);

        // Signal the barber that a customer is waiting
        pthread_cond_signal(&customer_waiting);

        // Wait for the barber to signal that it's the customer's turn
        while (barber_cutting == 0) {
            pthread_cond_wait(&barber_waiting, &num_waiting_mutex);
        }
        barber_cutting = 0;

        // Decrement the number of customers waiting
        num_waiting--;

        // Release the mutex for accessing num_waiting
        pthread_mutex_unlock(&num_waiting_mutex);

        // Wait for the haircut to finish
        usleep(rand() % max_haircut_duration);
        printf("Customer %d: Haircut finished.\n", id);
        haircuts++;
        struct timeval end_time;
        gettimeofday(&end_time, NULL);
        long long int waiting_time = (end_time.tv_sec - start_time.tv_sec) * 1000000 + (end_time.tv_usec - start_time.tv_usec);
        pthread_mutex_lock(&waiting_time_mutex);
        total_waiting_time += waiting_time;
        pthread_mutex_unlock(&waiting_time_mutex);
    }
    printf("Customer %d: Leaving the shop.\n", id);
    printf("Total waiting time for customer whose id %d: %lld\n", id , total_waiting_time);
    total_waiting_time=0;
    pthread_exit(NULL);
}

// Function for the barber thread
void *barber(void *arg) {
    while (1) {
// Acquire the mutex for accessing num_waiting
        pthread_mutex_lock(&num_waiting_mutex);

// Wait for a customer to arrive
        while (num_waiting == 0) {
            pthread_cond_wait(&customer_waiting, &num_waiting_mutex);
        }

// Signal the customer that it's their turn
        barber_cutting = 1;
        pthread_cond_signal(&barber_waiting);

// Release the mutex for accessing num_waiting
        pthread_mutex_unlock(&num_waiting_mutex);

// Cut the customer's hair
        usleep(rand() % max_haircut_duration);
        printf("Barber: Haircut finished.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: barber <num_customer> <max_arrival_time> <max_haircut_duration> <haircut_repetition>\n");
        exit(1);
    }

    num_customers = atoi(argv[1]);
    max_arrival_time = atoi(argv[2]);
    max_haircut_duration = atoi(argv[3]);
    haircut_repetition = atoi(argv[4]);
    num_seat = atoi(argv[5]);

    // Create the barber thread
    pthread_t barber_thread;
    pthread_create(&barber_thread, NULL, barber, NULL);

    // Create the customer threads
    pthread_t customer_threads[num_customers];
    for (int i = 0; i < num_customers; i++) {
        pthread_create(&customer_threads[i], NULL, customer, (void *)i);
    }

    // Wait for the customer threads to finish
    for (int i = 0; i < num_customers; i++) {
        pthread_join(customer_threads[i], NULL);
    }
    return 0;
}
