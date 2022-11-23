// C program for insertion sort
#include <math.h>
#include <stdio.h>
  
/* Function to sort an array 
   using insertion sort*/
void insertionSort(int arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++) 
    {
        key = arr[i];
        j = i - 1;
  
        /* Move elements of arr[0..i-1], 
           that are greater than key, 
           to one position ahead of 
           their current position */
        while (j >= 0 && arr[j] > key) 
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}
  
// A utility function to print 
// an array of size n
void printArray(int arr[], int n)
{
    int i;
    for (i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}
  
// Driver code
int main()
{
    int arr[] = {61,86,53,19,61};
    int n = sizeof(arr) / sizeof(arr[0]);
  
    for(int i=0; i<1000000; ++i)
	{
		insertionSort(arr, n);
        arr[0] = 61;
        arr[1] = 86;
        arr[2] = 53;
        arr[3] = 19;
        arr[4] = 61;
	}
  
    return 0;
}
