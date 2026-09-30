#include <stdio.h>
#include <stdlib.h>

int main()
{
    printf ("----------------\n");
    printf ("type       size \n");
    printf ("----------------\n");
    printf ("char       %5lu\n", sizeof(char));
    printf ("short      %5lu\n", sizeof(short));
    printf ("int        %5lu\n", sizeof(int));
    printf ("long       %5lu\n", sizeof(long));
    printf ("long long  %5lu\n", sizeof(long long));
    printf ("float      %5lu\n", sizeof(float));
    printf ("double     %5lu\n", sizeof(double));
    printf ("long double%5lu\n", sizeof(long double));
    printf ("size_t     %5lu\n", sizeof(size_t));
    printf ("void*      %5lu\n", sizeof(void*));
    printf ("int*       %5lu\n", sizeof(int*));
    printf ("char*      %5lu\n", sizeof(char*));
    printf ("----------------\n");
    
    return 0;
}