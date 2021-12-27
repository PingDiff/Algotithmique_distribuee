
// CHANAFI boutaina BENNAN manal BOUHOUCH faysal



// Propagation contrôlée PL identification

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

void addElementToArray(int array[], int *length, int element)
{
    int i, j = 0;
    
    array[*length] = element;
    ++(*length) ;
}


void sendMessage(int id, int data,int dest, MPI_Comm comm)
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

    // Création du graphe

    int index[5] = {1, 4, 7, 10, 12};
    int edges[] = {1, 0, 2, 3, 1, 3, 4, 1, 2, 4, 2, 3};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales
    int cmp = 1;
    int buffer[2];
    int maxNeighbors = 3;
    int neighborsCount;
    int neighbors[maxNeighbors]; // Liste des voisins
    int filsCount;
    int fils[maxNeighbors];
    int tailles[wsize];
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    MPI_Graph_neighbors(graph_comm, pi, maxNeighbors, neighbors);

    int i;
    int visited = 0; // Si ce noeud a était visité ou non
    int pred = -1;   // Le prédécesseur de ce noeud
    int phase = 1;
    int id = 0;
    int cid;
    MPI_Status status;

    // A la récéption de INIT() par le processus 0

    if (pi == 0)
    {
        visited = 1;
        printf("Node 0: INIT.\n");
        for (i = 0; i < neighborsCount; i++)
        {
      		
            sendMessage(0,-1, neighbors[i], graph_comm);
        }
    }

    while (1)
    {
        MPI_Recv(buffer, 2, MPI_INT, MPI_ANY_SOURCE, 0, graph_comm, &status);
        int pj = status.MPI_SOURCE;

        // A la reception de Traverse() par pi depuis pj

        if (buffer[0] == 0 && phase==1)
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if (!visited)
            {
                visited = 1;
                pred = pj;
                printf("Node %d: My predecessor is %d\n", pi, pred);
                if (neighborsCount == 0)
                {
                    
                   //envoi de retour au pred avec cmp = 1
                    sendMessage(1,cmp, pred, graph_comm);

                    phase = 2; // mise en attente de la phase 2
                }
                else
                {
                    for (i = 0; i < neighborsCount; i++)
                    {
                    	//envoi de traverse aux voisins
                        sendMessage(0,-1, neighbors[i], graph_comm);
                    }
                }
            }
            else
            {
            	 //déjà visité, envoi de retour avec count = 0
                sendMessage(1,0, pj, graph_comm);
            }
        }


        // A la reception de Retour(c) par pi depuis pj

        if (buffer[0] == 1 && phase==1) 
        {
            removeElementFromArray(neighbors, &neighborsCount, pj);
            if(buffer[1] > 0){
		 cmp +=buffer[1]; 
                tailles[pj] = buffer[1]; //  taille de la sous arb du fils pj
                //printf("fils count de %d avant ajout est %d\n",pi, filsCount);
                addElementToArray(fils, &filsCount, pj);
                
                
            }

            if (neighborsCount == 0)
            {	
                if (pred == -1)
                {
                    //initier la phase 2
                    cid = id + 1;
                    for (int i = 0; i < filsCount; i++)
                    {
                         //envoi d'un msg d'identification : enclenchement de la phase 2
                        sendMessage(2,cid, fils[i], graph_comm);
                        cid += tailles[fils[i]];
                    }
                }
                else
                {
                
                    sendMessage(1,cmp, pred, graph_comm);
                }

                phase = 2;
            }
        }

        //A la reception de IDENTIFICATION(n) 

        if(buffer[0] == 2 && phase == 2)
        {
            id = buffer[1];
            printf("Node %d: ID %d from %d\n", pi, id, pj);
            if (filsCount != 0)
            {
                cid = id + 1;
                for (int i = 0; i < filsCount; i++)
                {
                     
                    sendMessage(2,cid, fils[i], graph_comm);
                    cid += tailles[fils[i]];
                }
            }
            else
            {
               
                sendMessage(3,-1, pred, graph_comm); 
                break;
            }
            
        }


        //A la reception de TERMINER() de Pi depuis Pj
        if (buffer[0] == 3 && phase == 2)
        {
        printf("iam %d  recieved %d from %d\n", pi, buffer[0], pj);
            removeElementFromArray(fils, &filsCount, pj);
            if(filsCount == 0){
                if (pi == 0)
                {
                    printf("Node %d: END \n", pi);
                }
                else{
                    
                    sendMessage(3,-1, pred, graph_comm); //envoyer terminer au predecesseur
                }
                break;
            }
            
        }
        
    }

    MPI_Finalize();
    return 0;
}
