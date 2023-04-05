#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <tgmath.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    int id;
    int min_think;
    int max_think;
    int min_dine;
    int max_dine;
    char* dst;
    int num;
    pthread_mutex_t* chopsticks;
    pthread_cond_t* self;
} Philosopher;

int num_phsp;
double total_think_time;
double phsp_thinks[1];
Philosopher* createPhilosopher(int id, int min_think, int max_think, int min_dine, int max_dine, char* dst, int num, pthread_mutex_t* chopsticks, pthread_cond_t* self) {
    Philosopher* philosopher = (Philosopher*) malloc(sizeof(Philosopher));
    philosopher->id = id;
    philosopher->min_think = min_think;
    philosopher->max_think = max_think;
    philosopher->min_dine = min_dine;
    philosopher->max_dine = max_dine;
    philosopher->dst = dst;
    philosopher->num = num;
    philosopher->chopsticks = chopsticks;
    philosopher->self = self;
    return philosopher;
}

int generateRandomTime(int min, int max, char* dst) {
    int time;
    if (strcmp(dst, "uniform") == 0) {
        time = (rand() % (max - min + 1)) + min;
    } else if (strcmp(dst, "exponential") == 0) {
        double lambda = 1.0 / ((min + max) / 2);
        double rand_val;
        do {
            rand_val = ((double) rand() / (double) RAND_MAX);
        } while (rand_val == 0); // rand() can return 0
        time = -log(rand_val) / lambda;
    }
    return time;
}

void* philosopherThread(void* arg) {
    Philosopher* philosopher = (Philosopher*) arg;

    int id = philosopher->id;
    int min_think = philosopher->min_think;
    int max_think = philosopher->max_think;
    int min_dine = philosopher->min_dine;
    int max_dine = philosopher->max_dine;
    char* dst = philosopher->dst;
    int num = philosopher->num;
    pthread_mutex_t* chopsticks = philosopher->chopsticks;
    pthread_cond_t* self = philosopher->self;
    clock_t start,end;
    double thinking_time;
    for (int i = 0; i < num; i++) {
        // Generate thinking time
        int think_time = generateRandomTime(min_think, max_think, dst);
        // Think
        printf("Philosopher %d is thinking for %d ms\n", id, think_time);
        start = clock();
        usleep(think_time * 1000);
        end = clock();
        thinking_time += (((double)(end - start))/ CLOCKS_PER_SEC)*1000;
        // Get chopsticks
        pthread_mutex_lock(&chopsticks[id]);
        pthread_mutex_lock(&chopsticks[(id + 1) % num_phsp]);

        // Generate dining time
        int dine_time = generateRandomTime(min_dine, max_dine, dst);
        // Dine
        printf("Philosopher %d is dining for %d ms\n", id, dine_time);
        usleep(dine_time * 1000);

        // Release chopsticks
        pthread_mutex_unlock(&chopsticks[id]);
        pthread_mutex_unlock(&chopsticks[(id + 1) % num_phsp]);
    }
    printf("Philosopher %d finished dining\n", id);

    printf("Philosopher %d thinking_time is %0.f ms\n", id,thinking_time);
    phsp_thinks[id]= thinking_time;
    total_think_time+=thinking_time;

    pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    num_phsp = atoi(argv[1]);
    int min_think = atoi(argv[2]);
    int max_think = atoi(argv[3]);
    int min_dine = atoi(argv[4]);
    int max_dine = atoi(argv[5]);
    char* dst = argv[6];
    int num = atoi(argv[7]);
    phsp_thinks[num_phsp];

    // Initialize mutex and condition variables
    pthread_mutex_t chopsticks[num_phsp];
    for (int i = 0; i < num_phsp; i++) {
        pthread_mutex_init(&chopsticks[i], NULL);
    }
    pthread_cond_t self[num_phsp];
    for (int i = 0; i < num_phsp; i++) {
        pthread_cond_init(&self[i], NULL);
    }

    // Create philosopher threads
    pthread_t philosophers[num_phsp];
    for (int i = 0; i < num_phsp; i++) {
        Philosopher* philosopher = createPhilosopher(i, min_think, max_think, min_dine, max_dine, dst, num, chopsticks, self);
        pthread_create(&philosophers[i], NULL, philosopherThread, (void*)philosopher);
    }

    // Wait for all philosophers to finish dining
    for (int i = 0; i < num_phsp; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Clean up
    for (int i = 0; i < num_phsp; i++) {
        pthread_mutex_destroy(&chopsticks[i]);
        pthread_cond_destroy(&self[i]);
    }

    printf("Philosophers total_thinking_time is %0.f ms\n",total_think_time);
    double average_time= total_think_time/num_phsp;
    double variance=0;
    for (int i = 0; i < num_phsp; i++) {
        variance += pow(phsp_thinks[i]-average_time,2);
    }
    printf("Philosophers average_thinking_time is %0.f ms\n",average_time);
    double standardDeviation= sqrt(variance / num_phsp);
    printf("Philosophers standard deviation is %0.f ms\n", standardDeviation);
    return 0;
}
