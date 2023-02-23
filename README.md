
SUPPLIER CONSUMER INVENTORY MANAGEMENT


The implementation of a multiple Supplier-consumer system where producers create prioritized jobs using semaphores can be summarized as follows:



Data Structures:

1-> JOB:
 a structure containing information about a job, such as producer process ID, producer number, priority, compute time, and job ID.

2->priority_queue:
 a structure representing a priority queue of jobs, implemented as an array with a back pointer.

3->Segment:
 a structure representing a shared memory segment containing a priority queue of jobs, as well as counters for the number of jobs created and completed.


Algorithms and Functions:


1-> Main Function:

Takes the number of producers, consumers, and jobs as user input.
Creates a shared memory segment and initializes the priority queue and counters.
Forks a process for each producer and consumer, and waits for all processes to finish.
Cleans up the shared memory segment.
Calculates and prints the total execution time of the program.


2-> Producer:

Generates a new job with random values for priority and compute time.
Locks the shared memory segment using a semaphore before inserting the job.
Inserts the job into the priority queue in sorted order based on priority.
Releases the lock and repeats the process.


3-> Consumer:

Locks the shared memory segment using a semaphore before removing a job.
Retrieves the highest priority job from the priority queue and removes it.
Releases the lock and sleeps for the compute time of the retrieved job.

4-> Priority Queue:

Implements a priority queue using functions such as init_queue, isEmpty, isFull, enqueue, and dequeue.

5-> Shared Memory:
Uses semaphores to lock the critical section of the program, i.e., whenever the shared memory segment is accessed by a process.


Overall, this implementation ensures that producers create prioritized jobs and consumers retrieve and process them in order of priority, while avoiding race conditions using semaphores.