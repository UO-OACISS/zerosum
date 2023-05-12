#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#define UNUSED(expr) do { (void)(expr); } while (0)

char *buffer;
int flag=0;

int main(int argc, char *argv[])
{
    UNUSED(argc);
    UNUSED(argv);

    usleep(1500);

    raise(SIGSEGV);

    exit(EXIT_SUCCESS);
    //return EXIT_SUCCESS;
}
