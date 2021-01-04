#include "syscall.h"


int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], ch, buffer[60];
    int i;

   // prompt[0] = '-';
   // prompt[1] = '-';
    Shell("echo  'Welcome to X4 Nachos Shell 1.0.0'");
    while( 1 )
    {
	//Write(prompt, 2, output);
    
    Shell("echo -n 'X4@Nachos$'");
	i = 0;
	
	do {
	//Shell("echo xxxxxx");
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';
    if (buffer[0] == 'q' && buffer[1] == '\0')
            Exit(0);
	if( i > 0 ) {
		//newProc = Exec(buffer); 
		//Join(newProc);
        Shell(buffer);
	}
    }
}

