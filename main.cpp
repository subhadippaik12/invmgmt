#include <iostream>
#include<bits/stdc++.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <bits/stdc++.h>
#include <cstring>
#include <signal.h>
using namespace std::chrono;
#include <sys/sem.h>
using namespace std;

struct sembuf signalSemaphore(int index){
    struct sembuf sem_op;
    sem_op.sem_num = index;
    sem_op.sem_op = 1;
    sem_op.sem_flg = 0;
    return sem_op;
}
struct sembuf waitSemaphore(int index){
    struct sembuf sem_op;
    sem_op.sem_num = index;
    sem_op.sem_op = -1;
    sem_op.sem_flg = 0;
    return sem_op;
}


map<string,int>commodityToIndex;
#define ANSI_COLOR_RED     "\033[;31m"
#define ANSI_COLOR_BLUE   "\033[34m"
#define ANSI_COLOR_GREEN   "\033[;32m"
#define ANSI_COLOR_RESET   "\033[0m"


key_t IPC_key;
void* sharedMemory;
int sharedMemoryID;
map<string,int>indexOf;
map<int,vector<double>>prevValues;
map<int,double>prevAverageValues;
pair<int,double>* prices;
int bufferSize;
int* currentItem;
int* currentSize;
int semaphoreSetId;

string COMMODITIES[11]= {"ALUMINUM  ", "COPPER    ", "COTTON    ", "CRUDEOIL  ", "GOLD      ","LEAD      ", "MENTHAOIL ", "NATURALGAS", "NICKEL    ", "SILVER    ", "ZINC      "};

void updateCommodityPrice(int commodityIndex,double currentPrice){
    printf("\033[1;1H");
    if(prevValues[commodityIndex].size()==0){//aka empty
        printf("\033[%d;%dH",(4+commodityIndex),14);
        printf(ANSI_COLOR_GREEN);
        printf("%7.2lf ↑|",currentPrice);
        printf(ANSI_COLOR_RESET);
        prevValues[commodityIndex].push_back(currentPrice);
    }else{
        double previousValue = prevValues[commodityIndex][prevValues[commodityIndex].size()-1];
        printf("\033[%d;%dH",(4+commodityIndex),14);
        if(currentPrice-previousValue>0.001){
            printf(ANSI_COLOR_GREEN);
            printf("%7.2lf ↑",currentPrice);
            printf(ANSI_COLOR_RESET);
        }else if (previousValue-currentPrice>0.001){
            printf(ANSI_COLOR_RED);
            printf("%7.2lf ↓",currentPrice);
            printf(ANSI_COLOR_RESET);
        }else{
            printf("%7.2lf -",currentPrice);
        }
        cout<<"|";
        prevValues[commodityIndex].push_back(currentPrice);
        if(prevValues[commodityIndex].size()>5)
            prevValues[commodityIndex].erase(prevValues[commodityIndex].begin());
    }
    int sum =0,j=0;
    for(j=0;j<5 && j<prevValues[commodityIndex].size();j++){
        sum+=prevValues[commodityIndex][j];
    }
    double average = sum*1.0/j;
    if(prevAverageValues[commodityIndex]<0.0001){
        printf(ANSI_COLOR_GREEN);
        printf("%7.2lf ↑",average);
        printf(ANSI_COLOR_RESET);
    }else{
        double prevAverageValue = prevAverageValues[commodityIndex];
        if(average-prevAverageValue>0.0001){
            printf(ANSI_COLOR_GREEN);
            printf("%7.2lf ↑",average);
            printf(ANSI_COLOR_RESET);
        }else if(prevAverageValue-average>0.0001){
            printf(ANSI_COLOR_RED);
            printf("%7.2lf ↓",average);
            printf(ANSI_COLOR_RESET);
        }else{
            printf("%7.2lf -",average);
        }
    }
    prevAverageValues[commodityIndex]=average;
}


