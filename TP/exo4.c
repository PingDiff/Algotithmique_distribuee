#include<mpi.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char** argv) {
MPI_Init(&argc, &argv);

int world_rank;
MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
int world_size;
MPI_Comm_size(MPI_COMM_WORLD, &world_size);

int token=11;
// Receive from the lower process and send to the higher process.Take care
// of the special case when you are the forsy process tp prevent deadlock.

while(token>0)
{
        if(world_rank != 0)
        {

		MPI_Recv(&token, 1 , MPI_INT, world_rank - 1, 0 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("PROCESS %d received token %d from process %d\n" , world_rank, token, world_rank - 1);
	} else
	{
	//set the token's value if you are process 0 
		token --;

	}

 	MPI_Send(&token, 1, MPI_INT, (world_rank + 1) % world_size, 0, MPI_COMM_WORLD);

	//Now the process 0 can receive form the last process. This makes sure that at
	//lesat one MPI_send is initialized before all MPI_Recvs (again, to prevent 
	// deadlock)
	if (world_rank == 0)
	{
	 MPI_Recv(&token, 1 , MPI_INT, world_size - 1 , 0 , MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("Process %d received token %d from process %d\n", world_rank, token, world_size - 1);
	}
}
  MPI_Finalize();
}
