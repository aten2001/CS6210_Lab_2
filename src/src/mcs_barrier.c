#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>
#include <string.h>


//#define DEBUG_ON

#ifdef DEBUG_ON
#define DEBUG_MSG(...) fprintf (stderr, __VA_ARGS__)
#else
#define DEBUG_MSG(...) 
#endif


typedef struct {
	int num_children;
	int parent_id;
    int children[2];
} node_stat_t;

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

	// arrival: received messages from children
	if(node_stat->num_children != 0){
		for(i=0; i<(node_stat->num_children); i++){
			rc = MPI_Recv(inmsg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, Stat);
			if(rc != MPI_SUCCESS){
				printf("Error starting MPI program, Terminating.\n");
				MPI_Abort(MPI_COMM_WORLD, rc);
			}
		}

	}
	// arrival: signal to parent
	if(node_stat->parent_id != -1){
		rc = MPI_Send(outmsg, 1, MPI_CHAR, node_stat->parent_id, rank, MPI_COMM_WORLD);
		if(rc != MPI_SUCCESS){
			printf("Error starting MPI program, Terminating.\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}

		// wakeup: received message from parent
		rc = MPI_Recv(inmsg, 1, MPI_CHAR, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, Stat);
		if(rc != MPI_SUCCESS){
			printf("Error starting MPI program, Terminating.\n");
			MPI_Abort(MPI_COMM_WORLD, rc);
		}

		// wakeup: signal to children
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

	// wakeup: root node signals its children
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
	int numtasks, rank, num_barriers, num_array;
	int num_steps = 2;
	char inmsg, outmsg, bmsg;
	MPI_Status Stat;
	outmsg = 'd';
	bmsg = 'r';
	
	num_array = 4;
	
	double t1, t2, avg = 0;

	char name[MPI_MAX_PROCESSOR_NAME];
	int resultlen;


	if (argc == 2)
		num_barriers = strtol(argv[1], NULL, 10);
	else
	{
		fprintf(stderr, "Type in: mpirun -np <num_proc> mcs_barrier <NUM_BARRIERS>\n");
    	exit(1);
	}

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int i, j;
	node_stat_t node_stat;
	create_node_stat(rank, numtasks, num_array, &node_stat);

	MPI_Get_processor_name(name, &resultlen);
	printf("Rank: %d, Processor: %s, len: %d\n", rank, name, resultlen);	

	t1 = MPI_Wtime();
	for(i=0; i<num_barriers; i++){
		/*for(j=0; j<num_steps; j++){
			printf("Processor %d runs %d steps at barrier %d-----------------------------------\n", rank, j, i);
		}*/
		mcs_barrier(rank, numtasks, num_array, &inmsg, &outmsg, &bmsg, &node_stat, &Stat);
	}
	t2 = MPI_Wtime();
	avg += (t2 - t1)/num_barriers;
	printf("process %2d spent %.10f\n", rank, avg);
	MPI_Finalize();
}
