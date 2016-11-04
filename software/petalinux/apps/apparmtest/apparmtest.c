#define _GNU_SOURCE
#define _XOPEN_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <errno.h>
#include <sys/time.h>

#define NUM_THREADS     2
#define FIR_TAP         5
#define PR_ITERATION    20
typedef int     bench_type;
typedef float   pr_bench_type;

#ifndef MAC
pthread_barrier_t barrier_start;
pthread_barrier_t barrier_end;
#endif

typedef struct {
    pthread_t tid;
    int thread_num;
    int total_thread_num;
    void *a;
    void *b;
    void *c;
    int size_0;
} thread_info_t;

void *do_fir(void *param) {
    thread_info_t *thread_info = param;
#ifndef MAC
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_info->thread_num, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
#endif
    int offset = 0;
    int size = thread_info->size_0;
    int total_thread_num = thread_info->total_thread_num;
    bench_type filter[FIR_TAP] = {3,2,1,2,3};
    offset = thread_info->thread_num * (size / total_thread_num);
    bench_type *vector_a = (bench_type *)thread_info->a;
    bench_type *vector_b = (bench_type *)thread_info->b;
    int i, j;
    for (i = offset; i < offset + size / total_thread_num; i++) {
        bench_type sum = 0;
        for (j = 0; j < FIR_TAP; j++) {
            sum += vector_a[i + j] * filter[j];
        }
        vector_b[i] = sum;
    }
    pthread_exit(0);
}

void *do_vector(void *param) {
    thread_info_t *thread_info = param;
#ifndef MAC
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_info->thread_num, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
#endif
    int offset = 0;
    int size = thread_info->size_0;
    int total_thread_num = thread_info->total_thread_num;
    offset = thread_info->thread_num * (size / total_thread_num);
    bench_type *vector_a = (bench_type *)thread_info->a;
    bench_type *vector_b = (bench_type *)thread_info->b;
    bench_type *vector_c = (bench_type *)thread_info->c;
    int i, j;
    for (i = offset; i < offset + size / total_thread_num; i++) {
        vector_c[i] = vector_a[i] * vector_b[i];
    }
    pthread_exit(0);
}


void *do_matrix(void *param) {
    thread_info_t *thread_info = param;
#ifndef MAC
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_info->thread_num, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
#endif
    int size = thread_info->size_0;
    int total_thread_num = thread_info->total_thread_num;
    int offset = thread_info->thread_num * (size / total_thread_num);
    int i, j, k;
    bench_type *matrix_a = (bench_type *)thread_info->a;
    bench_type *matrix_b = (bench_type *)thread_info->b; 
    bench_type *matrix_c = (bench_type *)thread_info->c;
    for (i = offset; i < offset + size / total_thread_num; i++) {
        for (j = 0; j < size; j++) {
            bench_type sum = 0;
            for (k = 0; k < size; k++) {
                sum += matrix_a[i * size + k] * matrix_b[k * size + j];
            }
            matrix_c[i * size + j] = sum;
        }
    }
    pthread_exit(0);
}

void do_pr_helper(pr_bench_type *matrix, pr_bench_type *vector, pr_bench_type *vector_r, int offset, int size, int total_thread_num) {
    int i, j;

    for (i = offset; i < offset + size / total_thread_num; i++) {
        pr_bench_type sum = 0;
        for (j = 0; j < size; j++) {
            sum += matrix[i * size + j] * vector[j];
        }
        vector_r[i] = sum * 0.8 + 0.2 / size;
    }
}

void *do_pr(void *param) {


    thread_info_t *thread_info = param;
#ifndef MAC
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(thread_info->thread_num, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
#endif
    int size = thread_info->size_0;
    int total_thread_num = thread_info->total_thread_num;
    int offset = thread_info->thread_num * (size / total_thread_num);
    int i, j, k;
    pr_bench_type *matrix_a = (pr_bench_type *)thread_info->a;
    pr_bench_type *vector_b = (pr_bench_type *)thread_info->b;
    pr_bench_type *vector_c = (pr_bench_type *)thread_info->c;

    for (i = 0; i < PR_ITERATION; i++) {
#ifndef MAC
        int ret = 0;
        if (total_thread_num != 1) {
            ret = pthread_barrier_wait(&barrier_start);
            if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD) {
                perror("Could not wait on barrier_start.");
                exit(-1);
            }            
        }
#endif
        if (i % 2 == 0) {
            do_pr_helper(matrix_a, vector_b, vector_c, offset, size, total_thread_num);
        } else {
            do_pr_helper(matrix_a, vector_c, vector_b, offset, size, total_thread_num);
        }
#ifndef MAC
        if (total_thread_num != 1) {
            ret = pthread_barrier_wait(&barrier_end);
            if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD) {
                perror("Could not wait on barrier_end.");
                exit(-1);
            }            
        }
#endif
    }
    pthread_exit(0);
}

