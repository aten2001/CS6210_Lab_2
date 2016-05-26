#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "mpi.h"
#include "omp.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
	int num_children;
	int parent_id;
        int children[2];
} node_stat_t;

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

centralize_barrier(sense_count_t *sc){
	bool curr_sense;
#pragma omp critical
	{
		assert(sc->count > 0);
		curr_sense = !sc->sense;
		if(__sync_fetch_and_sub(&(sc->count), 1) == 1){
			sc->count = sc->nthds;
			sc->sense = curr_sense;
		}
	}
	while(sc->sense != curr_sense);
}

void create_node_stat(int rank, int numtasks, int num_array, node_stat_t *node_stat){
	int i, last_process_id, level = 0;
	last_process_id = numtasks - 1;
	memset(node_stat, 0, sizeof(node_stat_t));

	if(rank == 0)
		node_stat->parent_id = -1;
	else
		node_stat->parent_id = (rank - 1)/num_array;

	if((last_process_id - 1)/num_array > rank)
		node_stat->num_children = num_array;
	else if((last_process_id - 1)/num_array == rank)
		node_stat->num_children = (last_process_id%num_array == 0) ? num_array:(last_process_id%num_array);
	else if((last_process_id - 1)/num_array < rank)
		node_stat->num_children = 0;

	while(1){
		if(rank == 0){
			level = 0;
			break;
		}
		if(((1<<level) - 1 <= rank) && (rank < ((1<<(level+1)) - 1)))
			break;
		else
			level ++;
	}

	if((last_process_id - 1)/2 > rank){
		node_stat->children[0] = rank * 2 + 1;
		node_stat->children[1] = rank * 2 + 2;
	}
	else if((last_process_id - 1)/2 == rank){
		for(i=0; i<((last_process_id%2 == 0) ? 2:1); i++)
			node_stat->children[i] = rank * 2 + (i + 1);
	}
	else if((last_process_id - 1)/2 < rank){
		node_stat->children[0] = 0;
		node_stat->children[1] = 0;
	}
}

void mcs_barrier(int rank, int numtasks, int num_array, char *inmsg, char *outmsg, char *bmsg, node_stat_t *node_stat, MPI_Status *Stat){
	int i, j, rc;
	if(node_stat->num_children != 0){
		for(i=0; i<(node_stat->num_children); i++){
			rc = MPI_Recv(inmsg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, Stat);
			if(rc != MPI_SUCCESS){
				printf("Error starting MPI program, Terminating.\n");
				MPI_Abort(MPI_COMM_WORLD, rc);
			}
		}
	}
	if(node_stat->parent_id != -1){
		rc = MPI_Send(outmsg, 1, MPI_CHAR, node_stat->parent_id, rank, MPI_COMM_WORLD);
		if(rc != MPI_SUCCESS){
			printf("Error starting MPI program, Terminating.\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}
		rc = MPI_Recv(inmsg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, Stat);
		if(rc != MPI_SUCCESS){
			printf("Error starting MPI program, Terminating.\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}
		for(j=0; j<2; j++){
			if(node_stat->children[j] != 0){
				rc = MPI_Send(&bmsg, 1, MPI_CHAR, node_stat->children[j], rank, MPI_COMM_WORLD);
				if(rc != MPI_SUCCESS){
					printf("Error starting MPI program, Terminating.\n");
					MPI_Abort(MPI_COMM_WORLD, rc);
				}
			}
		}
	}
	if(node_stat->parent_id == -1){
		for(j=0; j<2; j++){
			if(node_stat->children[j] != 0){
				rc = MPI_Send(&bmsg, 1, MPI_CHAR, node_stat->children[j], rank, MPI_COMM_WORLD);
				if(rc != MPI_SUCCESS){
					printf("Error starting MPI program, Terminating.\n");
					MPI_Abort(MPI_COMM_WORLD, rc);
				}
			}
		}

	}
}

void main(int argc, char *argv[]){
	int nthds, num_proc_barriers;
	int numtasks, rank, num_thread_barriers, num_steps, num_array;
	int num_threads, j;
	sense_count_t barrier;
	char inmsg, outmsg, bmsg;
	double t1, t2, ave_time;
	
	ave_time = 0;
	MPI_Status Stat;
	node_stat_t node_stat;
	outmsg = 'd';
	bmsg = 'r';

	num_steps = 1;
	num_array = 4;

	if (argc == 3){
		nthds = strtol(argv[1], NULL, 10);
		num_proc_barriers = strtol(argv[2], NULL, 10);
	}
	else
	{
		fprintf(stderr, "Type in: mpirun -np <num_proc> mcs_cen_combine <NUM_THREADS> <NUM_BARRIERS>\n");
    	exit(1);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	init(&barrier, nthds);
	create_node_stat(rank, numtasks, num_array, &node_stat);
	omp_set_num_threads(nthds);
	t1 = MPI_Wtime();
	for(j=0; j<num_proc_barriers; j++){
#pragma omp parallel
		{
			centralize_barrier(&barrier);
		}
		mcs_barrier(rank, numtasks, num_array, &inmsg, &outmsg, &bmsg, &node_stat, &Stat);
	}
	t2 = MPI_Wtime();
	ave_time += (t2 - t1)/num_proc_barriers;
	printf("process %d spent %.10f\n", rank, ave_time);
	MPI_Finalize();
}
