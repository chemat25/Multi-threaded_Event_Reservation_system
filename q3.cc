#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>

using namespace std;

const int events = 100;
const int maxq = 5;
const int capacity = 500;
const int maxtickets = 10;
const int mintickets = 5;
const int workerThreads = 20;
const int runTime = 70;

vector<int> queries(maxq,0);
vector<int> seatsleft(events,capacity);

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t smutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qcond = PTHREAD_COND_INITIALIZER;

int freeQuery()
{
    for(int i=0; i<maxq; i++)
    {
        if(queries[i]==0)
        {
            return i;
        }
    }
    return -1;
}

void inquire(int event, int thread)
{
    pthread_mutex_lock(&qmutex);
    while(freeQuery()==-1)
    {
        cout<<"Thread "<<thread<<" WAITING "<<endl;
        pthread_cond_wait(&qcond, &qmutex);
    }
    int ind = freeQuery();
    queries[ind] = event;
    pthread_mutex_unlock(&qmutex);

    pthread_mutex_lock(&smutex);
    cout<<"Thread "<<thread<<" INQUIRING "<<seatsleft[event]<<" seats available for event "<<event<<endl;
    pthread_mutex_unlock(&smutex);

    pthread_mutex_lock(&qmutex);
    queries[ind] = 0;
    pthread_mutex_unlock(&qmutex);
    cout<<"Thread "<<thread<<" SIGNALED "<<endl;
    pthread_cond_signal(&qcond);

}

void book(int event, int bookings, int thread)
{
    pthread_mutex_lock(&qmutex);
    while(freeQuery()==-1)
    {
        cout<<"Thread "<<thread<<" WAITING "<<endl;
        pthread_cond_wait(&qcond, &qmutex);
    }
    int ind = freeQuery();
    queries[ind] = event;
    pthread_mutex_unlock(&qmutex);

    pthread_mutex_lock(&smutex);
    if(bookings <= seatsleft[event])
    {
        seatsleft[event] -= bookings;
        cout<<"Thread "<<thread<<" BOOKED "<<bookings<<" seats for event "<<event<<endl;
    }
    else
    {
        cout<<"Thread "<<thread<<" Not enough seats available to BOOK for event "<<event<<endl;
    }
    pthread_mutex_unlock(&smutex);

    pthread_mutex_lock(&qmutex);
    queries[ind] = 0;
    pthread_mutex_unlock(&qmutex);
    cout<<"Thread "<<thread<<" SIGNALED "<<endl; 
    pthread_cond_signal(&qcond);

}

void cancel(int event, int cancels, int thread)
{
    pthread_mutex_lock(&qmutex);
    while(freeQuery()==-1)
    {
        cout<<"Thread "<<thread<<" WAITING "<<endl;
        pthread_cond_wait(&qcond, &qmutex);
    }
    int ind = freeQuery();
    queries[ind] = event;
    pthread_mutex_unlock(&qmutex);

    pthread_mutex_lock(&smutex);
    if(cancels <= (capacity-seatsleft[event]))
    {
        seatsleft[event] -= cancels;
        cout<<"Thread "<<thread<<" CANCELED "<<cancels<<" seats for event "<<event<<endl;
    }
    else
    {
        cout<<"Thread "<<thread<<"Not enough seats to CANCEL for event "<<event<<endl;
    }
    pthread_mutex_unlock(&smutex);

    pthread_mutex_lock(&qmutex);
    queries[ind] = 0;
    pthread_mutex_unlock(&qmutex);
    cout<<"Thread "<<thread<<" SIGNALED "<<endl;
    pthread_cond_signal(&qcond);

}

void* userThread(void* tid)
{
    int thread = *((int*) tid);
    srand(time(NULL) + thread);
    
    int c=0;
    while(c < runTime)
    {
        int qtype = rand()%3;
        int event = rand()%events + 1;

        switch(qtype)
        {
            case 0:
            {
                inquire(event,thread);
                sleep(1);
                break;
            }
            case 1:
            {
                int bookings = rand()%6 + 5;
                book(event,bookings,thread);
                sleep(1);
                break;
            }
            case 2:
            {
                int cancels = rand()%6 + 5;
                cancel(event,cancels,thread);
                sleep(1);
                break;
            }
        }
        c++;

    } 
    
}

int main()
{
    pthread_t threads[workerThreads];
    int a[workerThreads];
    for(int i=0; i<workerThreads; i++)
    {
        a[i] = i;
        void* temp = &a[i];
        pthread_create(&threads[i], NULL, userThread, temp);
    }

    sleep(2);
    
    for(int i=0; i<workerThreads; i++)
    {
        pthread_join(threads[i],NULL);
    }

    cout<<endl<<endl<<"--------RESULTS----------"<<endl;
    for(int i=1; i<events; i++)
    {
        cout<<setprecision(2)<<fixed;
        double s=(double)seatsleft[i]; 
        double cap=(double)capacity; 
        double per = ((cap-s)/cap)*100;
        cout<<"EVENT "<<i<<": "<<per<<" percent full, with remaining seats "<<seatsleft[i]<<endl;
    }

    pthread_mutex_destroy(&qmutex);
    pthread_mutex_destroy(&smutex);
    pthread_cond_destroy(&qcond);

    return 0;

}