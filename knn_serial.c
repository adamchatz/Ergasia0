#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

#define N 100          // αριθμός σημείων
#define D 3            // διαστάσεις
#define K 5            // αριθμός κοντινότερων γειτόνων
#define NUM_THREADS 4  // αριθμός νημάτων

typedef struct {
    int index;
    float distance;
} Neighbor;

typedef struct {
    int thread_id;
    int start;
    int end;
    float (*data)[D];
    Neighbor (*knn)[K];
} ThreadArgs;

float euclidean_distance(float *a, float *b, int d) {
    float sum = 0.0;
    int i;
    for (i = 0; i < d; i++) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

int compare_neighbors(const void *a, const void *b) {
    Neighbor *n1 = (Neighbor *)a;
    Neighbor *n2 = (Neighbor *)b;
    return (n1->distance > n2->distance) - (n1->distance < n2->distance);
}

void *thread_knn(void *arg) {
    ThreadArgs *args = (ThreadArgs *)arg;
    int i, j, k;

    printf("Thread %d will process points %d to %d\n", args->thread_id, args->start, args->end - 1);

    for (i = args->start; i < args->end; i++) {
        Neighbor neighbors[N];
        int count = 0;

        for (j = 0; j < N; j++) {
            if (i == j) continue;
            neighbors[count].index = j;
            neighbors[count].distance = euclidean_distance(args->data[i], args->data[j], D);
            count++;
        }

        qsort(neighbors, N - 1, sizeof(Neighbor), compare_neighbors);
        for (k = 0; k < K; k++) {
            args->knn[i][k] = neighbors[k];
        }
    }

    pthread_exit(NULL);
}

int main() {
    float data[N][D];
    Neighbor knn[N][K];
    int i, j, k;

    srand((unsigned int)time(NULL));
    for (i = 0; i < N; i++)
        for (j = 0; j < D; j++)
            data[i][j] = (float)rand() / RAND_MAX;

    pthread_t threads[NUM_THREADS];
    ThreadArgs args[NUM_THREADS];
    int points_per_thread = N / NUM_THREADS;

    for (i = 0; i < NUM_THREADS; i++) {
        args[i].thread_id = i;
        args[i].start = i * points_per_thread;
        args[i].end = (i == NUM_THREADS - 1) ? N : (i + 1) * points_per_thread;
        args[i].data = data;
        args[i].knn = knn;

        printf("Thread %d will process points %d to %d\n", args[i].thread_id, args[i].start, args[i].end - 1);
        pthread_create(&threads[i], NULL, thread_knn, (void *)&args[i]);
    }

    for (i = 0; i < NUM_THREADS; i++)
        pthread_join(threads[i], NULL);

    printf("\n--- kNN Results (first 5 points) ---\n");
    for (i = 0; i < 5; i++) {
        printf("Point %d: ", i);
        for (k = 0; k < K; k++) {
            printf("(%d, %.3f) ", knn[i][k].index, knn[i][k].distance);
        }
        printf("\n");
    }

    return 0;
}
