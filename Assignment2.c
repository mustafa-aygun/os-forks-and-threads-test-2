/* Mustafa Aygün 2315125 */
/*Libraries added*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

/* They are our mutex and condition variables. I prefer using different ones for some cases.
* It is helping me to keep following them easily.
* I will explain each of them when they appear in code.*/
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_cond_t condition;
pthread_cond_t condition1;

int **matrix; /* It is the global double int array which will be our matrix.*/
int m; /* It is the global matrix size varaible. I define m globally because normally I should take it
* from user to create a file. I should read it from directly the input file. Therefore, I am keeping as global
* reach easily at everywhere.*/
int numThread = 0; /* It is a global integer variable to keep tracking number of thread.*/

/*It is our thread struct. Here, I am keeping different variables in it to access them easily
* in every thread. t_num is our threads number. 
* t_work is how many column and row that thread will shift. For example, we have 5x5 matrix with
* 3 thread. Then thread1 will have 1, thread2 will have 2 and thread3 will have 2. I will also
* explain how we are calculating it in code with details.
* t_prev_work is another important variable to keep following in which position that thread
* start to shift. Because the work may split differently according matrix size and thread number
* formalizing it may be complicated. Therefore, I am keeping from which row and colum it will
* start to shift. I will show its calculation in code too. 
* s and d have the same name with variables in assignment paper.*/
struct myThread{
  pthread_t t_id;
  int t_num;
  int t_work;
  int t_prev_work;
  int s;
  int d;
};

/* Basic function prototypes.*/
void createInputTxt(int); /* Creating input file with a random matrix with size m.*/
int** readInputTxt(); /* Reading input file.*/
void printMatrix(); /* It is  basic function to print matrix.*/
void shiftRight(int); /* Shifting matrix right.*/
void shiftUp(int); /* Shifting matrix up.*/
void *runner(void*); /* It is runner function for threads.*/
void splitWork(struct myThread**,int); /* It is another helper function to calculate works.*/


int main(int argc, char const *argv[]){
  /* Defining variables.*/
  int d,s,i,tempM; /* tempM is for creating file myself.*/
  pthread_attr_t attr; 
  /* Our srand to create random variables.*/
  srand(time(0));
  /* Here I am getting m from user and calling createInputTxt function to create a random matrix.*/
  printf("Enter the m: ");
  scanf("%d",&tempM);
  createInputTxt(tempM);
  matrix = readInputTxt(); /* After creating random matrix with size m, I am reading it to my global variable.*/
  printf("Enter the s: ");
  scanf("%d",&s);
  printf("Enter the d: ");
  scanf("%d",&d);
  while(d>m){ /* Basic operation to be sure thread number is lower than matrix size m.*/
    printf("Enter the d again with lower size than m: ");
    scanf("%d",&d);
  }
  /*Initilalizing attr, condition variables and mutexs.*/
  pthread_attr_init(&attr);
  pthread_cond_init(&condition,NULL);
  pthread_cond_init(&condition1,NULL);

  pthread_mutex_init(&lock,NULL);
  pthread_mutex_init(&lock2,NULL);
  pthread_mutex_init(&lock3,NULL);

  struct myThread **threads = malloc(sizeof(struct myThread*)*d); /* Malloc threads with size d.*/
  for(i = 0; i < d; i++){
    threads[i] = malloc(sizeof(struct myThread)); /* Malloc each node of thread array.*/
    threads[i]->t_num = i+1; /* I am starting thread number from 1.*/
    threads[i]->s = s; /* Our s and d which are matrix size, number of shift and number of thread.*/
    threads[i]->d = d;
  }
  
  splitWork(threads,d); /* Calling splitWork to divide works between threads.*/
  printf("Matrix before shift\n---------------------------------\n");
  printMatrix(m); /* Printing matrix before shifting.*/
  for(i = 0; i < d; i++){ /* Creating thread with sending its node as parameter.*/
    pthread_create(&threads[i]->t_id,&attr,runner,(void*)threads[i]);
  }
  for(i = 0; i < d; i++){ /* Joining threads.*/
     pthread_join(threads[i]->t_id,NULL);
  }
 
  printf("Matrix after shift\n----------------------------------\n");
  printMatrix(m); /* Printing matrix after shifting.*/
  
  /* Destroying mutexs and conditional variables.*/
  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock2);
  pthread_mutex_destroy(&lock3);

  pthread_cond_destroy(&condition);
  pthread_cond_destroy(&condition1);

  return 0;
}

