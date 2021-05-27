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

int **matrix;
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_cond_t condition;
pthread_cond_t condition1;
int shiftedLeft = 0;
int shiftedUp = 0;
int numThread = 0;

struct myThread{
  pthread_t t_id;
  int t_num;
  int t_work;
  int t_prev_work;
  int m;
  int s;
  int d;
};

void createInputTxt(int);
int** readInputTxt();
void printMatrix(int);
void shiftRight(int,int);
void shiftUp(int,int);
void *runner(void*);
void splitWork(struct myThread**,int, int);

int main(int argc, char const *argv[]){

  int m,d,s,i;
  pthread_attr_t attr; 

  srand(time(0));
  printf("Enter the m: ");
  scanf("%d",&m);
  createInputTxt(m);
  matrix = readInputTxt();
  printf("Enter the s: ");
  scanf("%d",&s);
  printf("Enter the d: ");
  scanf("%d",&d);
  while(d>m){
    printf("Enter the d: ");
    scanf("%d",&d);
  }
  
  pthread_attr_init(&attr);
  pthread_cond_init(&condition,NULL);
  pthread_cond_init(&condition1,NULL);

  pthread_mutex_init(&lock,NULL);
  pthread_mutex_init(&lock2,NULL);
  pthread_mutex_init(&lock3,NULL);

  struct myThread **threads = malloc(sizeof(struct myThread*)*d);
  for(i = 0; i < d; i++){
    threads[i] = malloc(sizeof(struct myThread));
    threads[i]->t_num = i+1;
    threads[i]->m = m;
    threads[i]->s = s;
    threads[i]->d = d;
  }
  splitWork(threads,d,m);

  for(i = 0; i < d; i++){
    pthread_create(&threads[i]->t_id,&attr,runner,(void*)threads[i]);
  }
  for(i = 0; i < d; i++){
     pthread_join(threads[i]->t_id,NULL);
  }
 
  printf("-------------------------\n");
  printMatrix(m);
  
  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock2);
  pthread_mutex_destroy(&lock3);

  pthread_cond_destroy(&condition);
  pthread_cond_destroy(&condition1);

  return 0;
}

void createInputTxt(int m){
  
  FILE *myFile; /*Creating a file.*/
  char filename[20]; /*A constant character array to arranging file name.*/
  strcpy(filename,"input.txt");
  myFile = fopen(filename, "w"); /*Opening file, if there is no file it creates it.*/
  fprintf(myFile, "%d\n", m); /*Putting m to first line of the file.*/
  int i, j;
  for (i = 0; i < m; i++){
    for(j = 0; j < m; j++){
      fprintf(myFile, "%d ", rand() % 10); /*Writing m number of random integer to one line*/
    }
    fprintf(myFile, "\n"); /*Ending line.*/
  }
  fclose(myFile); /*Closing the file.*/
}

int** readInputTxt(){
  FILE *myFile;
  char filename[20]; /*A constant character array to arranging file name.*/
  strcpy(filename,"input.txt");
  myFile = fopen(filename, "r"); /*Opening file, if there is no file it creates it.*/
  int localM,i,j;
  if (fscanf(myFile, "%d\n", &localM) != EOF); 
  int **localMatrix;
  localMatrix = malloc(sizeof(int*)*localM);
  for (i = 0; i < localM; i++){

    localMatrix[i] = malloc(sizeof(int)*localM);
  }
  for (i = 0; i < localM; i++){
    for(j = 0; j < localM; j++){
      if (fscanf(myFile, "%d ", &localMatrix[i][j]) != EOF); 
    }
    if (fscanf(myFile, "\n") != EOF); 
  }
  fclose(myFile); /*Closing the file.*/
  return localMatrix;
}

void printMatrix(int m){

  int i,j;
  for (i = 0; i < m; i++){
    for(j = 0; j < m; j++){
      printf("%d ",matrix[i][j]);
    }
    printf("\n");
  }
}

void shiftRight(int m, int row){

  int temp, i;
  temp = matrix[row][m-1];
  for(i = 0; i < m; i++){
    matrix[row][m-1-i] = matrix[row][m-2-i];
  }
  matrix[row][0] = temp;
}

void shiftUp(int m, int column){

  int temp, i;
  temp = matrix[0][column];
  for(i = m; 1<i; i--){
    matrix[m-i][column] = matrix[m+1-i][column];
  }
  matrix[m-1][column] = temp;
}

void* runner(void *p){

  int i,j;
  struct myThread *t = (struct myThread*) p;
  for(j = 0; j < t->s; j++){
    //printf("1-Hi from %d\n",t->t_num);
    if(j != 0){ 
      pthread_mutex_lock(&lock);
        if(numThread != t->d-1){
          numThread++;
          //printf("4-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
          pthread_mutex_unlock(&lock);
          pthread_mutex_lock(&lock2);
          pthread_cond_wait(&condition,&lock2);
          pthread_mutex_unlock(&lock2);
        }
        
          
          else{
          numThread = 0; 
          pthread_mutex_unlock(&lock);}
          //printf("3-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
        pthread_cond_signal(&condition);   
      
    }
    for(i = 0; i < t->t_work; i++){
        shiftRight(t->m,(t->t_prev_work+i));
    }

    pthread_mutex_lock(&lock);
    if(numThread != t->d-1){
      numThread++;
      //printf("2-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
      pthread_mutex_unlock(&lock);
      pthread_mutex_lock(&lock3);
      pthread_cond_wait(&condition1,&lock3);
      pthread_mutex_unlock(&lock3);
    }
    else{
      numThread = 0;
      //printf("1-Hi from %d with shifted %d round %d\n",t->t_num,numThread,j);
      pthread_mutex_unlock(&lock);
    }
      
    pthread_cond_signal(&condition1);
    for(i = 0; i < t->t_work; i++){
        shiftUp(t->m,(t->t_prev_work+i));
    }
  }
}

void splitWork(struct myThread **threads,int d, int m){

  int i, work,tempD = d,totalWork=0;;
  for(i = 0; i < tempD; i++){
    work = m/d;
    threads[i]->t_work = work;
    threads[i]->t_prev_work = totalWork;
    totalWork += work;
    m -= work;
    d -=1 ;
  }
}