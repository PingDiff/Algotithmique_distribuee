///hauteur minimale
// Propagation contrôlée

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>


/*
    Messages utilisés (chacun son id):
        - 0: Traverse()
        - 1: Retour()
*/
void sendMessage(int id,int data, int dest, MPI_Comm comm)
{
   int buffer[2];
    buffer[0] = id;
    buffer[1] = data;
    MPI_Send(buffer,2, MPI_INT, dest, 0, comm);
 
}

int main(int argc, char *argv[])
{
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);
 

    // Création du graphe

    int index[5] = { 1,4,7,10,12 };
    int edges[] = { 1,0,2,3,1,3,4,1,2,4,2,3 };
    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales
    int buffer[2];
    int message;
    int maxNeighbors = 3;
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);
    int c = 1;
    int i;
    int visited = 0; // Si ce noeud a était visité ou non
    int pred = -1;   // Le prédécesseur de ce noeud
    int h = 90;
    int retours=0;
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        h = 0;
        printf("Node 0: INIT.\n");
        for (i = 0; i < neighborsCount; i++)
        {
            sendMessage(0 , h, neighbors[i], graph_comm);
            retours++;
        }
    }

    while (1)
    {
        MPI_Recv(buffer, 2, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;

        // A la reception de Traverse() par pi depuis pj

        if (buffer[0] == 0)
        {
        
          
            if ( h> buffer[1]+1)
            {
           
              h = buffer[1]+1;
               
              pred = pj;
                if (retours > 0)
                {
                    sendMessage(1 , -1,pred, graph_comm);

                    // We can safely break the while in this case
                    break;
                }
                else
                {
                    for (i = 0; i < neighborsCount; i++)
                    {
                    if (neighbors[i] == pj){
                     continue;
                     }
                        sendMessage(0, h, neighbors[i], graph_comm);
                        retours++;
                    }
                }
            }
            else
            {
                sendMessage(1,-1 ,pj, graph_comm);
            }
            
        }

        // A la reception de Retour() par pi depuis pj

        if (buffer[0] == 1)
        {

        retours--;
           if (retours == 0){
               if (pred == -1){
               printf("\n fin");
               
               }
               else {
               printf("Node %d: My predecessor is %d  la hauteur: %d\n ", pi, pred,h);
               sendMessage(1,-1 ,pred, graph_comm);
              
               }
                break;
                
            }

        }
    }

    MPI_Finalize();
    return 0;
}