void *blank_runner(void *param) {
    pthread_exit(0);
}

int main(int argc, char *argv[]) {   
    if (argc != 2) {
        printf("Input: %s [Benchmark]\n", argv[0]);
        printf("Numbers for benchmark.\n");
        printf("1: Fir (5 Taps).\n");
        printf("2: Vector Multiplication.\n");
        printf("3: Matrix Multiplication.\n");
        printf("4: PageRanking.\n");
        printf("5: Blank Runner.\n");
        return 0;
    }
    int benchmark = atoi(argv[1]);

    struct timeval t0, t1;
    double timeuse;

    thread_info_t *thread_info;
    int i, j;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int ret;
#ifndef MAC
    if (pthread_barrier_init(&barrier_start, NULL, NUM_THREADS)) {
        perror("Could not create barrier_start.");
        return 0;
    }
    if (pthread_barrier_init(&barrier_end, NULL, NUM_THREADS)) {
        perror("Could not create barrier_end.");
        return 0;
    }
#endif
    thread_info = calloc(NUM_THREADS, sizeof(thread_info_t));
    if (!thread_info) {
        perror("calloc error.");
        return 0;
    }
    // Fir (5 Taps)
    if (benchmark == 1) {
        printf("Fir (5 Taps):\n");
        for (j = 2 * 1024; j <= 256 * 1024; j = j * 2) {
            for (i = 0; i < NUM_THREADS; i++) {
                if (i == 0) {
                    thread_info[0].a = (bench_type *)malloc(sizeof(bench_type) * (j + FIR_TAP - 1));
                    thread_info[0].b = (bench_type *)malloc(sizeof(bench_type) * j);
                    thread_info[0].size_0 = j;
                } else {
                    thread_info[i].a = thread_info[0].a;
                    thread_info[i].b = thread_info[0].b;
                    thread_info[i].size_0 = j;                    
                }
            }

            for (i = 0; i < j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
                ((bench_type *)(thread_info[0].b))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < NUM_THREADS; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = NUM_THREADS;
                ret = pthread_create(&thread_info[i].tid, &attr, do_fir, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %4dK; Time Use: %f s (Number of Cores: %d)\n", j / 1024, timeuse, NUM_THREADS);

            for (i = 0; i < j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
                ((bench_type *)(thread_info[0].b))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < 1; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = 1;
                ret = pthread_create(&thread_info[i].tid, &attr, do_fir, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %4dK; Time Use: %f s (Single-core)\n", j / 1024, timeuse);

            free(thread_info[0].a);
            free(thread_info[0].b);    
        }
        goto terminate;
    }
    // Vector Multiplication
    if (benchmark == 2) {
        printf("Vector Multiplication:\n");
        for (j = 2 * 1024; j <= 256 * 1024; j = j * 2) {
            for (i = 0; i < NUM_THREADS; i++) {
                if (i == 0) {
                    thread_info[0].a = (bench_type *)malloc(sizeof(bench_type) * j);
                    thread_info[0].b = (bench_type *)malloc(sizeof(bench_type) * j);
                    thread_info[0].c = (bench_type *)malloc(sizeof(bench_type) * j);
                    thread_info[0].size_0 = j;                    
                } else {
                    thread_info[i].a = thread_info[0].a;
                    thread_info[i].b = thread_info[0].b;
                    thread_info[i].c = thread_info[0].c;
                    thread_info[i].size_0 = j;                    
                }
            }
            for (i = 0; i < j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
                ((bench_type *)(thread_info[0].b))[i] = (bench_type)(i + 1);
                ((bench_type *)(thread_info[0].c))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < NUM_THREADS; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = NUM_THREADS;
                ret = pthread_create(&thread_info[i].tid, &attr, do_vector, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %4dK; Time Use: %f s (Number of Cores: %d)\n", j / 1024, timeuse, NUM_THREADS);

            for (i = 0; i < j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
                ((bench_type *)(thread_info[0].b))[i] = (bench_type)(i + 1);
                ((bench_type *)(thread_info[0].c))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < 1; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = 1;
                ret = pthread_create(&thread_info[i].tid, &attr, do_vector, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < 1; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %4dK; Time Use: %f s (Single-core)\n", j / 1024, timeuse);


            free(thread_info[0].a);
            free(thread_info[0].b); 
            free(thread_info[0].c);   
        }
        goto terminate;
    }
    // Matrix Multiplication
    if (benchmark == 3) {
        printf("Matrix Multiplication:\n");
        for (j = 32; j <= 512; j = j * 2) {
            for (i = 0; i < NUM_THREADS; i++) {
                if (i == 0) {
                    thread_info[0].a = (bench_type *)malloc(sizeof(bench_type) * j * j);
                    thread_info[0].b = (bench_type *)malloc(sizeof(bench_type) * j * j);
                    thread_info[0].c = (bench_type *)malloc(sizeof(bench_type) * j * j);
                    thread_info[0].size_0 = j;                    
                } else {
                    thread_info[i].a = thread_info[0].a;
                    thread_info[i].b = thread_info[0].b;
                    thread_info[i].c = thread_info[0].c;
                    thread_info[i].size_0 = j;                    
                }
            }
            for (i = 0; i < j * j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
                ((bench_type *)(thread_info[0].b))[i] = (bench_type)(i + 1);
                ((bench_type *)(thread_info[0].c))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < NUM_THREADS; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = NUM_THREADS;
                ret = pthread_create(&thread_info[i].tid, &attr, do_matrix, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %3d * %3d; Time Use: %f s (Number of Cores: %d)\n", j, j, timeuse, NUM_THREADS);

            for (i = 0; i < j * j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
                ((bench_type *)(thread_info[0].b))[i] = (bench_type)(i + 1);
                ((bench_type *)(thread_info[0].c))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < 1; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = 1;
                ret = pthread_create(&thread_info[i].tid, &attr, do_matrix, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < 1; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }            
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %3d * %3d; Time Use: %f s (Single-core)\n", j, j, timeuse);

            free(thread_info[0].a);
            free(thread_info[0].b);
            free(thread_info[0].c);
        }
        goto terminate;
    }
    // PageRanking
    if (benchmark == 4) {
        printf("PageRanking:\n");
        for (j = 32; j <= 512; j = j * 2) {
            for (i = 0; i < NUM_THREADS; i++) {
                if (i == 0) {
                    thread_info[0].a = (bench_type *)malloc(sizeof(bench_type) * j * j);
                    thread_info[0].b = (bench_type *)malloc(sizeof(bench_type) * j);
                    thread_info[0].c = (bench_type *)malloc(sizeof(bench_type) * j);
                    thread_info[0].size_0 = j;
                } else {
                    thread_info[i].a = thread_info[0].a;
                    thread_info[i].b = thread_info[0].b;
                    thread_info[i].c = thread_info[0].b;
                    thread_info[i].size_0 = j;                    
                }
            }
            for (i = 0; i < j * j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
            }
            for (i = 0; i < j; i++) {
                ((bench_type *)(thread_info[0].b))[i] = 1 / (bench_type)(j);
                ((bench_type *)(thread_info[0].c))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < NUM_THREADS; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = NUM_THREADS;
                ret = pthread_create(&thread_info[i].tid, &attr, do_pr, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %4d * %4d; Time Use: %f s (Number of Cores: %d)\n", j, j, timeuse, NUM_THREADS);

            for (i = 0; i < j * j; i++) {
                ((bench_type *)(thread_info[0].a))[i] = (bench_type)(i);
            }
            for (i = 0; i < j; i++) {
                ((bench_type *)(thread_info[0].b))[i] = 1 / (bench_type)(j);
                ((bench_type *)(thread_info[0].c))[i] = 0;
            }
            gettimeofday(&t0, NULL);
            for (i = 0; i < 1; i++) {
                thread_info[i].thread_num = i;
                thread_info[i].total_thread_num = 1;
                ret = pthread_create(&thread_info[i].tid, &attr, do_pr, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < 1; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %4d * %4d; Time Use: %f s (Single-core)\n", j, j, timeuse);

            free(thread_info[0].a);
            free(thread_info[0].b);
            free(thread_info[0].c);
        }
        goto terminate;
    } 
    // BlankRunner
    if (benchmark == 5) {
        printf("BlankRunner:\n");
        for (j = 32; j <= 512; j = j * 2) {
            gettimeofday(&t0, NULL);
            for (i = 0; i < NUM_THREADS; i++) {
                thread_info[i].thread_num = i;
                ret = pthread_create(&thread_info[i].tid, &attr, blank_runner, &thread_info[i]);
                if (0 != ret) {
                    perror("pthread create error.");
                    free(thread_info);
                    return 0;
                }
            }
            for (i = 0; i < NUM_THREADS; i++) {
                pthread_join(thread_info[i].tid, NULL);
            }
            gettimeofday(&t1, NULL);
            timeuse = t1.tv_sec - t0.tv_sec + (t1.tv_usec - t0.tv_usec) / 1000000.0;
            printf("Data Size: %3d * %3d; Time Use: %f s (Number of Cores: %d)\n", j, j, timeuse, NUM_THREADS);
        }
        goto terminate;
    }
terminate:
    free(thread_info);
    return 0;
}