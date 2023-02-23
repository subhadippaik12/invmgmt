#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <iostream>
using namespace std;


class  JOB{
public:
    int pid;           
    int pno;             
    int priority;               
    int compute_time;           
    int job_id;                 
} ;

class priority_queue {
public:
    priority_queue() : back(0) {}       
    bool isEmpty() const { return back == 0; }
    bool isFull() const { return back == QUEUE_SIZE; }
    bool enqueue(JOB job) {
        if (isFull()) return false;
        job_queue[back++] = job;
        return true;
    }
    JOB dequeue() {
        if (isEmpty()) return JOB();    
        JOB job = job_queue[0];
        for (int i = 1; i < back; ++i)
            job_queue[i-1] = job_queue[i];
        --back;
        return job;
    }
  void print() const {
        for (int i = 0; i < back; ++i)
            std::cout << job_queue[i].job_id << " ";
        std::cout << std::endl;
    }
private:
    static const int QUEUE_SIZE = 50;
    JOB job_queue[QUEUE_SIZE];      
    int back;                      
};

typedef struct {
    priority_queue pq;      
    int job_created;
    int job_completed;      
} Segment;


void init_SHM(Segment* shmseg) {
    shmseg->job_completed = 0;
    shmseg->job_created = 0;
    shmseg->pq = priority_queue();
}

SMT *create_SHM(int *shm_id) {

 if (shmid == -1) {
    perror("Shared Memory Segment Creation Failed");
    exit(EXIT_FAILURE);
}

SMT* shmseg = (SMT*)shmat(shmid, NULL, 0); 
if (shmseg == (void*)-1) {
    perror("Shared Memory Segment Attachment Failed");
    exit(EXIT_FAILURE);
}

*shm_id = shmid; 

memset(shmseg, 0, sizeof(SMT)); 
return shmseg; 
