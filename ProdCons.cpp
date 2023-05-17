#include <iostream>       
#include <semaphore.h>
#include <random>
#include <thread>
#include <time.h>
#include <ctime>
#include <chrono>
#include "gnuplot-iostream.h"
using namespace std;

vector<int> memory_tracker; // vector to store all the times memory was used
vector<double> timestamps;
vector<int> memory_vec;
sem_t mutex_memory_access;
sem_t empty_positions;
sem_t full_positions;
int to_process = 100000;
int to_produce = 100000;
int memory_size;
int num_full_positions;

int isPrime(int number){
    bool isPrime = true;

    for (int i = sqrt(number); i > 1; i--){
        if(number % i == 0){
            return 0;
        }
    }
    return 1; 
}

void Producer()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 10000000);
  
    while(to_produce > 0)
    {
        int random_int = distrib(gen);  
        sem_wait(&empty_positions);
        sem_wait(&mutex_memory_access);

        int index = -1;
        for (int i = 0; i < memory_vec.size(); i++) {
            if (memory_vec[i] == 0) {
            index = i;
            break;
            }
        }
        if (index != -1) {
        memory_vec[index] = random_int;
        }
        
        num_full_positions++;
        memory_tracker.push_back(num_full_positions);

        clock_t start = clock();
        timestamps.push_back(start);

        to_produce--;

        sem_post(&mutex_memory_access);
        sem_post(&full_positions);
    }
    sem_post(&empty_positions);
}

void Consumer()
{   
    int collected_number;
    while(to_process > 0) {
        sem_wait(&full_positions);
        sem_wait(&mutex_memory_access);
            int index = -1;
            for (int i = 0; i < memory_vec.size(); i++) {
                if (memory_vec[i] != 0) {
                    index = i;
                    break;
                }
            }
            if (index != -1) {
                collected_number = memory_vec[index];
                memory_vec[index] = 0;
            }    

            to_process--;

            num_full_positions--;
            memory_tracker.push_back(num_full_positions);

            clock_t start = clock();
            timestamps.push_back(start);

        sem_post(&mutex_memory_access);
        sem_post(&empty_positions);

        if (isPrime(collected_number)){
            printf("%d IS prime\n", collected_number);
        }
        else{
            printf("%d is NOT prime\n", collected_number);
        }
    }

    sem_post(&full_positions);
}

int main(int argc, char* argv[])
{
    memory_size = atoi(argv[1]);
    int amount_producers = atoi(argv[2]);
    int amount_consumers = atoi(argv[3]);
    memory_vec.resize(memory_size);

    sem_init(&full_positions,0,0);
    sem_init(&empty_positions,0,memory_size);
    sem_init(&mutex_memory_access,0,1);
    
    vector<thread> threads;

    auto start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < amount_producers; i++) {
        threads.push_back(thread(Producer));
    }
    for (int i = 0; i < amount_consumers; i++){
        threads.push_back(thread(Consumer));
    }
    for (auto& th : threads){
        th.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "Duration: " << duration << " microseconds" << std::endl;

    std::vector<int> beginning_mem_tracker;
    beginning_mem_tracker.resize(400);

    std::vector<int> beginning_timestamps;
    beginning_timestamps.resize(400);

    // Copy the first 500 elements of the source vector to the destination vector
    beginning_mem_tracker.assign(memory_tracker.begin(), memory_tracker.begin() + 400);

    // Copy the first 500 elements of the source vector to the destination vector
    beginning_timestamps.assign(timestamps.begin(), timestamps.begin() + 400);

    

    Gnuplot gp;

    // Set terminal and output file
    gp << "set terminal png size 1000,750\n";
    gp << "set output 'myplot.png'\n";

    gp << "set title 'My Plot'\n";
    gp << "set xlabel 'Time elapsed'\n";
    gp << "set ylabel 'Amount of full spaces in memory'\n";

    // Plot data
    gp << "plot '-' with lines title 'My Data'\n";
    gp.send1d(std::make_tuple(beginning_timestamps, beginning_mem_tracker));

    // Wait for user to close plot window
    std::cout << "Press enter to exit." << std::endl;
    std::cin.get();

    return 0;
}

// sudo apt-get install libgnuplot-iostream-dev
// g++ -pthread ProdCons.cpp -o pc -L/usr/lib -lboost_filesystem -lboost_system -lboost_iostreams
// feh myplot.png
