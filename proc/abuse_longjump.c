/*
 * Write a program to see what happens if we try to longjmp() into a function that has already returned.
 *
 *:~/proc$ ./abuse_longjump 
 *	Before calling f2()
 *	Before setjmp in f2()
 *	After initial setjmp in f2()
 *	After calling f2()
 *	Before calling longjmp in main()
 *	setjmp after longjmp()
 *	Segmentation fault (core dumped)
 * 
*/
#include <setjmp.h>
#include "tlpi_hdr.h"

static jmp_buf env;

static void
f2(void)
{
    printf("Before setjmp in f2()\n");
    switch(setjmp(env)){
	    case 0:
		    printf("After initial setjmp in f2()\n");
		    break;
	    case 1:
		    printf("setjmp after longjmp()\n");
		    break;
    }
}

static void
f1(void)
{
    printf("Before calling f2()\n");
    f2();
    printf("After calling f2()\n");
    
}

int
main(int argc, char *argv[])
{
    f1();
    printf("Before calling longjmp in main()\n");
    longjmp(env, 1);
    exit(EXIT_SUCCESS);
}

