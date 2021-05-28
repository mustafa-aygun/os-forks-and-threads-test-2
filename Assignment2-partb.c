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

/* Instead of common dining philosophes, we have an additional fork box in this problem.
* Therefore, we should also check that box too. To follow sending 3 signal rule, I create 2 phase
* conditional waiting. First phase is for waiting signal from neighbors, second phase is waiting signal 
* for box. Also we should be aware of the situation that there is two forks on table and no forks in the box.
* At this time, we shouldn't lock the forks on the table. However, because of that, after we get signal for
* the box, we have to check their availablity. Therefore, I implement the following solution:
* 1- Phase is checking neighbors, a philosopher firstly check its right and left neighbors. If they are
* not eating, and he is hungry, he will continue to check box. If one of them is currently eating, he will move
* waiting state from his neighbor on first phase waiting state. 2- Phase: If he can reach the step checking
* box without any problem, he will check the availability of the fork in the box and look neighbors too. 
* If there is no fork, he will 
* move the waiting state  on the box. However, here, the important thing after he get signal for the box, he can
* not directly start to eating. Philosopher has to check neighbor forks again, otherwise if he can continue
* without checking them again, it means he is locking forks on the table which is not allowed. If there is
* again no problem and he can get fork from box too. Then, he will start to eating. After eating, philosopher
* will test their neighbors if they can start to eat, if they can, he will send signal to them. Also, after
* putting fork back to the box, philosopher will signal to box too. Sending signal to box is critical. 
* Let's give an example how can a deadlock may happen if we don't send signal to the box. Let's say there is
* 5 philosopher which are p0-p1-p2-p3-p4 and they can eat max 2 times. We will have 5/3=1 fork in the box. 
* If they are eating in this order, which can be happen with random sleeping time, p3 > p2 > p3 > p2 > p1 >
* p0 > p4 > p0 > p4. All philosopher have eaten 2 times except p1. However, because of p4 is the last one who 
* was eating he can't send signal to p1 with sending right and left neighbors. At this time, sending signal to
* box will solve this deadlock problem. Because p1 is already waiting on the box condition due to his neighbors
* aren't eating. Lastly, let's give an example to way I implemented with 6 philosopher and 2 forks in the box.
* Let's say p0 comes first pass the neighbors test and also box test and started to eating. Then, p1 and p5 comes,
* They will fail neighbors test and will wait on that wait condition. Let's assume 4th one is p2. He will again pass
* neighbors and box test and starting to eating. p3 will fail neighbors test too and will wait on condition.
* And after p4 comes, he won't fail neighbors test however he will fail box test because there is no fork in the
* box. p4 will wait on the box condition. Let's say p2 finishes eating first. He will send signal to p1 and p3
* which are neighbors and box too. After, neighbors get signal they will check first test again, which is 
* neighbors test. p1 will fail it again due to p0 and p3 will pass it. At this time, we should remember p4 will
* also get signal for the box and he will start to check tests again. Both p3 and p4 can pass neighbors test.
* However, only one of them can take the fork from the box because there is only one fork. After one of them will
* get the fork other one will fail box test and also neighbors test too. At this time, philosopher who is faster
* will start to eating. After they finish eating, they will give same signal again and it will continue like that.*/



/* They are our mutex and condition variables. I prefer using different ones for some cases.
* It is helping me to keep following them easily.
* I will explain each of them when they appear in code.*/
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_mutex_t lock3;
pthread_mutex_t lock4;
pthread_cond_t condition;

/* It is our philosopher struct. It will help us the keep tracking philosophers state and some other information.
* It also has a conditional variable to differentiate conditional variable of philosophers when we are sending
* signals to them.  It has p_num which is philosopher number, p_state which is philosopher state and number
* of eatings, total hungry time and total philosopher number aswell. It has also a conditional varaible.*/
struct myPhilosophers{
  int p_num;
  int p_state; /* I plan state as 3 option. 0=thinking, 1=hungry, 2=eating.*/
  int p_numEats; 
  double p_hungryTime;
  int d;
  pthread_cond_t myCond;
};

/* It is our thread struct. Here, I am keeping t_num which is equavalent to philosopher number.
* Also, again I am keeping d in thread too.*/
struct threads{
  pthread_t t_id;
  int t_num;
  int d;
};

int boxFork; /* It is a global integer variable to keep tracking Fork in the box.*/
struct myPhilosophers  **myPhilosophers; /* It is philosophers array of struct.*/

/* Basic function prototypes.*/
void *runner(void*); /* It is runner function for threads.*/
int test(int, int); /* It is for testing right and left philosophers.*/
int testWithBox(int, int); /* It is for testing with the forks in the box.*/
void printMostHungry(); /* Helping to print results.*/


