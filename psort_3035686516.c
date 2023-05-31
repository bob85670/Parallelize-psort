// COMP3230 Programming Assignment Two
// The sequential version of the sorting using qsort -- multithreaded Program

/*
# Filename: psort_3035686516.c
# Student name and No.: Lau Tsz Yin 3035686516
# Development platform: Docker Ubtuntu Image
# Remark: Complete all 5 phase. 
          I have used 3 semaphores.
          All arrays can be sorted. 
          See my screenshot "proof in terminal".
*/

// Docker: focused_sammet
// gcc working.c -o psort
// ./psort 100000 4 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>

int checking(unsigned int *, long);
int compare(const void *, const void *);

// global variables
long size;  // size of the array
unsigned int * intarr; // array of random integers
int numOfWorker;



sem_t Sem1;
sem_t Sem3;
sem_t Sem5;

int thread_ids[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};


int Bigarray[30][100000];
int sizeOf_temparr[10];
int phase2array[1000];
int phase3pivot[32];
int phase4temparray[30][100000];
int phase4continueLengtharray[30];

int phase4partitionarray[30][100000];

int finalarray[100000];


int jagcount = 0;
int count = 0;
int currentEle = 0;
int index2 = 0;
int index3 = 0;

//int countdebug = 0;



void *phrase1func(void *idp){

  int *my_id = (int *)idp;

  //printf("Thread %d arrives in phrase1func()\n", *my_id);
  sem_wait(&Sem1);
  //printf("\n");
  //printf("Thread %d arrives in semaphore!\n", *my_id);

  // each worker thread gets n/p numbers
  int numTakenEach = size / numOfWorker;

  if(*my_id == numOfWorker){
    numTakenEach = size - (size / numOfWorker) * (*my_id - 1);
  }

  //printf("numTakenEach is %d\n", numTakenEach);

  //printf("intarr[0]: %d", intarr[0]);


  // allocate memory for each "sub array", it is an array of pointers
  int *jagged[numTakenEach];

  for (int i = 0; i < numTakenEach; i++){
    jagged[i] = malloc(sizeof(int) * 1);
  }


  //printf("allocated size.");


  // amend sizeOf_temparr array
  sizeOf_temparr[(*my_id-1)] = numTakenEach;

  for(int i = 0; i < sizeof(sizeOf_temparr); i++){
    //printf("sizeof_temparr %d\n", sizeOf_temparr[i]);
  }


  // for index of intarr
  currentEle = numTakenEach * ((*my_id-1));

  if(*my_id == numOfWorker){
    currentEle = size - numTakenEach;
  }

  //printf("currentEle is %d\n", currentEle);

  // unchanged pointer pointing to last element of each sub array
  int *lastElementPtr;
  lastElementPtr = &intarr[currentEle + numTakenEach - 1];
  //printf("lastElementPtr's index is %d\n", currentEle + numTakenEach - 1);
  //printf("lastElementPtr is %d\n", *lastElementPtr);



  // initialize iterate pointer
  int *p;
  p = &intarr[currentEle];
  //printf("p is %d\n", *p);


  
  // copy element in intarr[] to jagged[]
  int j = 0;
  while(1){


    if (p != lastElementPtr){
      *jagged[j] = intarr[currentEle+j];


      //printf("jagged[%d] = %d\n", j, *jagged[j]);

      j++;
      p++;
      //countdebug ++; 


    }else{
      *jagged[j] = intarr[currentEle+j];
      //printf("jagged[%d] = %d\n", j, *jagged[j]);

      //countdebug ++; 
      //printf("countdebug %d\n", countdebug);

      //printf("p arrives to the end.\n");

      j++;
      break;
    }
  }


  // qsort
  int qsortArray[j];

  //printf("j = %d\n", j);
  for (int i = 0; i < j; i++){
    qsortArray[i] = *jagged[i];

    //printf("qsortArray[%d] = %d\n", i, qsortArray[i]);
  }


  qsort(qsortArray, j, sizeof(int), compare);

  //debugging
  //for (int i = 0; i < j; i++){
    //printf("qsortArray[%d] after sorting = %d\n", i, qsortArray[i]);
  //}



  // store the subarray in Bigarray
  for (int i = 0; i < j; i++){
    Bigarray[(*my_id-1)][i] = qsortArray[i];
    //printf("Bigarray[%d][%d] = %d\n", (*my_id-1), i, Bigarray[(*my_id-1)][i]);

  }



  // each worker selects p samples from its 'local' sequence at corresponding indices and put into phase2array
  int x = 0;
  int currentIndex;

  for (int i = 0; i < numOfWorker; i++){
    currentIndex = (size * x)/ (numOfWorker * numOfWorker);

    //printf("currentIndex: %d\n", currentIndex);

    if (currentIndex < numTakenEach){
      phase2array[index2] = qsortArray[currentIndex];

      //printf("phase2array[%d] = %d\n", index2, phase2array[index2]);
    }

    index2 ++;

    x++;
  }


  
  sem_post(&Sem1);
  //printf("Thread %d leaves semaphore!\n", *my_id);


  pthread_exit(NULL);
}





