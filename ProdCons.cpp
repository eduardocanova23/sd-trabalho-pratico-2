#include <iostream>       
#include <semaphore.h>
#include <random>
#include <thread>
#include <time.h>

using namespace std;

vector<int> memory_vec;
sem_t empty_positions;
sem_t full_positions;
int to_process = 100000;
int memory_size;
sem_t mutex_memory_access;

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
    
     

    while(to_process > 0)
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
        
        sem_post(&mutex_memory_access);
        sem_post(&full_positions);
    }
    sem_post(&full_positions);
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
    sem_post(&empty_positions);
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

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    for (int i = 0; i < amount_producers; i++) {
        threads.push_back(thread(Producer));
    }
    for (int i = 0; i < amount_consumers; i++){
        threads.push_back(thread(Consumer));
    }
    for (auto& th : threads){
        th.join();
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    double duration = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    std::cout << "Execution time: " << duration << " seconds" << std::endl;

    return 0;
}
