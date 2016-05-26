#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "omp.h" 
#include <stdlib.h>
#include <assert.h>

//#define DEBUG_ON

#ifdef DEBUG_ON
#define DEBUG_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...) 
#endif


#define MAX_ROUNDS 500 
typedef struct {
	bool myflags[2][MAX_ROUNDS];
	bool *partnerflags[2][MAX_ROUNDS];
} flags_t;

void dissemination_barrier(flags_t *localflags, int rounds, bool *sense, int *parity){
	int i;
	for(i=0; i<rounds; i++){
#pragma omp critical
		{
			*(localflags->partnerflags[*parity][i]) = *sense;
		}
		while(localflags->myflags[*parity][i] != *sense);
	}
	/*if(*parity == 1) *sense = !*sense;
	*parity = 1 - *parity;
	*sense = !*sense;*/
	if(*parity == 1) *sense ^= 1;
	*parity ^= 1;
	//*sense ^= 1;

}

int main(int argc, char *argv[]){
	int i, steps, rounds, num_threads;
	int num_thrd_barriers, nthds;
	double t1, t2, avg;
	
  	if (argc < 3){
    	fprintf(stderr, "Type in: ./dissemination_barrier <NUM_THREADS> <NUM_BARRIERS>\n");
    	exit(1);
  	}

  	nthds = strtol(argv[1], NULL, 10);
  	num_thrd_barriers = strtol(argv[2], NULL, 10);

	steps = 10;
	flags_t allnodes[nthds];
	memset(allnodes, 0, nthds * sizeof(flags_t));
	rounds = ceil(log2((double)nthds));
	omp_set_num_threads(nthds);
	printf("Thread number: %d ---------------------\n", nthds);
	
#pragma omp parallel
	{
		int parity;
		int i, j, m;
		bool sense;
		int thread_id;
		flags_t *localflags;
		parity = 0;
		sense = true;
		thread_id = omp_get_thread_num();
		if(thread_id == 0)
		{
			num_threads = omp_get_num_threads();
			assert(num_threads == nthds);
		}
		for(m=0; m<rounds; m++){
			allnodes[thread_id].partnerflags[0][m] = &allnodes[(thread_id + (1<<m))%nthds].myflags[0][m];
			allnodes[thread_id].partnerflags[1][m] = &allnodes[(thread_id + (1<<m))%nthds].myflags[1][m];
		}
		localflags = &allnodes[thread_id];
		printf("sense: %d\n", (int)sense );
		t1 = omp_get_wtime();
		for(j=0; j<num_thrd_barriers; j++){
			/*for(i=0; i<steps; i++){
				printf("thread %d, step %d, barrier #%d--------------------------\n", thread_id, i, j);
			}*/
			dissemination_barrier(localflags, rounds, &sense, &parity);
		}
		t2 = omp_get_wtime();
		avg =(t2 - t1)/num_thrd_barriers;

		printf("Thread #%d spent time: %f\n", thread_id, avg);
	}
	
	return 0;
}
