// Propagation contrôlée

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

//Fonction pour ajouter un élément à un tableau
void addElementToArray(int array[], int *length, int element){
   array[*length] = element;
   (*length)++;
}
// Fonction pour supprimer un élément d'un tableau
void removeElementFromArray(int array[], int *length, int element)
{
    int i, pos = -1;
    for (i = 0; i < *length; i++)
    {
        if (array[i] == element)
        {
            pos = i;
            break;
        }
        }
    
    // If the element was found
    if (pos != -1)
    {
        for (i = pos; i < *length - 1; i++)
        {
            array[i] = array[i + 1];
        }

        (*length)--;
    }
}

/*
    Messages utilisés (chacun son id):
        - 0: Traverse()
        - 1: Retour()
        - 2: Identification()
        - 3: Terminer()
*/
void sendMessage(int id,int data, int dest, MPI_Comm comm)
{
    int buffer[2];
    buffer[0]=id;
    buffer[1]=data;
    
    MPI_Send(buffer, 2, MPI_INT, dest, 0, comm);
}

int main(int argc, char *argv[])
{   int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Création du graphe

    int index[7] = {1, 2, 3, 4, 5, 6, 7};
    int edges[] = {1, 2, 3, 4, 5, 6, 0};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales
    int buffer[2];
    int maxNeighbors = 3;
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);
    int fils[maxNeighbors];
    int filsCount = 0;
    int taille[wsize];
    int id=0;
    int i;
    int phase=1;
    int visited = 0; // Si ce noeud a était visité ou non
    int pred = -1; 
    int c=1;  // Le prédécesseur de ce noeud
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        visited = 1;
        printf("Je suis le noeud initiateur, j'initie l'anneau \n");
        for (i = 0; i < neighborsCount; i++)
        {
            sendMessage(0,pi, neighbors[i], graph_comm);
        }
    }

    while (1)
    {
        MPI_Recv(buffer, 2, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;
        // A la reception de Traverse() par pi depuis pj
        if (buffer[0] == 0)
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if (!visited)
            {
                visited = 1;
                pred = pj;
                if(pj> pi)
                {
                    buffer[1] = pj;
                    for (i = 0; i < neighborsCount; i++)
                    {
                        sendMessage(0,pj, neighbors[i], graph_comm);
                    }
                    printf("Node %d: Mypredeccesor %d, elu is %d \n", pi,pred, pj);
                }
                else
                {
                    buffer[1] = pi;
                    for (i = 0; i < neighborsCount; i++)
                    {
                        sendMessage(0, pi, neighbors[i], graph_comm);
                    }
                    printf("Node %d: Mypredeccesor %d, elu is %d \n", pi,pred, pi);
                }
            }
            else
            {
                if(pred==-1)
                {
                    pred = pj;
                    printf("node: %d: end\n",pj); 
                    printf("Je suis p0, l'elu est %d \n", buffer[1]);
                    if(pj> pi)
                    {
                        buffer[1] = pj;
                    }
                    else
                    {
                       buffer[1] = pi;
                    }
                }
            }
        }
    }

    MPI_Finalize();
    return 0;
}
