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

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

int boxFork;

int test(int, int);

struct myPhilosophers{
  pthread_t p_id;
  int p_num;
  int p_state;
  int p_numEats;
  double p_hungryTime;
  int d;
};

struct threads{
  pthread_t t_id;
  int t_num;
  int d;
};

struct myPhilosophers  **myPhilosophers;

void *runner(void*);


int main(int argc, char const *argv[]){

  int d, i;
  pthread_attr_t attr; 
  srand(time(0));

  printf("Enter the d: ");
  scanf("%d",&d);

  boxFork = d/3;
  myPhilosophers = malloc(d*sizeof(*myPhilosophers));
  struct threads **threads = malloc(d*sizeof(*threads));

  pthread_attr_init(&attr);
  
  for(i = 0; i < d; i++){
    threads[i] = malloc(sizeof(struct threads));
    threads[i]->t_num = i;
    threads[i]->d = d;

  }

  for(i = 0; i < d; i++){
    myPhilosophers[i] = malloc(sizeof(struct myPhilosophers));
    myPhilosophers[i]->d = d;
    myPhilosophers[i]->p_num = i;
    myPhilosophers[i]->p_numEats = 0;
    myPhilosophers[i]->p_state = 0;
    myPhilosophers[i]->p_hungryTime = 0;
  }

  for(i = 0; i < d; i++){
    pthread_create(&threads[i]->t_id,&attr,runner,(void*)threads[i]);
  }
  for(i = 0; i < d; i++){
     pthread_join(threads[i]->t_id,NULL);
  }
 
  printf("-------------------------\n");
  for(i = 0; i < d; i++){
    printf("Philosopher %d eat %d times and he stayed %lf seconds hungry\n",
      myPhilosophers[i]->p_num,myPhilosophers[i]->p_numEats,myPhilosophers[i]->p_hungryTime);
  }
  return 0;
}



void* runner(void *p){

  clock_t afterHungry, beforeEating;
  int i = 0;
  struct threads *t = (struct threads*) p;
  for(i = 0; i < 2; i++){
    myPhilosophers[t->t_num]->p_state = 1;
    afterHungry = clock();
    while(1){

        pthread_mutex_lock(&lock);
      if(test(t->t_num,t->d) && boxFork > 0){
        
          boxFork--;
          if(boxFork == 1){
            printf("Only one fork left at box!\n");
          }
        pthread_mutex_unlock(&lock);
        printf("Philosophers %d is eating\n", t->t_num);
        myPhilosophers[t->t_num]->p_state = 2;
        beforeEating = clock();
        myPhilosophers[t->t_num]->p_hungryTime += (double)(beforeEating - afterHungry) / CLOCKS_PER_SEC ;
        sleep(rand()%5+1);
        
        pthread_mutex_lock(&lock);
          boxFork++;
        pthread_mutex_unlock(&lock);
        myPhilosophers[t->t_num]->p_numEats++;
        break;
      }
      else{
        pthread_mutex_unlock(&lock);
        pthread_mutex_lock(&lock2);
        pthread_cond_wait(&condition,&lock2);
        pthread_mutex_unlock(&lock2);
    }
    }

    myPhilosophers[t->t_num]->p_state = 0;
    pthread_cond_broadcast(&condition);
    printf("Philosophers %d is thinking\n", t->t_num);
    sleep(rand()%5+1);
  }

}

int test(int p, int d){

  if(myPhilosophers[(p+1)%d]->p_state != 2 && myPhilosophers[(p+d-1)%d]->p_state != 2 && myPhilosophers[p]->p_state == 1)
    return 1;
  return 0;
  
}
