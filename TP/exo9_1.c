
// Propagation contrôlée
//election dans un graphe

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
// Fonction pour ajouter un element a un tableau

void addElementToArray(int array[], int *length, int element){
array[*length]=element;
(*length)++;
}

/*
    Messages utilisés (chacun son id):
        - 0: Traverse()
        - 1: Retour()
        - 2: information()
        - 3: Terminer()
       
*/
void sendMessage(int id,int data , int dest, MPI_Comm comm)
{
    int buffer[2];
    buffer[0]=id;
    buffer[1]=data;
    MPI_Send(buffer, 2, MPI_INT, dest, 0, comm);
}

int main(int argc, char *argv[])
{
    int wsize, pi;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    
   int index[8] = {3, 7, 10 , 14 , 15 , 18 , 19 , 20};
    int edges[] = {1, 2, 7, 0, 2 , 3 , 5 , 0, 1 , 3 , 1 , 2 , 5 , 4 , 3 , 1 , 3 , 6 , 5 , 0};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales
   

    int message;
    int buffer [2];
    int maxNeighbors = 4;
    int neighborsCount;
    int c=1;
    int neighbors[maxNeighbors]; // Liste des voisins
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    int visited=0; // Si ce noeud a était visité ou non
    int pred = -1;   // Le prédécesseur de ce noeud
    MPI_Status status;
   
    //variable créer pour election
   
    int elu = pi;
    int phase= 1;
    int fils [3];
    int filsCount;

    // A la récéption de INIT() par le processus 0

    if (pi == 0) // initiateur
    {
        visited = 1;
        printf("Node 0: INIT.\n");
        for (i = 0; i < neighborsCount; i++)
        {
            sendMessage(0, -1 , neighbors[i], graph_comm);
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
                printf("Node %d: My predecessor is %d \n", pi, pred);
                if (neighborsCount == 0)
                {
                    sendMessage(1, elu , pred, graph_comm);

                    // We can safely break the while in this case
                   
                }
                else
                {
                    for (i = 0; i < neighborsCount; i++)
                    {
                        sendMessage(0,-1, neighbors[i], graph_comm);
                    }
                }
            }
            else
            {
                sendMessage(1,-1, pj, graph_comm);
            }
        }

        // A la reception de Retour() par pi depuis pj

        if (buffer[0] == 1)
        {
       
            removeElementFromArray(neighbors, &neighborsCount, pj);
if (buffer[1]>= elu){
elu = buffer[1];
addElementToArray(fils,&filsCount,pj);
}
            if (neighborsCount == 0)
            {
                if (pi == 0)
                {
                    printf("elu %d: \n", elu);
                   for (int i = 0; i < filsCount; i++){
                    sendMessage(2, elu , fils[i] , graph_comm);
                   }
                }
                else
                {
                    sendMessage(1,elu, pred, graph_comm);
                }

                // We can safely break the while in both cases
               
            }
        }  //Reception de Elu(elu)
        if(buffer[0]==2){
    elu = buffer[1];
    if(filsCount==0){
    sendMessage(3,-1,pred , graph_comm);
    break;
    }
   
    else{
    for(int i =0 ; i<filsCount ; i++){
    sendMessage(2,elu,fils[i],graph_comm);
    //message Elu(elu)
    }
    }
    //reception de terminer
   
    if(buffer[0]==3){
    removeElementFromArray(fils,&filsCount,pj);
    if(filsCount == 0){
    if(pi==0){
    printf("Tous les noeuds sont informées");
    }
    else{
    sendMessage(3,-1,pred,graph_comm);
   
    }
    }
   
    }
   
   
    break;
    }  
}
    MPI_Finalize();
    return 0;
}

