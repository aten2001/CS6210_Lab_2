#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

//#define DEBUG_ON

#ifdef DEBUG_ON
#define DEBUG_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...) 
#endif

void centralized_barrier(int rank, int numtasks, char *inmsg, char *outmsg, char *bmsg, MPI_Status *Stat){
	int i, rc;
	if(rank == 0){
		for(i=0; i<numtasks-1; i++){
			rc = MPI_Recv(inmsg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, Stat);
			if(rc != MPI_SUCCESS){
				printf("Error starting MPI program, Terminating.\n");
				MPI_Abort(MPI_COMM_WORLD, rc);
			}
		}
		DEBUG_MSG("receive all msg\n");
		rc = MPI_Bcast(bmsg, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
	}
	else if(rank != 0){
		rc = MPI_Send(outmsg, 1, MPI_CHAR, 0, rank, MPI_COMM_WORLD);
		if(rc != MPI_SUCCESS){
			printf("Error starting MPI program, Terminating.\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}
		DEBUG_MSG("prcessor %d wait for bcast\n", rank);
		rc = MPI_Bcast(inmsg, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
		if(rc != MPI_SUCCESS){
			printf("Error starting MPI program, Terminating.\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}
		DEBUG_MSG("processor %d receive bcast\n", rank);
	}
}
int main(int argc, char *argv[]){
	int numtasks, rank, num_barriers;
	int num_steps = 2;
	char inmsg, outmsg, bmsg;
	double t1, t2, avg = 0;
	MPI_Status Stat;
	outmsg = 'd';
	bmsg = 'r';


	if (argc == 2)
		num_barriers = strtol(argv[1], NULL, 10);
	else
	{
		fprintf(stderr, "Type in: mpirun -np <num_proc> centralize_barrier_mpi <NUM_BARRIERS>\n");
    	exit(1);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int i, j;

	t1 = MPI_Wtime();
	for(i=0; i<num_barriers; i++){
		/*for(j=0; j<num_steps; j++){
			DEBUG_MSG("Processor %d runs %d steps at barrier %d-----------------------------------\n", rank, j, i);
		}*/
		centralized_barrier(rank, numtasks, &inmsg, &outmsg, &bmsg, &Stat);
	}
	t2 = MPI_Wtime();
	avg += (t2-t1)/num_barriers;
	printf("process %2d spent %.10f\n", rank, avg);
	MPI_Finalize();

}
