#include "stdio.h"
#include "stdlib.h"
#include "sys/time.h"
#include "utils.h"
#define DIM 3

int main(int argc, char** argv){

  FILE *fp;
  FILE *fpt;

  fp = fopen("pthreads_data.txt","a");
  fpt = fopen("pthreads_time.txt","a");

  if (fp == NULL) {
  fprintf(stderr, "Can't open input file pthreads_data.txt\n");
  exit(1);
  }
  

  // Time counting variables 
  struct timeval startwtime, endwtime;

  if (argc != 7) { // Check if the command line arguments are correct 
    printf("Usage: %s N dist pop rep P\n where\n N:number of points\n dist: distribution code (0-cube, 1-Plummer)\n pop: population threshold\n rep: repetitions\n L: maximum tree height.\n NUM: Number of Threads \n", argv[0]);
    return (1);
  }

  // Input command line arguments
  int N = atoi(argv[1]); // Number of points
  int dist = atoi(argv[2]); // Distribution identifier 
  int population_threshold = atoi(argv[3]); // population threshold
  int repeat = atoi(argv[4]); // number of independent runs
  int maxlev = atoi(argv[5]); // maximum tree height
  NUM_THREADS = atoi(argv[6]); //Number of Threads

  
  fprintf(fp,"\n %d , %d ,%d , %d , %d ",N,dist,population_threshold ,maxlev ,NUM_THREADS);
  printf("Running for %d particles with maximum height: %d\n", N, maxlev);

   float *X = (float *) malloc(N*DIM*sizeof(float)); 
/* Generate a 3-dimensional data distribution */
  gettimeofday (&startwtime, NULL); 
  
  create_dataset(X, N, dist);
  
  gettimeofday (&endwtime, NULL);
  
  double create_data_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
      /1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
  
  printf("Time to create the data: %f\n", create_data_time);



  
 int it = 0; 
  // Independent runs
 for(it = 0; it<repeat; it++){

  float *Y = (float *) malloc(N*DIM*sizeof(float));

  unsigned int *hash_codes = (unsigned int *) malloc(DIM*N*sizeof(unsigned int));
  unsigned long int *morton_codes = (unsigned long int *) malloc(N*sizeof(unsigned long int));
  unsigned long int *sorted_morton_codes = (unsigned long int *) malloc(N*sizeof(unsigned long int));
  unsigned int *permutation_vector = (unsigned int *) malloc(N*sizeof(unsigned int)); 
  unsigned int *index = (unsigned int *) malloc(N*sizeof(unsigned int));
  unsigned int *level_record = (unsigned int *) calloc(N,sizeof(unsigned int)); // record of the leaf of the tree and their level

  // initialize the index
  int i = 0;
  for(i=0; i<N; i++){
    index[i] = i;
  }

  
  //~ fprintf(fpt,"\n %f", create_data_time);
  /* Find the boundaries of the space */
  float max[DIM], min[DIM];
  find_max(max, X, N);
  find_min(min, X, N);

  int nbins = (1 << maxlev); // maximum number of boxes at the leaf level

 
 

    gettimeofday (&startwtime, NULL); 
  
    compute_hash_codes(hash_codes, X, N, nbins, min, max); // compute the hash codes

    gettimeofday (&endwtime, NULL);

    double hash_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    
    printf("Time to compute the hash codes: %f\n", hash_time);
    fprintf(fpt,"\n  %f", hash_time);
    
    gettimeofday (&startwtime, NULL); 

    morton_encoding(morton_codes, hash_codes, N, maxlev); // computes the Morton codes of the particles

    gettimeofday (&endwtime, NULL);


    double morton_encoding_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);

    printf("Time to compute the morton encoding: %f\n", morton_encoding_time);
    fprintf(fpt,",%f", morton_encoding_time);
    gettimeofday (&startwtime, NULL); 

    // Truncated msd radix sort
    int offset[8] = {0};
    int BinSizes2[8]={0};
    BinSizes2[0]=N;
    pthread_truncated_radix_sort(morton_codes, sorted_morton_codes, 
			 permutation_vector, 
			 index, level_record, N, 
			 population_threshold, 3*(maxlev-1), 0,1,offset ,BinSizes2);

    gettimeofday (&endwtime, NULL);


    double sort_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    printf("Time for the truncated radix sort: %f\n", sort_time);
    fprintf(fpt,",%f",sort_time);
    gettimeofday (&startwtime, NULL); 

    // Data rearrangement
    data_rearrangement(Y, X, permutation_vector, N);

    gettimeofday (&endwtime, NULL);


    double rearrange_time = (double)((endwtime.tv_usec - startwtime.tv_usec)
				/1.0e6 + endwtime.tv_sec - startwtime.tv_sec);
    

    printf("Time to rearrange the particles in memory: %f\n", rearrange_time);
    fprintf(fpt,",%f", rearrange_time);
    /* The following code is for verification */ 
    // Check if every point is assigned to one leaf of the tree
    int pass = check_index(permutation_vector, N); 

    if(pass){
      printf("Index test PASS\n");
      fprintf(fpt,",1");
    }
    else{
      printf("Index test FAIL\n");
      fprintf(fpt,",0");
    }

    // Check is all particles that are in the same box have the same encoding. 
    pass = check_codes(Y, sorted_morton_codes, 
		       level_record, N, maxlev);

    if(pass){
      printf("Encoding test PASS\n");
      fprintf(fp,",1");
    }
    else{
      printf("Encoding test FAIL\n");
      fprintf(fp,",0");
    }

  


  

  /* clear memory */
  
  free(Y);
  free(hash_codes);
  free(morton_codes);
  free(sorted_morton_codes);
  free(permutation_vector);
  free(index);
  free(level_record);
  }
  free(X);
  fclose(fpt);
  fclose(fp);
}