void *phrase3func(void *idp){

  int *my_id = (int *)idp;

  //printf("Thread %d arrives in phrase3func()\n", *my_id);
  sem_wait(&Sem3);
  //printf("\n");
  //printf("Thread %d arrives in semaphore 3!\n", *my_id);


  int phase4lengtharray[30];
  

  //partition according to phase3pivot, temparory store in phase4temparray
  int bigarrayIndex = 0;
  int rightIndex = 0;
  int phase4Index = 0;
  int count = 0;
  int lastcount = 0;
  int size = sizeOf_temparr[*my_id-1];

  int sublength = 0;

  //printf("index3 = %d, size = %d\n", index3, size);


  while (count < (index3+size)){
    if (Bigarray[(*my_id-1)][bigarrayIndex] <= phase3pivot[rightIndex]){

      //printf("%d, pivot = %d\n", Bigarray[(*my_id-1)][bigarrayIndex], phase3pivot[rightIndex]);

      
      phase4temparray[rightIndex][phase4Index] = Bigarray[(*my_id-1)][bigarrayIndex];

      //printf("phase4temparray[%d][%d] = %d\n", rightIndex, phase4Index, phase4temparray[rightIndex][phase4Index]);


      bigarrayIndex++;
      phase4Index++;

      count ++;
      lastcount ++;
      //printf("count = %d\n", count);

      sublength ++;
      //printf("sublength = %d\n", sublength);


    }else{


      rightIndex++;
      phase4Index = 0;
      count++;

      phase4lengtharray[rightIndex-1] = sublength;

      sublength = 0;

      //printf("count in else= %d\n", count);

      if (rightIndex == (index3-1)){
        break;
      }
    }
  }


  // last part of partition 
  //printf("rightIndex %d\n", rightIndex);
  //printf("lastcount %d\n", lastcount);
  //printf("bigarrayIndex %d\n", bigarrayIndex);
  phase4Index = 0;
  sublength = 0;
  for (lastcount; lastcount < size; lastcount ++){
    phase4temparray[rightIndex][phase4Index] = Bigarray[(*my_id-1)][bigarrayIndex];

    //printf("phase4temparray[%d][%d] = %d\n", rightIndex, phase4Index, phase4temparray[rightIndex][phase4Index]);

    bigarrayIndex++;
    phase4Index++;

    sublength++;

  }

  // mark down length of subarray
  phase4lengtharray[rightIndex] = sublength;

  // for (int i = 0; i<index3; i++){
  //   printf("phase4lengtharray[%d] = %d\n", i,  phase4lengtharray[i]);
  // }



  //store back to phase4partitionarray
  for (int x = 0; x < index3; x++){
    for (int y = 0; y < phase4lengtharray[x]; y++){


      phase4partitionarray[x][phase4continueLengtharray[x]] = phase4temparray[x][y];

      //printf("phase4partitionarray[%d][%d] : %d\n", x, phase4continueLengtharray[x], phase4partitionarray[x][phase4continueLengtharray[x]]);

      phase4continueLengtharray[x] ++;
    }
    

  }

  // for (int i = 0; i < index3; i++){
  //   printf("phase4continueLengtharray[%d] = %d\n", i, phase4continueLengtharray[i]);
  // }



  sem_post(&Sem3);
  //printf("Thread %d leaves semaphore!\n", *my_id);


  pthread_exit(NULL);
}





