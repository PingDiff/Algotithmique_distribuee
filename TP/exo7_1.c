// Calcul du nombre de processus à l'aide d'un parcours en profendeur avec le minimum de messages

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// Fonction pour enlever des élèments d'une liste
void removeAllFromArray(int array[], int *length, int toRemove[], int toRemoveLength)
{
    int i, j, k;
    for (i = 0; i < toRemoveLength; i++)
    {
        for (j = 0; j < *length; j++)
        {
            if (array[j] == toRemove[i])
            {
                for (k = j; k < *length - 1; k++)
                {
                    array[k] = array[k + 1];
                }
                (*length)--;
            }
        }
    }
}

// Fonction pour enlever un élèment d'une liste
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
        - 0: Traverse(list)
        - 1: Retour(list)
*/
void sendMessage(int id, int listLength, int list[], int dest, MPI_Comm comm)
{
    int buffer[10], i;
    buffer[0] = id;
    buffer[1] = listLength;
    for (i = 0; i < listLength; i++)
    {
        buffer[i + 2] = list[i];
    }

    MPI_Send(buffer, 10, MPI_INT, dest, 0, comm);
}

void setListFromBuffer(int list[], int *listLength, int buffer[10])
{
    int i;
    for (i = 0; i < buffer[1]; i++)
    {
        list[i] = buffer[i + 2];
    }
    (*listLength) = buffer[1];
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Création du graphe

    int index[5] = {1, 4, 7, 10, 12};
    int edges[] = {1, 0, 2, 3, 1, 3, 4, 1, 2, 4, 2, 3};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales

    int buffer[10];
    int neighborsCount;
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    int neighbors[neighborsCount]; // Liste des voisins
    MPI_Graph_neighbors(graph_comm, pi, neighborsCount, neighbors);

    int i;
    int pred = -1; // Le prédecesseur de ce processus
    int list[8];   // Liste des processus visités
    int listLength = 0;
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        printf("Node %i: INIT.\n", pi);
        int pk = neighbors[(rand() % neighborsCount)];
        list[0] = pi;
        sendMessage(0, 1, list, pk, graph_comm);
    }

    while (1)
    {
        MPI_Recv(buffer, 10, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;

        setListFromBuffer(list, &listLength, buffer);
        removeAllFromArray(neighbors, &neighborsCount, list, listLength);

        // A la reception de Traverse(list) par pi depuis pj

        if (buffer[0] == 0)
        {
            list[listLength] = pi;
            listLength++;
            pred = pj;

            if (neighborsCount == 0)
            {
                printf("Node %i: Finished, number of nodes: %i\n", pi, listLength);
                sendMessage(1, listLength, list, pred, graph_comm);
                break;
            }
            else
            {
                int index = rand() % neighborsCount;
                sendMessage(0, listLength, list, neighbors[index], graph_comm);
            }
        }

        // A la reception de Retour(list) par pi depuis pj

        if (buffer[0] == 1)
        {
            if (neighborsCount == 0)
            {
                printf("Node %i: Finished, number of nodes: %i\n", pi, listLength);

                if (pred == -1)
                {
                    printf("Node %i: END.\n", pi);
                }
                else
                {
                    sendMessage(1, listLength, list, pred, graph_comm);
                }

                // Its safe to break here
                break;
            }
            else
            {
                int index = rand() % neighborsCount;
                sendMessage(0, listLength, list, neighbors[index], graph_comm);
            }
        }
    }

    MPI_Finalize();
    return 0;
}
