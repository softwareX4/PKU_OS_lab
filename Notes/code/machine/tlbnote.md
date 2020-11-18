**First:**
Use <code>nachos -x USRPROG_PATH</code> to stast nachos  

Then  <code>thread/main.cc : main()</code>  will call <code>thread/system.cc : Initialize()</code> , which will do this thing:
```c
machine = new Machine(debugUserProg);
```
In this process:
```c
//.....
 mainMemory = new char[MemorySize];
  //MemorySize = NumPhysPages * PageSize :32*128
 //......
//if use TLB :
 tlb = new TranslationEntry[TLBSize];
 //TLBSize = 4
    for (i = 0; i < TLBSize; i++)
	tlb[i].valid = FALSE;
```

**Then:**
 main() calls <code>StartProcess()</code> in <code>userprog/progtest.cc</code>

 
```c
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;
    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable);    
    currentThread->space = space;

    delete executable;	

    space->InitRegisters();		
    space->RestoreState();	

    machine->Run();	
```

1. Open code file
2. Allocate address space
3. Set registers
4. Run machine

We focus on the Step 2. and Step 4.

**Step 2.**
Call <code> AddrSpace(OpenFile *executable)</code>  in <code>userprog/translate.cc</code> 

First read user code , calculate address space
```c
size = code.size + initData.size + uninitData.size + UserStackSize

numPages = (size / PageSize) + ( size % PageSize > 0 ? 1 : 0)

size = numPages * PageSize;
```
Then set up page table (virtual page # = phys page #)

Last copy in the code and data segments into memory

**Step 4.**
Call <code>Run()</code> in <code> machine/mipssim.cc</code>,which called <code>OneInstruction(Instruction *instr) </code>  

```c
for(;;){
    
    OneInstruction(instr);
    //......
}
```

<code>OneInstruction(Instruction *instr)</code> in <code>machine/mipssim.cc</code>  

which called <code>ReadMem</code> in <code>machine/translate.cc</code>
```c
bool Machine::ReadMem(int addr, int size, int *value){
    //......
    exception = Translate(addr, &physicalAddress, size, FALSE);
    if (exception != NoException) {
	machine->RaiseException(exception, addr);
	return FALSE;
    }
    //......
}

```

which called <code>Translate</code> in <code> machine/translate.cc</code>
If exception occur , call <code>RaiseException</code> in <code>machine/machine.cc</code>  -> <code>ExceptionHandler</code> in <code>userprg/exception.cc</code> to handle.




##  Translate()
<code>/machine/translate.cc</code>
ExceptionType Machine::Translate(int virtAddr, int* physAddr, int size, bool writing)
- in => VA
- do => set TranslationEntry use/dirty  && store PA  

if error return exception type  

- if use Page Table
```c
    //......
    //calculate VPN , offset
    //vpn = (unsigned) virtAddr / PageSize;
    //offset = (unsigned) virtAddr % PageSize;
    //......
 if (tlb == NULL) {		// => page table => vpn is index into table
	if (vpn >= pageTableSize) {
        //too big	    
	    return AddressErrorException;
	} else if (!pageTable[vpn].valid) {
	    //not in table
	    return PageFaultException;
	}
	entry = &pageTable[vpn];    

}
```

- if use TLB
```c
    for (entry = NULL, i = 0; i < TLBSize; i++)
    	if (tlb[i].valid && (tlb[i].virtualPage == vpn)) {
		entry = &tlb[i];			// FOUND!
		break;
	    }
	if (entry == NULL) {				// not found
    	
    	return PageFaultException;		// really, this is a TLB fault,
						// the page may be in memory,
						// but not in the TLB
```

