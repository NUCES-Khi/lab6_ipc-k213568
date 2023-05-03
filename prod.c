  /* FILE: shm_prod.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>
#include <errno.h>
   /* NOTE: These must be same on both prod and cons */
   //--CONSTANTS--//
#define SHMNAME "/OS" //Name for our shared memory
#define SIZE (sizeof(int)*262144) //size of shared memory
#define LEN 262144
#define LENBY2 131072
    //10 integers only.
   /**
    * @brief      main function entry point
    * @param[in]  argc  The count of arguments
    * @return     normal exit or error.
    * @param      argv  The arguments array
    */
   int main(int argc, char const *argv[]) {
     //--Creating shared memory object--//
     int shmObj = shm_open(SHMNAME, O_CREAT | O_RDWR, 0600);
     //--Ask OS to reserve the sharem memory for 10 integers--//
     ftruncate(shmObj, SIZE);
     //--Ask the OS to MAP the memory to a pointer--//
     int *ptrToShm =
         (int *)mmap(0, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shmObj, 0);
     //--Writing 262144 random numbers to our shared memory--//
     for (int i = 0; i < LEN; ++i) {
       ptrToShm[i] = rand();
     }

     //--Fork to children and send the array locations to sort--//
     pid_t left = fork();
     if(left>0){
        //--Parent--//
        //--Create right child--//
        pid_t right = fork();
        if(right>0){
            //--Parent--//
            //Do nothing here//
        }else if(right==0){
            //--right child--//
            exec("cons.o",itoa(LEN/2),itoa(LEN));
        }else strerror(errno);

     }else
     if(left==0){
        //--Left Child--//
        exec("cons.o", itoa(0), itoa(LEN/2));
     }
     else{
        strerror(errno);
     }

     wait();
     wait();
     //--Merge and print the shared memory--//
     printf("==SENDER==\nI woke up from my sleep after children.\n");
     //--Merge Array here.
     for (int i = 0,j=LENBY2; i < LENBY2 || j < LEN;) {
        if(ptrToShm[i]>ptrToShm[j]){
            int tmp = ptrToShm[i];
            ptrToShm[i] = ptrToShm[j];
            ptrToShm[j] = tmp;
        }
     }
     printf("\nHurray SOrted data...\n");
     //--Safely release and close the shared memory--//
     munmap(ptrToShm, SIZE);
     close(shmObj);
     shm_unlink(SHMNAME);
     return EXIT_SUCCESS;
   } // end