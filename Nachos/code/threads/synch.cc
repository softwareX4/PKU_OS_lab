// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
 
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!
Lock::Lock(char* debugName) {

    name = debugName;
    semaphore = new Semaphore("Lock",1);   // "1" means Free
}
Lock::~Lock() {}

bool Lock:: isLocked(){
    return this->holderThread != NULL;
}
bool Lock:: isHeldByCurrentThread(){
    return this->holderThread == currentThread;
}

//	Acquire -- wait until the lock is FREE, then set it to BUSY
void Lock::Acquire() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    DEBUG('s', "Lock \"%s\" Acquired by Thread \"%s\"\n", name, currentThread->getName());
    semaphore->P();
    holderThread = currentThread;    
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//	Release -- set lock to be FREE, waking up a thread waiting
//		in Acquire if necessary
//(Note: Only the Thread which own this lock can release lock)
void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts 
    DEBUG('s', "Lock \"%s\" Released by Thread \"%s\"\n", name, currentThread->getName());
    // make sure the owner of this lock is currentThread
    ASSERT(this->isHeldByCurrentThread());
    holderThread = NULL;
    semaphore->V();
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

Condition::Condition(char* debugName) {
    waitQueue = new List();
 }
Condition::~Condition() {
    delete waitQueue;
 }

//	Wait() -- release the lock, relinquish the CPU until signaled, 
//		then re-acquire the lock
void Condition::Wait(Lock* conditionLock) {    
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    // conditionLock must be held by the currentThread
    ASSERT(conditionLock->isHeldByCurrentThread())
    // conditionLock must be locked
    ASSERT(conditionLock->isLocked());

    // 1. Release the lock while it waits
    conditionLock->Release();

    // 2. Append into waitQueue and sleep
    waitQueue->Append(currentThread);
    currentThread->Sleep();

    // Awake by Signal...
    // 3. Reclaim lock while awake
    conditionLock->Acquire();
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
 }

 
 // In Nachos, condition variables are assumed to obey *Mesa*-style
// semantics.  When a Signal or Broadcast wakes up another thread,
// it simply puts the thread on the ready list, and it is the responsibility
// of the woken thread to re-acquire the lock (this re-acquire is
// taken care of within Wait()).

//	Signal() -- wake up a thread, if there are any waiting on 
//		the condition
void Condition::Signal(Lock* conditionLock) { 
    
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
     // conditionLock must be held by the current Thread
    ASSERT(conditionLock->isHeldByCurrentThread())

    if (!waitQueue->IsEmpty()) {
        // Putting thread from the front of waitQueue onto ready list
        Thread* thread = (Thread*) waitQueue->Remove();
        scheduler->ReadyToRun(thread);
    }

    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}



//	Broadcast() -- wake up all threads waiting on the condition
void Condition::Broadcast(Lock* conditionLock) {     
    
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    // conditionLock must be held by the current Thread
    ASSERT(conditionLock->isHeldByCurrentThread())

    DEBUG('b', "Condition \"%s\" Broadcasting: ", name);
    while (!waitQueue->IsEmpty()) {
        // Putting all the threads on ready list
        Thread* thread = (Thread*) waitQueue->Remove();
        DEBUG('b', "Thread \"%s\", ", thread->getName());
        scheduler->ReadyToRun(thread);
    }
    DEBUG('b', "\n");

    
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}




//-------------------------Lab 3 --------------------------------
//Producer–consumer problem using SEMAPHORE
#ifdef USE_SEMAPHORE
    Buffer::Buffer(){
        count = 0;
        buffer_mutex = new Semaphore("buffer_mutex",1);
        empty = new Semaphore("empty",BUFFER_SIZE);
        full = new Semaphore("full",0);
    }
    Buffer::~Buffer(){
        delete buffer_list;
    }

    void 
    Buffer::putItemInBuffer(Product  p){
        empty->P();
        buffer_mutex->P();
        
       // printf("p:%d\n",(p).value);
        buffer_list[count++] = p;
        
        buffer_mutex->V();
        full->V();
    }

    Product * 
    Buffer::removeItemFromBuffer(){
        full->P();
        buffer_mutex->P();
        Product * p = &buffer_list[count-- -1];
        
        buffer_mutex->V();
        empty->V();
        return p;
    }
#else
    Buffer::Buffer(){
        count = 0;
        empty = new Condition("empty_condition");
        full = new Condition("full_condition");
        lock = new Lock("buffer_lock");

    }
    Buffer::~Buffer(){
        delete buffer_list;
    }

    void 
    Buffer::putItemInBuffer(Product  p){
        lock->Acquire();
        while (count == BUFFER_SIZE)
        {
            empty->Wait(lock);
        }
        
        buffer_list[count++] = p;
        full->Signal(lock);
        lock->Release();
    }

    Product * 
    Buffer::removeItemFromBuffer(){
        lock->Acquire();
        while(count == 0){
            full->Wait(lock);
        }
        Product * p = &buffer_list[count-- -1];        
        empty->Signal(lock);
        lock->Release();
        return p;
    }
#endif



    void Buffer::printBuffer(){
        printf("Buffer: [", BUFFER_SIZE, count);
            int i;
            for (i = 0; i < count; i++) {
                printf("%d, ", buffer_list[i].value);
            }
            for (; i < BUFFER_SIZE; i++) {
                printf("__, ");
            }
            printf("]\n");
    }


#ifdef USE_W_R

  WriteReadLock::WriteReadLock(char * debugName){
      name = debugName;
      readers = 0;
      mutex = new Lock("reader_mutex");
      lock = new Semaphore("buffer lock",1);
  }
  WriteReadLock::~WriteReadLock(){ }


  void
  WriteReadLock::ReaderAcquire(){
      IntStatus oldLevel = interrupt->SetLevel(IntOff);
      mutex->Acquire();
      readers++;
      
    DEBUG('w', "Reader \"%s\" comming in  \t(blockingReader=%d)\n", currentThread->getName(), readers);
      if(readers == 1){   //the first reader
          lock->P();
      }
      mutex->Release();

      (void) interrupt->SetLevel(oldLevel);
  }

  void
  WriteReadLock::ReaderRelease(){      
      IntStatus oldLevel = interrupt->SetLevel(IntOff);	
      mutex->Acquire();
      readers--;
      
    DEBUG('w', "Reader \"%s\" getting out \t(blockingReader=%d)\n", currentThread->getName(), readers);
      if(readers == 0) {
          lock->V();
      }
      mutex->Release();
      (void) interrupt->SetLevel(oldLevel);
  }

  void
  WriteReadLock::WriterAcquire(){      
      IntStatus oldLevel = interrupt->SetLevel(IntOff);	
    DEBUG('w', "Writer \"%s\" comming in  \t(blockingReader=%d)\n", currentThread->getName(), readers);
      lock->P();
      (void) interrupt->SetLevel(oldLevel);
  }

  void
  WriteReadLock::WriterRelease(){
      IntStatus oldLevel = interrupt->SetLevel(IntOff);	      
    DEBUG('w', "Writer \"%s\" getting out \t(blockingReader=%d)\n", currentThread->getName(), readers);
    lock->V();
      (void) interrupt->SetLevel(oldLevel);
  }

#endif
