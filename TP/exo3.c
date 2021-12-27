#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/*Processus 1 rempli le tableau par des valeurs aléatoires puis envoie ce tableau au processes 2.
Ce dernier affiche chaque élément puis leur somme.
j'ai séparé le priting dans une fonction qui prend comme argument le tableau et le nombre maximum d'éléments*/


void printing(double Vals[], double ValMax)
{
double sum=0;

	for(int iter =0; iter < ValMax ; iter ++)
	{
	sum+=Vals[iter];
	printf("Element %f \n",Vals[iter]);
	}
printf("Sum : %f \n", sum);


}

int main (int argc, char *argv[])
{
int rank , iter;
int nbValeurs = 2000;
double Valeurs[nbValeurs];

		MPI_Status statut;
double tempsDebut, tempsFin;

		MPI_Init(&argc, &argv);
		MPI_Comm_rank(MPI_COMM_WORLD, &rank);
tempsDebut = MPI_Wtime();//Important to be initialized here !

if(rank== 0)//proc 0
{
		for(iter=0;iter<nbValeurs; iter++)
		{
			Valeurs[iter]=rand()/(RAND_MAX+1.);
		}
		
		MPI_Send(Valeurs,nbValeurs, MPI_DOUBLE ,1,1, MPI_COMM_WORLD);
}

else//processus 1
{
MPI_Recv(Valeurs,nbValeurs,MPI_DOUBLE,0,1,MPI_COMM_WORLD,&statut);
tempsFin= MPI_Wtime();
printf("Temps de communication est : %f \n", (tempsFin-tempsDebut));
printing(Valeurs, nbValeurs);
}
MPI_Finalize();
return 0;

}
