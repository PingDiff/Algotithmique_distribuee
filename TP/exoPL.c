//parcours en largeur 
// Propagation contrôlée

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

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
*/
//envoi msg au distinataire
void sendMessage(int id, int dest, MPI_Comm comm)
{
	//envoie msg with buffer
    int buffer[] = {id};
    MPI_Send(buffer, 1, MPI_INT, dest, 0, comm);
}

int main(int argc, char *argv[])
{
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Création du graphe

    int index[5] = {1, 4, 7, 10, 12};
    int edges[] = {1, 0, 2, 3, 1, 3, 4, 1, 2, 4, 2, 3};

    //int index[8] = { 3, 7, 10, 14, 15, 18, 19, 20 };
    //int edges[] = { 1,2,7 ,0,2,3,5, 0,1,3, 1,2,4,5, 3, 1,3,6, 5, 0 };
    
    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales

    int message;
    int maxNeighbors = 3;//graphe 2 n=4
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    int visited = 0; // Si ce noeud a était visité ou non --> init tous les noeuds ont 0
    int pred = -1;   // Le prédécesseur de ce noeud
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        visited = 1;
        printf("Node 0: INIT.\n");
        //envoi le msg a tous les voisins immediat
        for (i = 0; i < neighborsCount; i++)
        {
            sendMessage(0, neighbors[i], graph_comm);
        }
    }

    while (1)
    {
        MPI_Recv(&message, 1, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;

        // A la reception de Traverse() par pi depuis pj

        if (message == 0) //travers
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if (!visited)
            {
                visited = 1;
                pred = pj;
                printf("Node %d: My predecessor is %d\n", pi, pred); //Vi' = Vi'-Pj
                if (neighborsCount == 0)
                {
                    sendMessage(1, pred, graph_comm);

                    // We can safely break the while in this case --> lors de l'envoi de retour break
                    break;
                }
                else
                {
                    for (i = 0; i < neighborsCount; i++)
                    {
                        sendMessage(0, neighbors[i], graph_comm);
                    }
                }
            }
            else
            {
                sendMessage(1, pj, graph_comm); //envoi le retour a Pj
            }
        }

        // A la reception de Retour() par pi depuis pj

        if (message == 1)
        {
            removeElementFromArray(neighbors, &neighborsCount, pj); //enlever dans Vi

            if (neighborsCount == 0)
            {
                if (pred == -1)//racine
                {
                    printf("Node %d: END.\n", pi);
                }
                else
                {
                    sendMessage(1, pred, graph_comm);
                }

                // We can safely break the while in both cases
                break;
            }
        }
    }

    MPI_Finalize();
    return 0;
}
