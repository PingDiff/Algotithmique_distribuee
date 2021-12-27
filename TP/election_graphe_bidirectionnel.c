// Election de processus dans un anneau bidirectionel


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>       

#define TRUE 1
#define FALSE 0



/*
    Messages utilisés (chacun son id):
        - 0: Candidature(id,lg,lgmax)
        - 1: Retour(dest,rep)
        - 2: Terminer(vainqueur)

*/
void sendMessage(int id, int pid,int lg, int lgmax, int dest, MPI_Comm comm)
{
    int buffer[4];
    buffer[0] = id;
    buffer[1] = pid;
    buffer[2] = lg;
    buffer[3] = lgmax;


    MPI_Send(buffer, 4, MPI_INT, dest, 1, comm);
}



int main(int argc, char *argv[])
{
    srand(time(NULL));
    int wsize, pi,deja=0;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &wsize);

    // Création du graphe

    int index[8] = {2, 4, 6, 8,10 ,12,14,16};
    int edges[] = {4,7,3,6,5,7,1,5,0,6,2,3,1,4,0,2};

    MPI_Comm graph_comm;
    MPI_Graph_create(MPI_COMM_WORLD, wsize, index, edges, 1, &graph_comm);
    MPI_Comm_rank(graph_comm, &pi);

    // Variables locales

    int buffer[4];
    int neighborsCount;
    MPI_Graph_neighbors_count(graph_comm, pi, &neighborsCount);
    int neighbors[neighborsCount]; // Liste des voisins
    MPI_Graph_neighbors(graph_comm, pi, neighborsCount, neighbors);

    int i;
    int pred = -1; // Le prédecesseur de ce processus

    int lgmax = 0,nbrep = 0,rep;
    
    int repOk = TRUE;

    MPI_Status status;

    // A la récéption de INIT() par tous les processus 
    enum etats {NONCONCERNE,CANDIDAT,BATTU,ELU}; 

    enum etats etat = CANDIDAT;
    lgmax = 1;
    int vainqueur =-1;
    
 

    nbrep = 0;
    repOk = TRUE;
    printf(" %d canditature  lgmax %d \n",pi,lgmax);

    for(int c=0; c<neighborsCount;c++){
        
        //printf(" %d send canditature to %d lgmax %d \n",pi,neighbors[c],lgmax);
        sendMessage(0,pi,0,lgmax,neighbors[c],graph_comm);
    }

      
    while (1)
    {
 
        MPI_Recv(buffer, 4, MPI_INT, MPI_ANY_SOURCE, 1, graph_comm, &status);
        int pj = status.MPI_SOURCE;
        //destination pour faire suivre un message
        int dest = neighbors[0]+neighbors[1]-pj;


        // A la reception de Candidature() par pi depuis pj
        if (buffer[0] == 0)
        {
           // printf("reception de candidateur");
            if(buffer[1] <pi){

                sendMessage(1,FALSE,buffer[1],-1,pj,graph_comm);
                if(etat == NONCONCERNE){
                    etat = CANDIDAT;

                    nbrep = 0;
                    repOk = TRUE;
                    for(i=0;i<2;i++){
                        
                        //printf(" %d send canditature to %d lgmax  %d \n",pi,neighbors[i],lgmax);
 
                        sendMessage(0,pi,0,lgmax,neighbors[i],graph_comm);
                    }

                }

               

            }
            else if (buffer[1] > pi){
                etat = BATTU;
                //printf("battu %d\n",pi);

                if(buffer[2]<buffer[3]){
                    sendMessage(0,buffer[1],buffer[2]+1,buffer[3],dest,graph_comm);

                }
                else{

                    sendMessage(1,TRUE,buffer[1],-1,pj,graph_comm);

                }
            }
            else
            {
                if(etat != ELU ){
                    etat = ELU;
                    vainqueur = pi;
                    //printf("vainqueur %d\n",pi);
                    sendMessage(2,vainqueur,-1,-1 ,dest, graph_comm);
                    break;
                }

            }
            
           
        }
        

    

          
        

        // A la reception de Reponse() par pi depuis pj

        if (buffer[0] == 1)
        {
            if(buffer[2] == pi){
                nbrep++;
                repOk = repOk && buffer[1];
                if(nbrep == 2){
                    if(repOk == FALSE){
                        etat = BATTU;
                    }
                    lgmax *= 2;
                    if(etat == CANDIDAT)
                    {
                        printf(" %d canditature  lgmax %d \n",pi,lgmax);

                        nbrep = 0;
                        repOk = TRUE;
                        for(i=0;i<2;i++){
                            //printf(" %d send canditature to %d  lgmax: %d\n",pi,neighbors[i],lgmax);


                            sendMessage(0,pi,0,lgmax,neighbors[i],graph_comm);
                        }
                    }

                }
                
                
            }
            else
                {

                    sendMessage(1,buffer[1],buffer[2],-1,dest,graph_comm);

                }

           
           
        }
        
        // A la reception de Terminer par pi de pj
        if (buffer[0] == 2)
        {
            printf("vainqueur %d\n",buffer[1]);
            vainqueur = buffer[1];
            if(vainqueur != pi){
                sendMessage(2,vainqueur,-1,-1 ,dest, graph_comm);
                etat = NONCONCERNE;

            }
         break;

        }
    }

    MPI_Finalize();
    return 0;
}