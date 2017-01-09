#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <pthread.h>
#include "utils.h"
#define DIM 3

struct thread_data{

  float *X;
  float *Y;
  unsigned int* permutation_vector;
  int chunk;
  
  };

void* parallel_data_rearrangement(void* arg){

  
  float *X;
  float *Y;
  unsigned int *permutation_vector;
  int chunk;
  
  struct thread_data *my_data;
  my_data = (struct thread_data *) arg;
  int i=0;

  Y = my_data->Y;
  X = my_data->X;
  permutation_vector = my_data->permutation_vector;
  chunk = my_data->chunk;
  int n;
  n = sizeof(X)/sizeof(X[0]);
  for(i=0; i<chunk; i++){
    memcpy(&Y[i*DIM], &X[permutation_vector[i]*DIM], DIM*sizeof(float));
  }

}

void* data_rearrangement(float *Y, float *X, unsigned int *permutation_vector, int N)
{
        
  pthread_t threads[NUM_THREADS];
  struct thread_data data_array[NUM_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int i;
  int chunk = (int)(N/NUM_THREADS);
  int n;
  double a;

  
  for (i = 0; i<NUM_THREADS;i ++){

    data_array[i].chunk = chunk;
    data_array[i].X = X;
    data_array[i].Y = &Y[i*chunk*DIM];
    data_array[i].permutation_vector = &permutation_vector[i*chunk];
  }

  data_array[NUM_THREADS-1].chunk = N%NUM_THREADS +chunk;
  


  int rc;
 
  for(i=0; i<NUM_THREADS; i++) {
  
      rc = pthread_create(&threads[i], &attr, parallel_data_rearrangement,(void *)&data_array[i]);
       
  
    }
 
   pthread_attr_destroy(&attr);
   for(i=0; i< NUM_THREADS; i++) {
      rc = pthread_join(threads[i], NULL);
    }
  
  }
  
