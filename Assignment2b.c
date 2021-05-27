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

pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;

int boxFork;

int test(int, int);



struct myPhilosophers{
  int p_num;
  int p_state;
  int p_numEats;
  double p_hungryTime;
  int d;
  pthread_cond_t myCond;
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
  pthread_mutex_init(&lock,NULL);
  pthread_mutex_init(&lock2,NULL);
  
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
    pthread_cond_init(&myPhilosophers[i]->myCond,NULL); 
  }

  for(i = 0; i < d; i++){
    pthread_create(&threads[i]->t_id,&attr,runner,(void*)threads[i]);
  }
  for(i = 0; i < d; i++){
     pthread_join(threads[i]->t_id,NULL);
  }
 
  printf("-------------------------\n");
  for(i = 0; i < d; i++){
    printf("Philosopher %d eat %d times and he stayed averagely %.2lf seconds hungry\n",
      myPhilosophers[i]->p_num,myPhilosophers[i]->p_numEats,myPhilosophers[i]->p_hungryTime / myPhilosophers[i]->p_numEats);
  }


  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock2);

  pthread_cond_destroy(&condition);

  return 0;
}



void* runner(void *p){

  time_t afterHungry, beforeEating;
  int i = 0,counter,control;
  struct threads *t = (struct threads*) p;

  myPhilosophers[t->t_num]->p_state = 0;
    printf("Philosophers %d is thinking\n", t->t_num);
    sleep(rand()%5+1);

  for(i = 0; i < 2; i++){
    counter = 1; control = 0;
    myPhilosophers[t->t_num]->p_state = 1;
    printf("Philosophers %d is hungry\n", t->t_num);
    afterHungry = time(NULL);

    while(1){
      
      pthread_mutex_lock(&lock);
      if(test(t->t_num,t->d)){
      
          if(boxFork == 1){
            printf("Only one fork left at box!\n");
          }
          boxFork--;
        pthread_mutex_unlock(&lock);
        printf("Philosophers %d is eating\n", t->t_num);
        myPhilosophers[t->t_num]->p_state = 2;
        beforeEating = time(NULL);
        myPhilosophers[t->t_num]->p_hungryTime += (double)(beforeEating - afterHungry);
        sleep(rand()%5+1);
        myPhilosophers[t->t_num]->p_numEats++;
        pthread_mutex_lock(&lock);
          boxFork++;
        pthread_mutex_unlock(&lock);
        
        break;
      }
      else{
        pthread_mutex_unlock(&lock);
        pthread_mutex_lock(&lock2);
        pthread_cond_wait(&myPhilosophers[t->t_num]->myCond,&lock2);
        pthread_mutex_unlock(&lock2);
    }
    }

    myPhilosophers[t->t_num]->p_state = 0;
    printf("Philosophers %d is thinking\n", t->t_num);
    while(t->d/2 + 1 > counter && control != 1){
      if (test((t->t_num+counter)%t->d,t->d) || test((t->t_num+t->d-counter)%t->d,t->d))
      {
        control = 1;
      }
      counter ++;
    }
    
    
    sleep(rand()%5+1);
  }

}

int test(int p, int d){

  int i = 1;
  if(myPhilosophers[(p+1)%d]->p_state != 2 && myPhilosophers[(p+d-1)%d]->p_state != 2 && myPhilosophers[p]->p_state == 1 && boxFork > 0){
    pthread_cond_signal(&myPhilosophers[p]->myCond);
    return 1;
  }
  /*while(d/2 > i){
    if(myPhilosophers[(p+i)%d]->p_state == 1){
      pthread_cond_signal(&myPhilosophers[(p+i)%d]->myCond);
      break;
    }
    else if(myPhilosophers[(p+d-i)%d]->p_state == 1){
      pthread_cond_signal(&myPhilosophers[(p+d-i)%d]->myCond);
      break;
    }
    i++;
  }*/
  /*for(i = 0; i < d; i++){
    if(myPhilosophers[i]->p_state == 1){
      pthread_cond_signal(&myPhilosophers[i]->myCond);
      break;
    }
      
      
  }*/
  return 0;
}
