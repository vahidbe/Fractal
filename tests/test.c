
#include <CUnit/CUnit.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <pthread.h>
#include "fractal.c"
#include "fractal.h"
#include "tools.c"
#include <semaphore.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/stat.h>
#include "CUnit/Basic.h"
#include <fcntl.h>

struct fractal** buff;
int fractCountP=0;
char* fileName = "fractTest";
int numberThreads=2;

void testProducer(){	
  FILE* file;
  int x;
  int done=0;
  file=fopen(fileName,"r");
  if(file==NULL)
    {
      if(numberThreads==1)
	{
	  fprintf(stderr, "%s", "This file does not exist!\n");
	  exit(-1);
	}
      else
	{
	  fprintf(stderr, "%s", "One of your files does not exist!\n");
	  exit(-1);
	}
      return (NULL);
    }	

  char* buf1 = (char*) malloc(sizeof(char));
  int* buf2 = (int*) malloc(sizeof(int));
  int* buf3 = (int*) malloc(sizeof(int));
  double* buf4 = (double*) malloc(sizeof(double));
  double* buf5 = (double*) malloc(sizeof(double));
  char bufName[64];
  if((buf1==NULL)|(buf2==NULL)|(buf3==NULL)|(buf4==NULL)|(buf5==NULL))
    {
      fclose(file);				
      free(buf1);
      free(buf2);
      free(buf3);
      free(buf4);
      free(buf5);
      fprintf(stderr,"Malloc error!\n");
      exit(-1);
    }	


  x=fscanf(file,"%64s",buf1);
  while(!done){
    if(x==EOF)
      {		

	if(fclose(file)!=0)
	  {				
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    fprintf(stderr,"Error closing your file!\n");
	    exit(-1);
	  }
	free(buf1);
	free(buf2);
	free(buf3);
	free(buf4);
	free(buf5);
	done=1;
      }
    else
      {		
	if(buf1[0]=='#')
	  {
	    char trash[1024];
	    fgets(trash,1024,file);
	    x=fscanf(file,"%64s",buf1);
	  }
	else
	  {
	    char* name=buf1;
	    x=fscanf(file,"%d",buf2);
	    if(x==EOF)
	      {
		fclose(file);				
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }
	    x=fscanf(file,"%d",buf3);
	    if(x==EOF)
	      {
		fclose(file);				
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }
	    x=fscanf(file,"%lf",buf4);
	    if(x==EOF)
	      {
		fclose(file);				
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }
	    x=fscanf(file,"%lf",buf5);
	    if(x==EOF)
	      {
		fclose(file);			
		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);
		fprintf(stderr,"Error in you file, you don't respect the structure of input!\n");
		exit(-1);
	      }

	    struct fractal* f;
	    f = fractal_new(name,*buf2,*buf3,*buf4,*buf5);
	    buff[fractCountP]=f;
		fractCountP++;
        sleep(0);
	    
	    x=fscanf(file,"%64s",buf1);
	  }
      }
  }
  CU_ASSERT(0==strcmp("fract01",fractal_get_name(buff[0])));
  CU_ASSERT(0==strcmp("fract02",fractal_get_name(buff[1])));
  CU_ASSERT(0==strcmp("fract03",fractal_get_name(buff[2])));
  CU_ASSERT(1400==fractal_get_width(buff[0]));
  CU_ASSERT(1400==fractal_get_width(buff[1]));
  CU_ASSERT(1400==fractal_get_width(buff[2]));
  CU_ASSERT(900==fractal_get_height(buff[0]));
  CU_ASSERT(1400==fractal_get_height(buff[1]));
  CU_ASSERT(900==fractal_get_height(buff[2]));
  CU_ASSERT(0.5==fractal_get_a(buff[0]));
  CU_ASSERT(0.2==fractal_get_a(buff[1]));
  CU_ASSERT(-1.5==fractal_get_a(buff[2]));
  CU_ASSERT(0.1==fractal_get_b(buff[0]));
  CU_ASSERT(3.5==fractal_get_b(buff[1]));
  CU_ASSERT(2==fractal_get_b(buff[2]));
  
  return NULL;
}

void testConsumer(){
	
}

void testWriter(){
	
	return NULL;
}

int main(int argc, char *argv[])
{
	buff=malloc(sizeof(struct fractal*)*3);
	CU_pSuite pSuite = NULL;

   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   pSuite = CU_add_suite("test du main", NULL,NULL);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   } if ((NULL == CU_add_test(pSuite, "test of producer", testProducer)))// ||
      // (NULL == CU_add_test(pSuite, "test of consumer", testConsumer))||
      // (NULL == CU_add_test(pSuite, "test of writer", testWriter)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