int main(int argc, char const *argv[]){
  /* Defining variables.*/
  int d, i;
  pthread_attr_t attr; 

  /* Our srand to create random variables.*/
  srand(time(0));
  do{ /* Getting number of philosopher as higher than 2.*/
    printf("Enter the d higher than 2: "); /* Because (d<3)/3 gives 0, there can't be fork in box.*/
    scanf("%d",&d); /* Therefore d must be higher than 2.*/
  }while(d<3);
  

  boxFork = d/3; /* Calculating fork in the box according to d.*/
  myPhilosophers = malloc(d*sizeof(*myPhilosophers)); /* Dynamically allocing philosopher array.*/
  struct threads **threads = malloc(d*sizeof(*threads)); /* Dynamically allocing thread array.*/

  /*Initilalizing attr, condition variables and mutexs.*/
  pthread_attr_init(&attr);
  pthread_mutex_init(&lock,NULL);
  pthread_mutex_init(&lock2,NULL);
  pthread_mutex_init(&lock3,NULL);
  pthread_mutex_init(&lock4,NULL);
  pthread_cond_init(&condition,NULL);
  
  for(i = 0; i < d; i++){
    threads[i] = malloc(sizeof(struct threads)); /* Malloc each node of thread array.*/
    threads[i]->t_num = i; /* I am starting thread number from 0 as philosophers.*/
    threads[i]->d = d; /* Giving d.*/

  }

  for(i = 0; i < d; i++){
    myPhilosophers[i] = malloc(sizeof(struct myPhilosophers)); /* Malloc each node of philosopher array.*/
    myPhilosophers[i]->d = d; /* Giving d.*/
    myPhilosophers[i]->p_num = i; /* I am starting philosopher number from 0 as matching with threads.*/
    myPhilosophers[i]->p_numEats = 0; /* Number of eating is 0 at the beginning.*/
    myPhilosophers[i]->p_state = 0; /* Giving 0 which is thinking state at the beginning.*/
    myPhilosophers[i]->p_hungryTime = 0; /* Initialising hungryTime to 0 at the beginning.*/
    pthread_cond_init(&myPhilosophers[i]->myCond,NULL);  /* Initialising conditional variable for each philosopher.*/
  }

  for(i = 0; i < d; i++){/* Creating thread with sending its node as parameter.*/
    pthread_create(&threads[i]->t_id,&attr,runner,(void*)threads[i]);
  }
  for(i = 0; i < d; i++){/* Joining threads.*/
     pthread_join(threads[i]->t_id,NULL);
  }
  /* Calling the function to print result.*/
  printMostHungry();
  /* Destroying mutexs and conditional variables.*/
  pthread_mutex_destroy(&lock);
  pthread_mutex_destroy(&lock2);
  pthread_mutex_destroy(&lock3);
  pthread_mutex_destroy(&lock4);
  pthread_cond_destroy(&condition);

  return 0;
}



