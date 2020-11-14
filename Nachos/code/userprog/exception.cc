// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"


#ifdef USE_TLB
//Lab2 TLB Miss Handle
void TLBAlgoFIFO(TranslationEntry page);
void TLBMissHandler(int BadVAddr);
void TLBAlgoLRU(TranslationEntry page);
#endif
void IncrementPCRegs(void);
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{

    int type = machine->ReadRegister(2);
        //Lab2 TLB exception handle
     if (which == PageFaultException) {
         // linear page table page fault
            DEBUG('m', "=> Page table page fault.\n");
            int BadVAddr = machine->ReadRegister(BadVAddrReg); 
            printf("BadAddr : %d\n",BadVAddr);
            ASSERT(FALSE);
	//ASSERT(FALSE);
    }
    #ifdef USE_TLB    
    else if(which == TLBException)
    { // TLB miss (no TLB entry)
            DEBUG('m', "=> TLB miss (no TLB entry)\n");
            int BadVAddr = machine->ReadRegister(BadVAddrReg); // The failing virtual address on an exception
            TLBMissHandler(BadVAddr);
    }
    
    #endif 

    else if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");

 #ifdef USE_TLB
    double rate = (machine->TLBMiss * 100.0) / (machine->TLBMiss + machine->TLBHit);
    printf("TLB Miss Rate : %.2lf \% \n",rate);
    #endif 
   	interrupt->Halt();
    } 
    
    else if((which == SyscallException) && (type == SC_Exit)){
        printf("program (uid=%d, tid=%d, pri=%d, name=%s) exit \n",currentThread->getUserId(), currentThread->getThreadId(), currentThread->getPriority(),currentThread->getName());
       

        
       // IncrementPCRegs();
        currentThread->Finish();
      //  int NextPC = machine->ReadRegister(NextPCReg);
      //  machine->WriteRegister(PCReg,NextPC);
    }
    
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
 
	ASSERT(FALSE);
    }
}


void IncrementPCRegs(void) {
    // Debug usage
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));

    // Advance program counter
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+4);
}

#ifdef USE_TLB

void TLBMissHandler(int virtAddr){
    printf("TLB Miss ! BadVAddr : %d \n",virtAddr);
    
    unsigned int vpn;
    vpn = (unsigned) virtAddr / PageSize;

    // ONLY USE FOR TESTING Lab4 Exercise2
    TLBAlgoFIFO(machine->pageTable[vpn]);
}



void TLBAlgoFIFO(TranslationEntry page)
{
    int TLBreplaceIdx = -1;
    // Find the empty entry
    for (int i = 0; i < TLBSize; i++) {
        if (machine->tlb[i].valid == FALSE) {
            TLBreplaceIdx = i;
            break;
        }
    }
    // If full then move everything forward and remove the last one
    if (TLBreplaceIdx == -1) {
        TLBreplaceIdx = TLBSize - 1;
        for (int i = 0; i < TLBSize - 1; i++) {
            machine->tlb[i] = machine->tlb[i+1];
        }
    }
    // Update TLB
    machine->tlb[TLBreplaceIdx] = page;
}

void TLBAlgoLRU(TranslationEntry page){
    
    int TLBreplaceIdx = -1;
    // Find the empty entry
    for (int i = 0; i < TLBSize; i++) {
        if (machine->tlb[i].valid == FALSE) {
            TLBreplaceIdx = i;
            break;
        }
    }
    // If full then move everything forward and remove the last one
    if (TLBreplaceIdx == -1) {
        TLBreplaceIdx = TLBSize - 1;
        for (int i = 0; i < TLBSize - 1; i++) {
            if(machine->tlb[i].count  > machine->tlb[TLBreplaceIdx].count) {
                TLBreplaceIdx = i;
            }
        }
    }
    // Update TLB
    machine->tlb[TLBreplaceIdx] = page;

}

#endif