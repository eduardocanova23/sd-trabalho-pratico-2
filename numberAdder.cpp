#include "./Spinlock.h"
#include <ctime>
#include <pthread.h>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <chrono>

int totalSum = 0;
Spinlock lock;

struct ThreadParams {
    int start;
    int end;
    char* numbers; // pointer to the 1st address
    int id;
};

void setRandomArray(char* arr, int size) {
    srand(time(NULL));
    for (int i = 0; i < size; i++) {
        int value = rand() % 201 - 100; 
        arr[i] = static_cast<char>(value);
    }
}

void* adder(void* arg){
    ThreadParams* params = static_cast<ThreadParams*>(arg); // setting params type as ThreadParams
    char* localNumbers = new char[params->end - params->start + 1]; // allocating memory for localNumbers
    std::copy(params->numbers + params->start, params->numbers + params->end + 1, localNumbers); 
    
    int localTotal = 0;
    int size = params->end - params->start + 1; // size of the local array

    /*
        We increase i in 2 every loop.
        In the last loop, we do arr[i] + 0. 
    */
    for(int i = 0; i < size; i+=2){
        int a = static_cast<int>(localNumbers[i]);
        int b = 0;
        if (i != size - 1){
            b += static_cast<int>(localNumbers[i+1]); 
        }
        localTotal += a + b;
    }
    lock.acquire();
    totalSum += localTotal;
    lock.release();
    delete[] localNumbers;
    pthread_exit(NULL);
}

int main(int argc, char* argv[]){
    int N, K;
    std::istringstream(argv[1]) >> N; 
    std::istringstream(argv[2]) >> K;

    char* numbers = new char[N];
    setRandomArray(numbers, N);

    // control adder
    int controlTotal = 0;
    for(int i = 0; i < N; i+=2){
        int a = static_cast<int>(numbers[i]);
        int b = 0;
        if (i != N - 1){
            b += static_cast<int>(numbers[i+1]);
        }
        controlTotal += a + b;
    }
    
    // threads 
    auto start = std::chrono::high_resolution_clock::now();
    pthread_t* thread = new pthread_t[K];
    ThreadParams* params = new ThreadParams[K];

    for (int i = 0; i < K; i++) {
        // getting range
        int start = i * (N/K); 
        int end = (i == K - 1) ? (N-1) : ( (N/K) * (i + 1) - 1 );

        // passing adder parameters
        params[i].start = start;
        params[i].end = end;
        params[i].numbers = numbers;
        params[i].id = i;

        // uncomment if more memory is needed
        // pthread_attr_t attr;
        // pthread_attr_init(&attr);
        // pthread_attr_setstacksize(&attr, 16 * 1024 * 1024); // 16 MB stack size
        // pthread_create(&thread[i], &attr, adder, &params[i]);
        // pthread_attr_destroy(&attr);

        // normal thread creation
        pthread_create(&thread[i], NULL, adder, &params[i]);
    }

    for (int i = 0; i < K; i++){
        pthread_join(thread[i], NULL);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Duration: " << duration << " microseconds" << std::endl;

    delete[] numbers;
    delete[] params;
    delete[] thread;

    if ( controlTotal != totalSum ){
        std::cerr << "Error: different results" << std::endl;
    }

    std::cout << "The total sum is: " << totalSum << "\n";

    return 0;
}