void *phrase5func(void *idp){
  int *my_id = (int *)idp;

  //printf("Thread %d arrives in phrase5func()\n", *my_id);
  sem_wait(&Sem5);
  //printf("\n");
  //printf("Thread %d arrives in semaphore 5!\n", *my_id);


  //make phase5temparray[] a temporary 1D array
  int phase5temparray[phase4continueLengtharray[(*my_id-1)]];

  for (int i = 0; i < phase4continueLengtharray[(*my_id-1)]; i++){
    phase5temparray[i] = phase4partitionarray[(*my_id-1)][i];

    //printf("phase5temparray[%d] : %d\n", i, phase5temparray[i]);
  }



  //qsort
  qsort(phase5temparray, phase4continueLengtharray[(*my_id-1)], sizeof(int), compare);

  //debugging
  // for (int i = 0; i < phase4continueLengtharray[(*my_id-1)]; i++){
  //   printf("phase5temparray[%d] after sorting in Phase 5= %d\n", i, phase5temparray[i]);
  // }


  //put the sorted array into finalarray[],
  int startingpoint;

  if (*my_id == 1){
    startingpoint = 0;
  }else if (*my_id == 2){
    startingpoint = phase4continueLengtharray[0];
  }else{
    startingpoint = phase4continueLengtharray[0];
    for (int i = 1; i < (*my_id-1); i++){
      startingpoint = startingpoint + phase4continueLengtharray[i]; 

    }
  }

  //printf("startingpoint : %d\n", startingpoint);



  for (int j = 0; j < phase4continueLengtharray[(*my_id-1)]; j++){
    finalarray[startingpoint+j] = phase5temparray[j];

    //printf("finalarray[%d] : %d\n", startingpoint+j, finalarray[startingpoint+j]);

  }



  sem_post(&Sem5);
  //printf("Thread %d leaves semaphore!\n", *my_id);


  pthread_exit(NULL);
}




