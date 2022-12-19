#include <stdio.h>

extern "C"
{
    void puti(int n){
        printf("%d\n", n);
    }

    void putd(double n){
        printf("%lf\n", n);
    }
}