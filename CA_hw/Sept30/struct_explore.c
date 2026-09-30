# include <stdio.h>
# include <float.h>
# include <stdlib.h>

struct struct1      // shortest
{                   // offset(byte)
    char a;         // 0
    short b;        // 2
    int c;          // 4
    long d;         // 8
    float e;        // 16
    double f;       // 24
    long double g;  // 32   <- x86_64 (Linux/GCC) 下 10 字节，但是视作 16 字节做 16 字节对齐
};                  // total: 48

struct struct2
{                   // offset(byte)
    double f;       // 0
    char a;         // 8
    long double g;  // 16
    int c;          // 32
    long d;         // 40
    float e;        // 44
    short b;        // 48
};                  // total: 64

struct struct3
{                   // offset(byte)
    char a;         // 0
    long double g;  // 16
    short b;        // 32
    double f;       // 40
    int c;          // 48
    long d;         // 56
    float e;        // 64
};                  // total: 80

int main()
{
    printf("x87 long double size(byte) & actual used size(bit)\n");
    printf("long double:    %lu\n", sizeof(long double));
    printf("mantissa:       %d\n", LDBL_MANT_DIG);
    printf("exponent:       %d\n", __builtin_clz(1) - __builtin_clz(__LDBL_MAX_EXP__) + 1);
    printf("list sizeof structs(byte)\n");    
    printf("struct1:        %lu\n", sizeof(struct struct1));
    printf("struct2:        %lu\n", sizeof(struct struct2));
    printf("struct3:        %lu\n", sizeof(struct struct3));    
}