void createInputTxt(int tempM){
  
  FILE *myFile; /* Creating a file.*/
  char filename[20]; /* A constant character array to arranging file name.*/
  strcpy(filename,"input.txt");
  myFile = fopen(filename, "w"); /* Opening file, if there is no file it creates it.*/
  fprintf(myFile, "%d\n", tempM); /* Putting m to first line of the file.*/
  int i, j;
  for (i = 0; i < tempM; i++){
    for(j = 0; j < tempM; j++){
      fprintf(myFile, "%d ", rand() % 10); /* Writing m number of random integer to one line*/
    }
    fprintf(myFile, "\n"); /* Ending line.*/
  }
  fclose(myFile); /* Closing the file.*/
}

int** readInputTxt(){
  FILE *myFile;
  char filename[20]; /* A constant character array to arranging file name.*/
  strcpy(filename,"input.txt");
  myFile = fopen(filename, "r"); /* Opening file to read.*/
  int localM,i,j;
  if (fscanf(myFile, "%d\n", &localM) != EOF); /* I am firstly reading our matrix size to a local variable.*/
  int **localMatrix; /* Creating a localMatrix.*/
  localMatrix = malloc(sizeof(int*)*localM);
  for (i = 0; i < localM; i++){ /* Malloc that matrix according to our size.*/
    localMatrix[i] = malloc(sizeof(int)*localM);
  }
  for (i = 0; i < localM; i++){
    for(j = 0; j < localM; j++){ /* Reading our input file line by line into our matrix.*/
      if (fscanf(myFile, "%d ", &localMatrix[i][j]) != EOF); 
    }
    if (fscanf(myFile, "\n") != EOF); 
  }
  fclose(myFile); /* Closing the file.*/
  m = localM; /* Giving localM to our global variable which I will use in all other parts.*/
  return localMatrix; /* returning local matrix to our global matrix.*/
}

void printMatrix(){

  int i,j; /* It is a basic printing function to print matrix.*/
  for (i = 0; i < m; i++){ /* It takes size from our global variable which we will read from file.*/
    for(j = 0; j < m; j++){
      printf("%d ",matrix[i][j]);
    }
    printf("\n");
  }
}


void splitWork(struct myThread **threads,int d){/* It takes thread array and number of threads.*/

  int i, work,tempD = d,totalWork=0,localM = m; /* Giving our global m to a local one because we will change it.*/
  
  for(i = 0; i < tempD; i++){
    work = localM/d; /* Calculating work for current m and d.*/
    threads[i]->t_work = work; /* Giving that work to thread.*/
    threads[i]->t_prev_work = totalWork; /*totalWork is the position which previous matrix stop.*/
    totalWork += work; /* Adding current work to total one.*/
    localM -= work; /* Substracting line number from our matrix size to calculate new matrix.*/
    d -=1 ; /* Decreasing d to calculating with new matrix size and new thread number.*/
  }
  /* For example, we have m=5 and d=3. 
  *When i=0; t_work=work=1 which means thread1 will shift 1 column and row, 
  *          t_prev_work=totalWork=0 which means thread1 will start shifting column and row from 0. 
  *          totalWork will become 1. new matrix size, localM=4 and new thread number d=2;
  *When i=1; t_work=work=2 which means thread2 will shift 2 column and row. 
  *          t_prev_work=totalWork=1 which means thread2 will start shifting column and row from 1. 
  *          totalWork will become 3. new matrix size, localM=2 and new thread number d=1;
  *When i=2; t_work=work=2 which means thread3 will shift 2 column and row. 
  *          t_prev_work=totalWork=3 which means thread3 will start shifting column and row from 3. 
  *          totalWork will become 5. new matrix size, localM=0 and new thread number d=0.*/
}

