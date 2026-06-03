#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#define MAX 1000


int main()
{
    int n;
    int array[MAX] = {0};
    scanf("%d", &n);
    if (n > MAX - 1 || n < 1)
        return 0;
    int limit = (int)ceil(sqrt(n));
    int i = 2;
    while (i <= limit)
    {
        if (array[i] == 0)
        {
            printf("%d ", i);
            for (int j = i * i; j <= n; j += i)
            {
                array[j] = 1;
            }
        }
        i ++;
    }
    for (; i <= n; i ++)
    {
        if (array[i] == 0)
        {
            printf("%d ", i);
        }
    }
    printf("\n");
    return 0;
}