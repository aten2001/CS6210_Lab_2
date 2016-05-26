#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <omp.h>
#include <sys/time.h>
#include <time.h>

//#define DEBUG_ON

#ifdef DEBUG_ON
#define DEBUG_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...) 
#endif

typedef struct {
	bool sense;
	int count;
	int nthds;
} sense_count_t;

void init(sense_count_t *sc, int num_threads){
	sc->sense = false;
	sc->count = num_threads;
	sc->nthds = num_threads;
}

void centralize_barrier(sense_count_t *sc){
	bool curr_sense;
#pragma omp critical
	{
		assert(sc->count > 0);
		curr_sense = !(sc->sense);
		if(__sync_fetch_and_sub(&(sc->count), 1) == 1){
			sc->count = sc->nthds;
			sc->sense = curr_sense;
		}
	}
	DEBUG_MSG("waiting in barrier, curr_sense=%d\n", curr_sense);
	while(sc->sense != curr_sense);
	DEBUG_MSG("waiting finished\n");
}


int main(int argc, char *argv[]){
	int num_threads;
	int num_thrd_barriers, nthds;
	double t1, t2, average;

  	if (argc < 3){
    	fprintf(stderr, "Type in: ./centralize_barrier_mp <NUM_THREADS> <NUM_BARRIERS>\n");
    	exit(1);
  	}

  	nthds = strtol(argv[1], NULL, 10);
  	num_thrd_barriers = strtol(argv[2], NULL, 10);

	sense_count_t barrier;
	init(&barrier, nthds);
	omp_set_num_threads(nthds);
	
	printf("Thread number: %d ---------------------\n", nthds);
#pragma omp parallel
	{
		int i, j, thread_id; 
		thread_id = omp_get_thread_num();
		if(thread_id == 0){
			num_threads = omp_get_num_threads();
			assert(num_threads == nthds);
			DEBUG_MSG("It has %d threads\n", num_threads);
		}
		t1 =omp_get_wtime();
		for(i = 0; i < num_thrd_barriers; i++){
			DEBUG_MSG("Round #%d\n",i);
			/*for(j = 0; j < 1000; j++){
				DEBUG_MSG("thread %d is runing at step %d at round %d\n", thread_id, j, i);
			}*/
			centralize_barrier(&barrier);

		}
		t2 =omp_get_wtime();
		average =(t2 - t1)/num_thrd_barriers;
		printf("Thread #%d spent time: %f\n", thread_id, average);
		
	}
	
	return 0;
}
