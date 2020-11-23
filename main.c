#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h> 
#include <sys/signal.h>
#include <sys/time.h>
#include <pthread.h>
#include <time.h>


#define nLINE printf("\n")
#define debug printf("DEBUGGING");nLINE;
#define LOG(X, Y) fprintf (f, #X ": Time:%s, File:%s(%d) " #Y  "\n", __TIMESTAMP__, __FILE__, __LINE__)
#define separator printf("-----------------------------------\n")
#define MAX_THREADS 4 // max number of threads
#define uLL long int

int rowA,colA,rowB,colB;
int* A;
int* B;
int* C;

struct timeval stop, start;

typedef struct{
    int a;
    int b;
}pair;

void WriteOP(uLL secs, uLL Msecs){
    //Outputs the whole information of the run and the ouput matrix
    printf("Matrix C : rows = %d columns = %d" ,rowA,colB);nLINE;separator;
    for(int i=0;i<rowA;i++){
        for(int j=0;j<colB;j++)
            printf("%d " ,*(C + i*colB + j));
        nLINE;
    }
    separator;
    printf("Running info: \n=============\n");
    printf("Seconds taken %lu\n", secs);
    printf("Microseconds taken: %lu\n", Msecs);
}


void* getRow(void *vargp){
    int rIndx = *((int* )vargp);//extracting the passed arguments to the thread

    for(int i=0;i<colB;i++){
        *(C + rIndx*colB + i) = 0; 
        for(int j=0;j<colA;j++){
            (*(C + rIndx*colB + i)) += (*(A + rIndx*colA + j)) * (*(B + j*colB + i)); //Modifying the Row 
        }
    }
    free(vargp);//freeing the allocated memory to the passed arguments
    pthread_exit(NULL);
} 

void* getElem(void* vargp){
    pair ptr = *((pair* )vargp);//extracting the passed arguments to the thread
    
    *(C + ptr.a*colB + ptr.b) = 0; 
    for(int i=0;i<colA;i++){
        (*(C + ptr.a*colB + ptr.b)) += (*(A + ptr.a*colA + i)) * (*(B + i*colB + ptr.b));//Modifying the elements 
    }
    free(vargp);//freeing the allocated memory to the passed arguments
    pthread_exit(NULL);
}

int initiate(char** argv){
    freopen(argv[1], "r", stdin);//Opening the first file
    scanf("row = %d col = %d", &rowA , &colA);
    A = (int *)malloc(rowA * colA * sizeof(int)); //Allocating memory to the first matrix 
    
    for(int i=0;i<rowA;i++)
        for(int j=0;j<colA;j++)
            scanf("%d ",(A + i*colA + j));//Reading the first matrix

    freopen(argv[2], "r", stdin);//Opening the seconf file

    scanf("row = %d col = %d", &rowB , &colB);
    B = (int *)malloc(rowB * colB * sizeof(int)); //Allocating memory to the second matrix 

    for(int i=0;i<rowB;i++)
        for(int j=0;j<colB;j++)
            scanf("%d ",(B + i*colB + j));//Reading the first matrix

    if(colA!=rowB) return 0;

    C = (int *)malloc(rowA * colB * sizeof(int));//Allocating memory to the output matrix 
    return 1;
}

void ROWWISE(){
    gettimeofday(&start, NULL); //start checking time
    pthread_t tid[rowA]; 

    for (int i = 0; i < rowA; i++){
        int* ptr = (int*)malloc(sizeof(int)); *ptr = i;
        pthread_create(&tid[i], NULL, getRow, ptr); //Creating the threads
    }

    for (int i=0;i<rowA;i++)
        pthread_join(tid[i],NULL);//Waiting for all the threads to terminate

    gettimeofday(&stop, NULL); //end checking time
    WriteOP(stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);//Printing the final data 
    printf("Threads Created = %d \n",rowA);
}

void ELEMENTWISE(){
     gettimeofday(&start, NULL); //start checking time

    pthread_t tid[rowA*colB]; 
    int cnt = 0;
    for (int i = 0; i < rowA; i++){
        for(int j =0;j < colB; j++){
            pair* p = (pair*)malloc(sizeof(pair));
            p->a = i;
            p->b = j;
            pthread_create(&tid[cnt++], NULL, getElem, (void*)p); //Creating the threads
        }
    }

    for (int i=0;i<rowA*colB;i++)
        pthread_join(tid[i],NULL);//Waiting for all the threads to terminate

    gettimeofday(&stop, NULL); //end checking time
    WriteOP(stop.tv_sec - start.tv_sec, stop.tv_usec - start.tv_usec);//Printing the final data 
    printf("Threads Created = %d \n",rowA*colB);
}

int main(int argc,char **argv){
    //Checking the arguments
    if(argc < 4){
        printf("Invalid command\n");
        return 0;
    }
    //Checking the input matrices
    if(!initiate(argv)){
        printf("Invalid input\n");
        return 0;
    }
    //Opening the output file
    freopen(argv[3], "w+", stdout);
    printf("ROW WISE \n");nLINE;
    //Executing first approach
    ROWWISE();
    
    nLINE;nLINE;printf("===================================");nLINE;nLINE;
    printf("ELEMENT WISE \n");nLINE;
    //Executing second approach
    ELEMENTWISE();
   
    //Freeing allocated memory
    free(A);free(B);free(C);

    exit(0); 
}