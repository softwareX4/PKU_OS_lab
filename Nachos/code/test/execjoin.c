/* execjoin.c
 *	Simple program to test the thread syscall (Lab 6) using the
 *  previous made "exit" program.
 *  This is used to test the address space control operations
 *  This will create executable Exec child with it and the parent will Join
 *  the thread and then Exit with the result
 */

#include "syscall.h"

int main() {
    char executable[5];
    int exitCode;
    SpaceId sp;
 char data[9]; // as file name and content
    data[0] = 'e';
    data[1] = 'x';
    data[2] = 'e';
    data[3] = 'c';
    data[4] = 'j';
    data[5] = 'o';
    data[6] = 'i';
    data[7] = 'n';
    data[8] = '\0';
    executable[0] = 'e';
    executable[1] = 'x';
    executable[2] = 'i';
    executable[3] = 't';
    executable[4] = '\0';

    sp = Exec(executable);

    exitCode = Join(sp);
    Exit(exitCode);
}