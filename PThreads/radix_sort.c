#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <pthread.h>

#define MAXBINS  8
// our data
struct thread_data {

  unsigned long int *morton_codes;
  unsigned long int *sorted_morton_codes;
  unsigned int *permutation_vector;
  unsigned int *index;
  int* level_record;
  int N;
  int population_threshold;
  int sft;
  int lv;
  };
void serial_truncated_radix_sort(unsigned long int *morton_codes, 
			  unsigned long int *sorted_morton_codes, 
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  int *level_record,
			  int N, 
			  int population_threshold,
			  int sft, int lv);
void pthread_truncated_radix_sort(unsigned long int *morton_codes, 
			  unsigned long int *sorted_morton_codes, 
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  int *level_record,
			  int N, 
			  int population_threshold,
			  int sft, int lv, int NUM, int *offset, int * BinSizes2);


void swap_long(unsigned long int **x, unsigned long int **y){

  unsigned long int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

void swap(unsigned int **x, unsigned int **y){

  unsigned int *tmp;
  tmp = x[0];
  x[0] = y[0];
  y[0] = tmp;

}

void* truncated_radix_sort( void* arg)
{
  
  int BinSizes[MAXBINS] = {0};
  int BinSizes2[MAXBINS] = {0};
  int offset[MAXBINS] = {0};
  unsigned int *tmp_ptr;
  unsigned long int *tmp_code;
  unsigned long int *morton_codes; 
  unsigned long int *sorted_morton_codes; 
  unsigned int *permutation_vector;
  unsigned int *index;
  int *level_record;
  int N; 
  int population_threshold;
  int sft;
  int lv;
  
  struct thread_data *my_data;
  my_data = (struct thread_data *) arg;
  morton_codes = my_data->morton_codes ;
  sorted_morton_codes = my_data->sorted_morton_codes ;
  permutation_vector = my_data->permutation_vector ;
  index = my_data->index;
  level_record = my_data->level_record;
  N =my_data->N;
  population_threshold = my_data->population_threshold;
  sft = my_data->sft;
  lv = my_data->lv;

  if (N>0)
  {
  
  level_record[0] = lv; // record the level of the node
}
  if(N<=population_threshold || sft < 0) {
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes
  }
  else{

    // Find which child each point belongs to 
    int j = 0;
    for(j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      BinSizes[ii]++;
    }

// scan prefix (must change this code)  
    int off = 0, i = 0;
    for(i=0; i<MAXBINS; i++){
      BinSizes2[i] = BinSizes[i];
      BinSizes[i] = off;
      off += BinSizes2[i];
    }
    
    for(j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      permutation_vector[BinSizes[ii]] = index[j];
      sorted_morton_codes[BinSizes[ii]] = morton_codes[j];
      BinSizes[ii]++;
    }
    for (j=0 ;j<MAXBINS;j++)
    {
      offset[j] = BinSizes[j] -BinSizes2[j];
    }

    //swap the index pointers  
    swap(&index, &permutation_vector);

    //swap the code pointers 
    swap_long(&morton_codes, &sorted_morton_codes);

    /* Call the function recursively to split the lower levels */
    if (lv>3)
    {
    serial_truncated_radix_sort(morton_codes, sorted_morton_codes, 
    permutation_vector, 
    index, level_record,N, 
    population_threshold, sft, lv);
  }
  else{
    pthread_truncated_radix_sort(morton_codes, sorted_morton_codes, 
    permutation_vector, 
    index, level_record,N, 
    population_threshold, sft-3, lv+1,8,offset,BinSizes2);
}
    

    }
  } 
void pthread_truncated_radix_sort(unsigned long int *morton_codes, 
			  unsigned long int *sorted_morton_codes, 
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  int *level_record,
			  int N, 
			  int population_threshold,
			  int sft, int lv, int NUM, int *offset,int *BinSizes2){

  
  pthread_t threads[NUM];
  struct thread_data data_array[NUM];
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
  int rc;
  int i;
 

  for (i = 0; i<NUM;i ++){

    data_array[i].morton_codes = &morton_codes[offset[i]];
    data_array[i].sorted_morton_codes = &sorted_morton_codes[offset[i]];
    data_array[i].permutation_vector = &permutation_vector[offset[i]];
    data_array[i].index = &index[offset[i]];
    data_array[i].level_record = &level_record[offset[i]];
    data_array[i].N = BinSizes2[i];
    data_array[i].population_threshold = population_threshold;
    data_array[i].sft = sft;
    data_array[i].lv = lv;
  }

   
   for(i=0; i<NUM; i++) {

      rc = pthread_create(&threads[i], &attr, truncated_radix_sort,(void *)&data_array[i]);
    }


  pthread_attr_destroy(&attr);
  void * status;
   for(i=0; i< NUM; i++) {
     
      rc = pthread_join(threads[i], &status);
      
    }



}


void serial_truncated_radix_sort(unsigned long int *morton_codes, 
			  unsigned long int *sorted_morton_codes, 
			  unsigned int *permutation_vector,
			  unsigned int *index,
			  int *level_record,
			  int N, 
			  int population_threshold,
			  int sft, int lv){

  int BinSizes[MAXBINS] = {0};
if (N>0)
  {
  
  level_record[0] = lv; // record the level of the node
}
  if(N<=population_threshold || sft < 0) { // Base case. The node is a leaf
    memcpy(permutation_vector, index, N*sizeof(unsigned int)); // Copy the pernutation vector
    memcpy(sorted_morton_codes, morton_codes, N*sizeof(unsigned long int)); // Copy the Morton codes 

  }
  else{

    // Find which child each point belongs to 
    int j = 0;
    for(j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      BinSizes[ii]++;
    }


    // scan prefix (must change this code)  
    int offset = 0, i = 0;
    for(i=0; i<MAXBINS; i++){
      int ss = BinSizes[i];
      BinSizes[i] = offset;
      offset += ss;
    }
    
    for(j=0; j<N; j++){
      unsigned int ii = (morton_codes[j]>>sft) & 0x07;
      permutation_vector[BinSizes[ii]] = index[j];
      sorted_morton_codes[BinSizes[ii]] = morton_codes[j];
      BinSizes[ii]++;
    }
    
    //swap the index pointers  
    swap(&index, &permutation_vector);

    //swap the code pointers 
    swap_long(&morton_codes, &sorted_morton_codes);

    /* Call the function recursively to split the lower levels */
    offset = 0; 
    for(i=0; i<MAXBINS; i++){
      
      int size = BinSizes[i] - offset;
      
      serial_truncated_radix_sort(&morton_codes[offset], 
			   &sorted_morton_codes[offset], 
			   &permutation_vector[offset], 
			   &index[offset], &level_record[offset], 
			   size, 
			   population_threshold,
			   sft-3, lv+1);
      offset += size;  
    }
    
      
  } 
}

        