void consume(){
    IPC_key = ftok("interprocesscommunication",65);
    sharedMemoryID = shmget(IPC_key,(bufferSize*16)+8,0666|IPC_CREAT); 
    sharedMemory= shmat(sharedMemoryID,NULL,0);
    memset(sharedMemory,0,(bufferSize*16)+8);

    currentSize = (int *) sharedMemory;
    currentItem= (int *) sharedMemory+4;

    prices = (pair<int,double>*)sharedMemory+8;

    struct sembuf sem_op;
    semaphoreSetId = semget(IPC_key,3,0666 | IPC_CREAT );
    semctl(semaphoreSetId,0,SETVAL,1); //mutex at index 0
    semctl(semaphoreSetId,1,SETVAL,bufferSize); // empty at index 1
    semctl(semaphoreSetId,2,SETVAL,0); //full at index 2

    while(true){
        sem_op = waitSemaphore(2);
        semop(semaphoreSetId,&sem_op,1);
        sem_op = waitSemaphore(0);
        semop(semaphoreSetId,&sem_op,1);
        cout<<"|"<<endl;
        pair<int,double>p = prices[*currentItem];
        *currentItem = (*currentItem+1)%bufferSize;
        int commodityIndex = p.first;
        double commodityPrice = p.second;
        updateCommodityPrice(commodityIndex,commodityPrice);
        sem_op = signalSemaphore(0);
        semop(semaphoreSetId,&sem_op,1);
        sem_op = signalSemaphore(1);
        semop(semaphoreSetId,&sem_op,1);
    }
}   
void signalHandler(int signum){
    printf("\033[16;1H");
    shmdt(sharedMemory);
    shmctl(sharedMemoryID,IPC_RMID,NULL);
    exit(signum);
}

void initializeMap(){
    commodityToIndex["ALUMINUM"]=0;
    commodityToIndex["COPPER"]=1;
    commodityToIndex["COTTON"]=2;
    commodityToIndex["CRUDEOIL"]=3;
    commodityToIndex["GOLD"]=4;
    commodityToIndex["LEAD"]=5;
    commodityToIndex["MENTHAOIL"]=6;
    commodityToIndex["NATURALGAS"]=7;
    commodityToIndex["NICKEL"]=8;
    commodityToIndex["SILVER"]=9;
    commodityToIndex["ZINC"]=10;
}

int main(int argc, char** argv){
  signal(SIGINT,signalHandler);
    signal(SIGTSTP,signalHandler);
    signal(SIGTERM,signalHandler);
    signal(SIGABRT,signalHandler);
    bufferSize= stoi(argv[1]);
    printf("\e[1;1H\e[2J");
    cout<<"+-------------------------------------+"<<endl;
    cout<<"| Currency  |  Price  | AvgPrice |"<<endl;
    cout<<"+-------------------------------------+"<<endl;
    for(int i=0;i<11;i++){
        cout<<"| "<<COMMODITIES[i];
        printf("|");
        printf(ANSI_COLOR_BLUE);
        printf("%7.2lf  ",0.0);
        printf(ANSI_COLOR_RESET);
        printf("|");
        printf(ANSI_COLOR_BLUE);
        printf("%7.2lf  ",0.0);
        printf(ANSI_COLOR_RESET);
        printf("|\n");
    }
    cout<<"+-------------------------------------+"<<endl;
    consume();
    
    initializeMap();
    string commodityName = argv[1];
    double commodityMean = stoi(argv[2]);
    double standardDeviation = stoi(argv[3]);
    double timeOut = stoi(argv[4]);
    int bufferSize = stoi(argv[5]);
    default_random_engine randomVariablegenerator;
    normal_distribution<double> normalDistribution(commodityMean,standardDeviation);
    int commodityIndex = commodityToIndex[commodityName];

    key_t IPC_key = ftok("interprocesscommunication",65);
    int sharedMemoryID = shmget(IPC_key,(bufferSize*16)+8,0666);
    void* sharedMemory= shmat(sharedMemoryID,NULL,0);
    int* currentSize = (int *) sharedMemory;
    int *currentItem = (int *) sharedMemory+4;

    struct sembuf sem_op;
    int semaphoreSetId =semget(IPC_key,3,0666);
    pair<int,double>*array = (pair<int,double>* )sharedMemory+8;

    while(true){
        double number = normalDistribution(randomVariablegenerator);
        struct timespec start;
        time_t now = system_clock::to_time_t(system_clock::now());
        cerr<<"["<<put_time(localtime(&now),"%M/%d/%Y %H:%M:%S")<<"] "<<commodityName<<": generating a new value "<<setprecision(2)<<fixed<< number<<endl;;
        sem_op = waitSemaphore(1);
        semop(semaphoreSetId,&sem_op,1);
        cerr<<"trying to get mutex on shared buffer"<<endl;
        sem_op = waitSemaphore(0);
        semop(semaphoreSetId,&sem_op,1);
        cerr<<"placing "<<number<<" on shared buffer"<<endl;
        array[*currentSize].first= commodityIndex;
        array[*currentSize].second=number;
        *currentSize = (*currentSize+1)%bufferSize;
        sem_op = signalSemaphore(0);
        semop(semaphoreSetId,&sem_op,1);
        sem_op = signalSemaphore(2);
        semop(semaphoreSetId,&sem_op,1);
        cerr<<"sleeping for "<<timeOut<<" ms"<<endl;
        usleep(timeOut*1000);
    }
    
    return 0;
}