void* runner(void *p){
  /* Definning variables.*/
  time_t afterHungry, beforeEating; 
  int i = 0,counter,control;
  struct threads *t = (struct threads*) p; /* We are doing our thread pointers typecasting.*/
  /* Printing beginning state of the philosophers.*/
  printf("Round %d: Philosophers %d is thinking\n",i, t->t_num);
  sleep(rand()%5+1); /* Sleeping random time up to 5.*/

  for(i = 0; i < 50; i++){ /* They can eat max 50 times.*/
    myPhilosophers[t->t_num]->p_state = 1; /* After thinking they are passing hungry state.*/
    printf("Round %d: Philosophers %d is hungry\n",i, t->t_num); /* Printing state.*/
    afterHungry = time(NULL); /* Getting time after they started to be hungry.*/

    while(1){ /* It is while(1) and I will break it when philosopher started to eat. 
    * Because there is wait conditions, it won't run while loop infitinly and will wait on condition.*/
      pthread_mutex_lock(&lock); /* Before checking left and right philosopher, I am locking it.*/
      if(test(t->t_num,t->d)){ /* If checking will fail, philosopher will go else condition and wait
      * its eating neighbors signal.*/
        pthread_mutex_unlock(&lock); /* If test will pass it release the lock and continue.*/

        pthread_mutex_lock(&lock3); /* Now again we are checking if there is forks in the box. 
        * If there is fork philosopher will start to eat. If there isn't 
        * It will move else condition and wait on signal to the fork box to check it again. Here,
        * important thing is we are not locking any of right and left fork.*/
        if(testWithBox(t->t_num,t->d)){
          
          boxFork--; /* If left and right neighbor doesn't eating and there is fork in the box.*/
          if(boxFork == 1){ /* Philosopher will get that fork from the box.*/
          printf("Only one fork left at box!\n"); /* And will print warning if only one fork left in the box.*/
          }
          pthread_mutex_unlock(&lock3); /* Releasing lock on the box variable. I will use "lock3" for the boxFork.*/
          
          /* Here after philosopher got all 3 fork, starting to eating state. These operations for eating.*/
          beforeEating = time(NULL); /* Getting time before eating.*/
          myPhilosophers[t->t_num]->p_state = 2; /* Making state 2 which is eating.*/
          printf("Round %d: Philosophers %d is eating\n",i, t->t_num); /* Printing state after it changes.*/
          myPhilosophers[t->t_num]->p_hungryTime += (double)(beforeEating - afterHungry);
          /* Calculating hungry time with the information we get directly after becoming hungry and before eating.*/
          sleep(rand()%5+1); /* Philosopher eating phase taking up to 5.*/
          myPhilosophers[t->t_num]->p_numEats++; /* After eating, we increase the eating count of the philosopher.*/
          
          /* Here, these operations for the thinking state of the philosopher after he eats.*/
          myPhilosophers[t->t_num]->p_state = 0; /* Making state 0 which is thinking.*/
          printf("Round %d: Philosophers %d is thinking\n",i, t->t_num); /* Printing state after it changes.*/
          pthread_mutex_lock(&lock3); /* Locking boxFork with "lock3" again and put one fork to box again.*/
          boxFork++;
          pthread_mutex_unlock(&lock3); /* Realising lock.*/
          /* Here, I assume philosophers number is increasing clockwise. Therefore 2 is the right neighbor
          * of the 3 and 4 is the left neighbor of the 3.*/
          test((t->t_num+1)%t->d,t->d); /* Testing&signaling left neighbor.*/
          test((t->t_num+t->d-1)%t->d,t->d); /* Testing&signaling right neighbor.*/
          pthread_cond_signal(&condition); /* Signaling the box because we put fork in it.*/
          sleep(rand()%5+1); /* Philosopher thinking phase taking up to 5.*/
          break; /* Breaking the while loop and turn back to for loop where it will be hungry again.*/
        }
        else{ /* Here if there is no fork in the box, philosopher comes this else.*/
          pthread_mutex_unlock(&lock3); /* Releasing lock on the box to others can check later too.*/
          pthread_mutex_lock(&lock4); /* Entering a waiting condition. We can call this waiting on the box.*/
          pthread_cond_wait(&condition,&lock4); /* After signal will come to the box,
          * philosopher will return the beginning of the while loop and will check if the neighbors  
          * catch fork before him and starting to eat.*/
          pthread_mutex_unlock(&lock4);  
        }
        
      }
      else{ /* Here if one of the neighbor is at eating state, philosopher will come here.*/
        pthread_mutex_unlock(&lock); /* Releasing the lock on testing.*/
        pthread_mutex_lock(&lock2); /* Entering a waiting condition. We can call this waiting neighbors.*/
        pthread_cond_wait(&myPhilosophers[t->t_num]->myCond,&lock2); /* Philosopher enter waiting condition with his id.*/
        pthread_mutex_unlock(&lock2); /* After neighbor give chance to check again if he can eat or not,
        * philosopher will return to beginning of the while loop and check his neighbors again.*/
      }
    }
  }

}

int test(int p, int d){
  /* We are doing basic testing here. We are checking right and left neighbor and philosopher itself state.
  * If philosopher is hungry and left and right neighbors are not on eating state which is 2, he will send signal
  * to conditional variable with his id.*/
  if(myPhilosophers[(p+1)%d]->p_state != 2 && myPhilosophers[(p+d-1)%d]->p_state != 2 && myPhilosophers[p]->p_state == 1){
    pthread_cond_signal(&myPhilosophers[p]->myCond);
    return 1; /* We are returning 1 or 0 also to use this in the if condition.*/
  }

  return 0;
}

int testWithBox(int p, int d){
  /* We are doing basic testing here. We are checking right and left neighbor and philosopher itself state and box.
  * If philosopher is hungry and left and right neighbors are not on eating state which is 2, and there is 
  * fork in the box it will return 1 to if condition.*/
  if(myPhilosophers[(p+1)%d]->p_state != 2 && myPhilosophers[(p+d-1)%d]->p_state != 2 && boxFork>0){

    return 1; /* We are returning 1 or 0 also to use this in the if condition.*/
  }

  return 0;
}

void printMostHungry(){
  /* It is basic printing function.*/
  double highest = 0;
  int i;
  printf("-------------------------\n"); /* Firstly we are printing the all philosopher average hungry time.*/
  for(i = 0; i < myPhilosophers[0]->d; i++){ /* While doing that we also calculate the highest one.*/
    printf("Philosopher %d eat %d times and he stayed averagely %.2lf seconds hungry\n",
      myPhilosophers[i]->p_num,myPhilosophers[i]->p_numEats,myPhilosophers[i]->p_hungryTime / myPhilosophers[i]->p_numEats);
    if(highest < (myPhilosophers[i]->p_hungryTime / myPhilosophers[i]->p_numEats)){
      highest = myPhilosophers[i]->p_hungryTime / myPhilosophers[i]->p_numEats;
    }
  }
  /* After printing average hungry times and calculating the highest one. We are printing the highest
  * ones too.*/
  for(i = 0; i < myPhilosophers[0]->d; i++){
    if(highest == (myPhilosophers[i]->p_hungryTime / myPhilosophers[i]->p_numEats)){
      printf("%.2lf is the/one of the highest average hungry time which belongs to philosopher %d!\n",highest,i);
    }
  }
}