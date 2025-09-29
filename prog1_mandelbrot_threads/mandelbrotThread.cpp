#include <stdio.h>
#include <thread>
#include <chrono>
#include <vector>

#include "CycleTimer.h"

typedef struct {
    float x0, x1;
    float y0, y1;
    unsigned int width;
    unsigned int height;
    int maxIterations;
    int* output;
    int threadId;
    int numThreads;
} WorkerArgs;


extern void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int numRows,
    int maxIterations,
    int output[]);


extern void mandelbrotSerialInterleaved(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int s,
    int maxIterations,
    int output[]);


struct ThreadWork {
    int startRow;
    int numRows;
};

ThreadWork getThreadRowsNaive(int threadId, int height, int numThreads) {
    int numRows = height / numThreads;
    int startRow = threadId * numRows;

    // Handle rounding 
    if (threadId == numThreads - 1){
        numRows = height - startRow;
    }

    return {startRow, numRows};
}

//
// workerThreadStart --
//
// Thread entrypoint.
void workerThreadStartNaive(WorkerArgs * const args) {

    // TODO FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.
    auto start = std::chrono::high_resolution_clock::now();

    printf("Hello world from thread %d\n", args->threadId);

    auto threadWork = getThreadRowsNaive(args->threadId, args->height, args->numThreads);

    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, args->width, args->height, threadWork.startRow, threadWork.numRows, args->maxIterations, args->output);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    printf("Thread %d took %ld ms\n", args->threadId, ms);
}

void workerThreadStartInterleaved(WorkerArgs * const args) {

    // TODO FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.
    auto start = std::chrono::high_resolution_clock::now();

    // printf("Hello world from thread %d\n", args->threadId);

    mandelbrotSerialInterleaved(args->x0, args->y0, args->x1, args->y1, args->width, args->height, args->threadId, args->numThreads, args->maxIterations, args->output);

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // printf("Thread %d took %ld ms\n", args->threadId, ms);
}

//
// MandelbrotThread --
//
// Multi-threaded implementation of mandelbrot set image generation.
// Threads of execution are created by spawning std::threads.
void mandelbrotThread(
    int numThreads,
    float x0, float y0, float x1, float y1,
    int width, int height,
    int maxIterations, int output[])
{
    static constexpr int MAX_THREADS = 32;

    if (numThreads > MAX_THREADS)
    {
        fprintf(stderr, "Error: Max allowed threads is %d\n", MAX_THREADS);
        exit(1);
    }

    // Creates thread objects that do not yet represent a thread.
    std::thread workers[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    for (int i=0; i<numThreads; i++) {
      
        // TODO FOR CS149 STUDENTS: You may or may not wish to modify
        // the per-thread arguments here.  The code below copies the
        // same arguments for each thread
        args[i].x0 = x0;
        args[i].y0 = y0;
        args[i].x1 = x1;
        args[i].y1 = y1;
        args[i].width = width;
        args[i].height = height;
        args[i].maxIterations = maxIterations;
        args[i].numThreads = numThreads;
        args[i].output = output;
      
        args[i].threadId = i;
    }

    // Spawn the worker threads.  Note that only numThreads-1 std::threads
    // are created and the main application thread is used as a worker
    // as well.
    for (int i=1; i<numThreads; i++) {
        workers[i] = std::thread(workerThreadStartInterleaved, &args[i]);
    }
    
    workerThreadStartInterleaved(&args[0]);

    // join worker threads
    for (int i=1; i<numThreads; i++) {
        workers[i].join();
    }
}

