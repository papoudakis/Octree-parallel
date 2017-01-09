#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include <pthread.h>
#include "utils.h"
#define DIM 3


struct thread_data{

  unsigned int *codes;
  float *X;
  float *low;
  float step;
  int chunk;
  
  };

unsigned int compute_code(float x, float low, float step){

  return floor((x - low) / step);

}


/* Function that does the quantization */
void* quantize(void* arg){

  unsigned int *codes;
  float *X;
  float *low;
  float step;
  int chunk;
  struct thread_data *my_data;
  my_data = (struct thread_data *) arg;

  codes = my_data->codes;
  X = my_data->X;
  low = my_data->low;
  step = my_data->step;
  chunk = my_data->chunk;
  

  int i = 0, j = 0; 
  for(i=0; i<chunk; i++){
    for(j=0; j<DIM; j++){
      codes[i*DIM + j] = compute_code(X[i*DIM + j], low[j], step); 
    }
  }

}

float max_range(float *x){

  int i=0;
  float max = -FLT_MAX;
  for(i=0; i<DIM; i++){
    if(max<x[i]){
      max = x[i];
    }
  }

  return max;

}

void compute_hash_codes(unsigned int *codes, float *X, int N, 
			int nbins, float *min, 
			float *max){


  pthread_t threads[NUM_THREADS];
  struct thread_data data_array[NUM_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc;
  int i;
  int chunk = (int)(N/NUM_THREADS);


 
  float range[DIM];
  float qstep;

  for(i=0; i<DIM; i++){
    range[i] = fabs(max[i] - min[i]); // The range of the data
    range[i] += 0.01*range[i]; // Add somthing small to avoid having points exactly at the boundaries 
  }

  qstep = max_range(range) / nbins; // The quantization step 
 

  for (i = 0; i<NUM_THREADS;i ++){

    data_array[i].chunk = chunk;
    data_array[i].X = &X[i*chunk*3];
    data_array[i].codes = &codes[i*chunk];
    data_array[i].low = min;
    data_array[i].step = qstep;
  }

  data_array[NUM_THREADS-1].chunk = N%NUM_THREADS +chunk;
  //~ data_array[NUM_THREADS-1].X =   &X[(NUM_THREADS-1)*chunk*3];
  //~ data_array[NUM_THREADS-1].codes = &codes[i*chunk];
  
    
   for(i=0; i<NUM_THREADS; i++) {

      rc = pthread_create(&threads[i], &attr, quantize,(void *)&data_array[i]); 

    }

  pthread_attr_destroy(&attr);
   for(i=0; i< NUM_THREADS; i++) {
      rc = pthread_join(threads[i], NULL);
    }

}



