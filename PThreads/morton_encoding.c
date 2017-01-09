#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include <pthread.h>
#include "utils.h"
#define DIM 3


struct thread_data{

  unsigned int *codes;
  unsigned long int *mcodes;
  int max_level;
  int chunk;
  
  };

inline unsigned long int splitBy3(unsigned int a){
    unsigned long int x = a & 0x1fffff; // we only look at the first 21 bits
    x = (x | x << 32) & 0x1f00000000ffff;  // shift left 32 bits, OR with self, and 00011111000000000000000000000000000000001111111111111111
    x = (x | x << 16) & 0x1f0000ff0000ff;  // shift left 32 bits, OR with self, and 00011111000000000000000011111111000000000000000011111111
    x = (x | x << 8) & 0x100f00f00f00f00f; // shift left 32 bits, OR with self, and 0001000000001111000000001111000000001111000000001111000000000000
    x = (x | x << 4) & 0x10c30c30c30c30c3; // shift left 32 bits, OR with self, and 0001000011000011000011000011000011000011000011000011000100000000
    x = (x | x << 2) & 0x1249249249249249;
    return x;
}

unsigned long int mortonEncode_magicbits(unsigned int x, unsigned int y, unsigned int z){
    unsigned long int answer = 0;
    answer |= splitBy3(x) | splitBy3(y) << 1 | splitBy3(z) << 2;
    return answer;
}

/* The function that transform the morton codes into hash codes */ 
void *pthread_morton_encoding(void *arg){
  unsigned long int *mcodes;
  unsigned int *codes;
  int chunk;
  int max_level;
  struct thread_data *my_data;
  my_data = (struct thread_data *) arg;

  mcodes = my_data->mcodes;
  codes = my_data->codes;
  max_level = my_data->max_level;
  chunk = my_data->chunk;
  
  int i = 0;
  for(i=0; i<chunk; i++){
    mcodes[i] = mortonEncode_magicbits(codes[i*DIM], codes[i*DIM + 1], codes[i*DIM + 2]); // Compute the morton codes from the hash codes using the magicbits mathod
  }
 
  
}


void morton_encoding(unsigned long int *mcodes, unsigned int *codes, int N, int max_level){

  pthread_t threads[NUM_THREADS];
  struct thread_data data_array[NUM_THREADS];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc;
  int i;
  void* status;
  int chunk = (int)(N/NUM_THREADS);
  for (i = 0; i<NUM_THREADS;i ++){

    data_array[i].chunk = chunk;
    data_array[i].mcodes = &mcodes[i*chunk];
    data_array[i].codes = &codes[i*chunk];
    data_array[i].max_level = max_level;
  }

  data_array[NUM_THREADS-1].chunk =  chunk + N%NUM_THREADS;
    
   for(i=0; i<NUM_THREADS; i++) {

      rc = pthread_create(&threads[i], &attr, pthread_morton_encoding,(void *)&data_array[i]); 

    }

  pthread_attr_destroy(&attr);
   for(i=0; i< NUM_THREADS; i++) {
      rc = pthread_join(threads[i], &status);
      //~ printf("Main: completed join with thread %d having a status of %ld\n",i,(long)status);
    }
  }