void shiftRight(int row){/* It takes row which means which row will be shifted.*/

  int temp, i;
  temp = matrix[row][m-1]; /* It takes last position of row in a temp variable.*/
  for(i = 0; i < m; i++){
    matrix[row][m-1-i] = matrix[row][m-2-i]; /* It shifts each element in the row by one.*/
  }
  matrix[row][0] = temp; /* We are putting our last value to initial position.*/
}

void shiftUp(int column){ /* It takes column which means which column will be shifted.*/

  int temp, i;
  temp = matrix[0][column]; /* It takes first position of column in a temp variable.*/
  for(i = m; 1<i; i--){
    matrix[m-i][column] = matrix[m+1-i][column]; /* It shifts each element in the column to up by one.*/
  }
  matrix[m-1][column] = temp; /* We are putting our first value to last position.*/
}

void* runner(void *p){

  int i,j;
  struct myThread *t = (struct myThread*) p; /* We are doing our thread pointers typecasting.*/
  for(j = 0; j < t->s; j++){ /* It will run according to number of shift.*/
    if(j != 0){ /* If j=0 it means we are just starting so we can skip sync of threads coming from up shifting.
    * and can directly start to shifting right. If it is not 0, we have to check sync between up and right shift.*/
    /* Here, I following the the logic that where all threads waiting on a conditional variables till the last one.
    * They checking if they are the last one or not. If they aren't last one, they are increasing count.
    * If it is the last one, it is making numThread=0 and signal to conditional variable.*/  
    pthread_mutex_lock(&lock); /* Here, I am locking it because I will check global variable which is
    numThread. I will always use "lock" to locking numThread.*/
      if(numThread != t->d-1){
        numThread++; /* If it isn't the last one increasing count.*/
          //printf("4-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
        pthread_mutex_unlock(&lock); /* Releasing the lock.*/
        pthread_mutex_lock(&lock2); /* Here, they are waiting on the conditional variable.*/
        pthread_cond_wait(&condition,&lock2); /* For this conditinal variable, I will use "lock2" and "condition".*/
        pthread_mutex_unlock(&lock2);
      }
      else{ /* If it is the last one coming here.*/
        numThread = 0; /* Making numThread 0 again.*/
        pthread_mutex_unlock(&lock);} /* Releasing lock for others.*/
          //printf("3-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
        pthread_cond_signal(&condition); /* Signaling to condional varaible.*/
      
    }
    for(i = 0; i < t->t_work; i++){ /* All threads starting shift right at the same time.*/
        shiftRight((t->t_prev_work+i)); /* Because of each of them reaching another part of the global matrix.*/
    } /* We don't need too lock our matrix and use threads efficiency.*/

    /* After shifting right is finished, our threads starting to grouping for shifting up. 
    * Logic is totally same. Until the last one they are entering the if condition, increasing count and wait there.
    * When last one comes, it goes to else, making numThread=0 again and signalling others.*/
    pthread_mutex_lock(&lock); /* Here, again I used "lock" to locking numThread variable.*/
    if(numThread != t->d-1){
      numThread++; /* If it isn't the last one increasing count.*/
      //printf("2-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
      pthread_mutex_unlock(&lock); /* Releasing the lock.*/
      pthread_mutex_lock(&lock3); /* Here, they are waiting on the conditional variable.*/
      pthread_cond_wait(&condition1,&lock3); /* For this conditinal variable, I will use "lock3" and "condition1".*/
      pthread_mutex_unlock(&lock3);
    }
    else{ /* If it is the last one coming here.*/
      numThread = 0; /* Making numThread 0 again.*/
      //printf("1-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
      pthread_mutex_unlock(&lock); /* Releasing lock for others.*/
    }
      
    pthread_cond_signal(&condition1); /* Signaling to condional varaible.*/
    for(i = 0; i < t->t_work; i++){ /* All threads starting shift up at the same time.*/
        shiftUp((t->t_prev_work+i)); /* Again, because of each of them reaching another part of the global matrix.*/
    }/* We don't need too lock our matrix and use threads efficiency.*/
  }
}
