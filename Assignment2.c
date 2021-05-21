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

struct myThread{
  pthread_t t_id;
  int t_num;
  int t_work;
  int t_prev_work;
  int m;
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
  struct myThread **threads = malloc(sizeof(struct myThread*)*d);
  for(i = 0; i < d; i++){
    threads[i] = malloc(sizeof(struct myThread));
    threads[i]->t_num = i+1;
    threads[i]->m = m;
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
  printf("Hello from %d\n",t->t_num);
 
  for(i = 0; i < t->t_work; i++){
      shiftRight(t->m,(t->t_prev_work+i));
  }
  printMatrix(t->m);
  
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