#include <stdlib.h>
#include <time.h>

#define N 100
#define DEBUG 0

int list[100];

int main()
{
    srand((unsigned int)time(NULL));

    for (int i = 0; i < N; i ++)
    {
        list[i] = rand() % 1000;
    }

    // bubble_sort
    for (int i = 0; i < N; i ++)
    {
        for (int j = 0; j < N - i - 1; j ++)
        {
            if (list[j] > list[j + 1])
            {
                int k = list[j];
                list[j] = list[j + 1];
                list[j + 1] = k;
            }
        }
    }
    
    #if DEBUG == 1
        # include <stdio.h>
        for (int i = 0; i < N; i ++)
        {
            printf ("%d ", list[i]);
        }
        printf ("\n");
    #endif

    return 0;
}