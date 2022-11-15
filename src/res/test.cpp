#include <stdio.h>

extern "C"
{
    void puti(int n){
        printf("%d\n", n);
    }
}