void *captainthreadfunc (void *arg){

  pthread_t threads[numOfWorker];
  sem_init(&Sem1, 0, 1);

  
  //printf("Captain thread arrives.\n");

  //printf("intarr[0]: %d ,intarr[1]: %d , no. of item in intarr: %lu\n", intarr[0], *(intarr+1), size);

  //**************************************** Phase 1 **************************************** 

  for (int t = 0; t < numOfWorker; t++){
    int threadnum = t+1;
    pthread_create(&threads[t], NULL, phrase1func, (void*)&thread_ids[t]);
  }

  for (int i = 0; i < numOfWorker; i++){
    pthread_join(threads[i], NULL);
  }
  //printf("Captain threads is back now.\n");
  //printf("\n");


  //**************************************** Phase 2 **************************************** 

  //qsort
  // printf("index2 = %d\n", index2);

  qsort(phase2array, index2, sizeof(int), compare);

  //debugging
  // for (int i = 0; i < index2; i++){
  //   printf("phase2array[%d] after sorting in Phase 2= %d\n", i, phase2array[i]);
  // }




  // Selects p-1 pivots from the phase2array
  int y = 1;
  int currentIndex2;
  for (int j = 0; j < numOfWorker; j++){
    currentIndex2 = y * numOfWorker + (numOfWorker/2) -1;

    //printf("currentIndex2: %d\n", currentIndex2);

    if ((currentIndex2 < sizeof(phase2array)) && phase2array[currentIndex2]!=0){
      phase3pivot[index3] = phase2array[currentIndex2];

      //printf("phase3pivot[%d] = %d\n", index3, phase3pivot[index3]);

    }

    index3 ++;

    y++;
  }


  //**************************************** Phase 3 & 4 **************************************** 
  
  //printf("Phase3 starts...\n");


  sem_init(&Sem3, 0, 1);

  for (int t = 0; t < numOfWorker; t++){
    int threadnum = t+1;
    pthread_create(&threads[t], NULL, phrase3func, (void*)&thread_ids[t]);
  }

  for (int i = 0; i < numOfWorker; i++){
    pthread_join(threads[i], NULL);
  }
  
  //printf("Captain threads is back now after Phase 4.\n");
  //printf("\n");



  //**************************************** Phase 5 **************************************** 
  
  //printf("Phase5 starts...\n");

  sem_init(&Sem5, 0, 1);

  for (int t = 0; t < numOfWorker; t++){
    int threadnum = t+1;
    pthread_create(&threads[t], NULL, phrase5func, (void*)&thread_ids[t]);
  }

  for (int i = 0; i < numOfWorker; i++){
    pthread_join(threads[i], NULL);
  }

  //printf("Captain threads is back now after Phase 5.\n");
  //printf("\n");


  //**************************************** After Phase 5 **************************************** 
  
  //printf("Print final array here ------------------------------ \n");
  for (int i = 0; i < size; i++){
    intarr[i] = finalarray[i];


    //printf("intarr[%d]: %d\n", i, intarr[i]);
  }
  //printf("Finish Printing final array ------------------------------ \n");

  


  pthread_exit(NULL);
}





int main (int argc, char **argv)
{
  long i, j;
  struct timeval start, end;

  pthread_t thread_captain;

  // if ((argc != 2))
  // {
  //   printf("Usage: seq_sort <number>\n");
  //   exit(0);
  // }

  //can enter num of workers in argument, default is 4.
  if ((argc == 2)){
    numOfWorker = 4;
  }else{
    numOfWorker = atol(argv[2]);
  }

    
  size = atol(argv[1]);
  intarr = (unsigned int *)malloc(size*sizeof(unsigned int));
  if (intarr == NULL) {perror("malloc"); exit(0); }
  
  // set the random seed for generating a fixed random
  // sequence across different runs
  char * env = getenv("RANNUM");  //get the env variable
  if (!env)                       //if not exists
    srandom(3230);
  else
    srandom(atol(env));
  
  for (i=0; i<size; i++) {
    intarr[i] = random();
  }
  
  // measure the start time
  gettimeofday(&start, NULL);
  
  // just call the qsort library
  // replace qsort by your parallel sorting algorithm using pthread
      //[previous code] qsort(intarr, size, sizeof(unsigned int), compare);

  pthread_create(&thread_captain, NULL, captainthreadfunc, NULL);
  pthread_join(thread_captain, NULL);



  // measure the end time
  gettimeofday(&end, NULL);
  
  if (!checking(intarr, size)) {
    printf("The array is not in sorted order!!\n");
  }
  
  printf("Total elapsed time: %.4f s\n", (end.tv_sec - start.tv_sec)*1.0 + (end.tv_usec - start.tv_usec)/1000000.0);
    
  free(intarr);
  return 0;
}

int compare(const void * a, const void * b) {
  return (*(unsigned int *)a>*(unsigned int *)b) ? 1 : ((*(unsigned int *)a==*(unsigned int *)b) ? 0 : -1);
}

int checking(unsigned int * list, long size) {
  long i;
  printf("First : %d\n", list[0]);
  printf("At 25%%: %d\n", list[size/4]);
  printf("At 50%%: %d\n", list[size/2]);
  printf("At 75%%: %d\n", list[3*size/4]);
  printf("Last  : %d\n", list[size-1]);
  for (i=0; i<size-1; i++) {
    if (list[i] > list[i+1]) {
      return 0;
    }
  }
  return 1;
}

