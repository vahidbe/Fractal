#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fractal.h"
#include "fractal.c"
#include "tools.c"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>

int main(int argc, char *argv[])
{
  int entreestd = 1;
  pthread[(argc-1)%5] threads;
  if(entreestd){
    for(int i=0;(i<argc-1) % 5;i++){
      struct fractal* f= fractal_create(argv[0+i*5],toInt(argv[1+i*5]),toInt(argv[2+i*5]),toInt(argv[3+i*5]),toInt(argv[4+i*5]));
      p_thread_create(
					
}


	int sbuf_remove(sbuf_t *sp)
	{
	  
