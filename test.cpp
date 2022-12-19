#include <stdio.h>

bool is_prime(int n)
{
  for(int i=2; i<n; ++i)
  {
    if(n%i==0)
    {
      return false;
    }
  }

  return true;
}

int
main (void)
{
  int p=0;

  for(int i=0; i<100000; ++i)
  {
    if(is_prime(i))
    {
        p+=1;
    }
  }

  printf("%d\n", p);
}