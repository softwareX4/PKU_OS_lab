// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "elevatortest.h"
// testnum is set in main.cc
int testnum = 4;

//-----------Lab3------------
#include "synch.h"

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}


//---------------------------------------------------------------------------
//exercise 2 && 3-1
//create 129 threads 
//print thread infos
//------------------------------------------------------------------------------

void ThreadMaxNumTest(){
    for(int i = 0; i <= MAX_THREAD_NUM ;++i){
       try{

        Thread *t = new Thread("t");
        
       }
       catch(const char* msg){
           printf(msg);
       }

    }

}



//----------------------------------------------------------------------
// TSThread
//----------------------------------------------------------------------

void
TSThread(int which)
{
   // printf("*** current thread (uid=%d, tid=%d, pri=%d name=%s) => ", currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(), currentThread->getName());
   printf("*** current thread (uid=%d, tid=%d, pri=%d, name=%s) => ", currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(),currentThread->getName());
    IntStatus oldLevel; // for case 1 sleep (avoid cross initialization problem of switch case)
    switch (which)
    {
        case 0:
            printf("Yield\n");
            //Relinquish the CPU ，Print
            scheduler->Print();
            printf("\n\n");
            currentThread->Yield();
            break;
        case 1:
            printf("Sleep\n");
            oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
            currentThread->Sleep();
            (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
            break;
        case 2:
            printf("Finish\n");
            currentThread->Finish();
            break;
        default:
            printf("Yield (default)\n");
            currentThread->Yield();
            break;
    }
}


//----------------------------------------------------------------------
// Lab1 Exercise3-2
// 	Create some threads and use TS to show the status
//----------------------------------------------------------------------

void
TSTest()
{
    DEBUG('t', "Entering Lab1 Exercise3_2");

    Thread *t1 = new Thread("Yield_1");

    Thread *t2 = new Thread("Sleep_2");
    Thread *t3 = new Thread("Finish_3");

    t1->Fork(TSThread, (void*)0);
    t2->Fork(TSThread, (void*)1);
    t3->Fork(TSThread, (void*)2);

    Thread *t4 = new Thread("fork_4");

    TSThread(0); 

    printf("--- Calling TS command ---\n");
    TS();
    printf("--- End of TS command ---\n");
}


void
priThread(int which)
{
   // printf("*** current thread (uid=%d, tid=%d, pri=%d name=%s) => ", currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(), currentThread->getName());
   printf("*** current thread (uid=%d, tid=%d, pri=%d, name=%s) \n", currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(),currentThread->getName());
 
}
//----------------------------------------------------------------------
//exercise  5
// if preemptive,use priThread and note off code in scheduler.cc/Scheduler::ReadyToRun
//   and change Thread:non-parameter constructor 's pri = 10 (because 0 is supreme pri)
//   main will never Relinquish CPU
//if not preemptive , use TSThread  
//----------------------------------------------------------------------
void priTest(){
     DEBUG('t', "Entering Lab1 Exercise5"); 

    Thread *t1 = new Thread("high ",0);
    Thread *t2 = new Thread("mid ",5);
    Thread *t3 = new Thread("low",30);
#ifdef USE_PREEMPTIVE
    t2->Fork(priThread, (void*)0);
    t1->Fork(priThread, (void*)0);
    t3->Fork(priThread, (void*)0);
    priThread(0);
#else

    t2->Fork(TSThread, (void*)0);
    t1->Fork(TSThread, (void*)0);
    t3->Fork(TSThread, (void*)0);
     TSThread(0); 

#endif 

   
    printf("--- Calling TS command ---\n");
    TS();
    printf("--- End of TS command ---\n");
}


//---------------------------------------------------------------------
//Thread Round Robin 
//---------------------------------------------------------------------
void ThreadRR(int dummy){
    int ut = currentThread->getUsedTime();
    int rt = currentThread->getRunTime();
        
    
        while(ut < rt){
            ut += SystemTick;
            currentThread->setUsedTime(ut);   //increase used time
            currentThread->setPriority(currentThread->getPriority() + PRI_DEGRADE);  //drop pri
            printf("***thread %s run %d SystemTicks (need:%d,pri:%d) system time:%d\n",currentThread->getName(),currentThread->getUsedTime(),rt,currentThread->getPriority(),stats->systemTicks);
            
            interrupt->OneTick();
        }
    
        printf("\nFINISHED THREAD %s ! (total:%d,used:%d)\n\n",currentThread->getName(),rt,currentThread->getUsedTime());
        
    printf("--- Calling TS command ---\n");
    TS();
    printf("--- End of TS command ---\n");
        currentThread->Finish();
    
}


//--------------------------------------------------------------------
//Lab1 challenge: RR Test
//--------------------------------------------------------------------
void RRTest(){
     DEBUG('t', "Entering Lab1 challenge");
     printf("\nSystem initial ticks:\tsystem=%d, user=%d, total=%d\n", stats->systemTicks, stats->userTicks, stats->totalTicks);

    Thread *t1 = new Thread("11 ",1,11);
    Thread *t2 = new Thread("5 ",4,5);
    Thread *t3 = new Thread("2",3,2);
    Thread *t4 = new Thread("20",2,20);

    printf("\nAfter new Thread ticks:\tsystem=%d, user=%d, total=%d\n", stats->systemTicks, stats->userTicks, stats->totalTicks);

    t1->Fork(ThreadRR, (void*)0);
    t2->Fork(ThreadRR, (void*)0);
    t3->Fork(ThreadRR, (void*)0);
    t4->Fork(ThreadRR, (void*)0);

    printf("\nAfter 4 fork() ticks:\tsystem=%d, user=%d, total=%d\n\n", stats->systemTicks, stats->userTicks, stats->totalTicks);
    currentThread->Yield();

}


//--------------------Lab 3--------------------------------------------
Buffer * buffer;
Product   produce(int value){
        printf("Producing item with value %d\n", value);
        Product  item;
        item.value = value;
        return item;
    }

//Consumer
void consume(Product * p){
        printf("Consuming item with value %d\n", p->value);
    }

void Produce_Thread(int n){
    for(int i = 1; i <= n;++i){
        
        printf("## %s ##: ", currentThread->getName());
        Product item = produce(i);
        
        buffer->putItemInBuffer(item);
        interrupt->OneTick();
    }
    buffer->printBuffer();
}


void Consume_Thread(int n){
    for(int i = 1; i <= n; ++i){
        printf("$$ %s $$: ", currentThread->getName());
        Product * item = buffer->removeItemFromBuffer();
        consume(item);
        interrupt->OneTick();
    }
    buffer->printBuffer();
}

void
Lab3ProducerConsumer()
{
    buffer = new Buffer();
    DEBUG('t', "Entering Lab3ProducerConsumer");

    Thread *producer1 = new Thread("Producer 1");
    Thread *producer2 = new Thread("Producer 2");
    Thread *consumer1 = new Thread("Consumer 1");
    Thread *consumer2 = new Thread("Consumer 2");

    producer1->Fork(Produce_Thread, (void*)8);
    consumer1->Fork(Consume_Thread, (void*)6);
    consumer2->Fork(Consume_Thread, (void*)9);
    producer2->Fork(Produce_Thread, (void*)7);
    currentThread->Yield(); // Yield the main thread
}

//---------------------------------------------------------------------

//------------------------------Lab 3 Challenge------------------------
WriteReadLock * RWLock;

#define SENTENCE_LENGTH 9
#define MAX_CHAR 20

char sentence[SENTENCE_LENGTH][MAX_CHAR]= {"The","world" ,"is" ,"in" ,"continuous","flux" ,"and" ,"is","impermanent."  } ;
char quote[SENTENCE_LENGTH][MAX_CHAR];
int words;
void
ReaderThread(int reader_id)
{
    do {
        RWLock->ReaderAcquire();

        printf("Reader %d is reading: ", reader_id);
        for (int i = 0; i < words; i++) {
            printf("%s ", quote[i]);
        }
        printf("\n");

        RWLock->ReaderRelease();
    } while (words < SENTENCE_LENGTH);
    if(words == SENTENCE_LENGTH){
        RWLock->ReaderAcquire();
        printf("Reader %d is reading: ", reader_id);
        for (int i = 0; i < words; i++) {
            printf("%s ", quote[i]);
        }
        printf("\n");

        RWLock->ReaderRelease();
    }
}
void
WriterThread(int dummy)
{
    while (words < SENTENCE_LENGTH) {
        RWLock->WriterAcquire();
        strcpy(quote[words], sentence[words]); // composing...
        words++;
        printf("Writer is writting: ");
        for (int i = 0; i < words; i++) {
            printf("%s ", quote[i]);
        }
        printf("\n");

        RWLock->WriterRelease();
    }
}

void
Lab3WR()
{
    RWLock = new WriteReadLock("WRLock");
    words = 0;
    DEBUG('t', "Entering Lab3 Write/Read Lock");

    Thread *Writer = new Thread("W");
    Thread *Reader1 = new Thread("1");
    Thread *Reader2 = new Thread("2");
    //Thread *Reader3 = new Thread("3");

    Writer->Fork(WriterThread, (void*)0);
    Reader1->Fork(ReaderThread, (void*)1);
    Reader2->Fork(ReaderThread, (void*)2);
   // Reader3->Fork(ReaderThread, (void*)3);
    currentThread->Yield(); // Yield the main thread
}
//---------------------------------------------------------------------


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    case 2:
    ThreadMaxNumTest();
    break;
    case 3:
    TSTest();
    break;
    case 4:
    priTest();
    break;
    case 5:
    RRTest();
    break;
    
    case 6:
    Lab3ProducerConsumer();
    break;
    case 7:
    Lab3WR();
    break;
    default:
	printf("No test specified.\n");
	break;
    }
}

 