#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stddef.h>

int main(int argc, char *argv[]){	
  FILE* file;
  int x;
  int done=0;
  char* fileName=argv[1];
  printf("%s\n",fileName);
  file=fopen(fileName,"r");
  if(file==NULL)
    {
      printf("Erreur fopen\n");
      return -1;
    }
  char* buf1 = (char*) malloc(sizeof(char));
  int* buf2 = (int*) malloc(sizeof(int));
  int* buf3 = (int*) malloc(sizeof(int));
  double* buf4 = (double*) malloc(sizeof(double));
  double* buf5 = (double*) malloc(sizeof(double));
  char bufName[64];
  if((buf1==NULL)|(buf2==NULL)|(buf3==NULL)|(buf4==NULL)|(buf5==NULL))
    {
      printf("Erreur malloc\n");
      if(fclose(file)!=0)
	{
	  printf("Erreur close\n");
	  return -1;
	}
      return -1;
    }	
  int i;
  x=fscanf(file,"%64s",buf1);
  for(i=0;(!done);i++){
    if(x==EOF)
      {
	printf("End of file\n");
	if(fclose(file)!=0)
	  {
	    printf("Erreur close\n");
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    return (-1);
	  }
	free(buf1);
	free(buf2);
	free(buf3);
	free(buf4);
	free(buf5);
	done=1;
	printf("Done\n");
      }
    else
      {
	char* name=buf1;
	x=fscanf(file,"%d",buf2);
	if(x==EOF)
	  {
	    //TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
	      return (-1);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    return (-1);
	  }
	x=fscanf(file,"%d",buf3);
	if(x==EOF)
	  {
	    //TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
	      return (-1);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    return (-1);
	  }
	x=fscanf(file,"%lf",buf4);
	if(x==EOF)
	  {
	    //TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
	      return (-1);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    return (-1);
	  }
	x=fscanf(file,"%lf",buf5);
	if(x==EOF)
	  {
	    //TODO: gérer les erreurs/la sortie
	    if(fclose(file)!=0)
	      return (-1);
	    free(buf1);
	    free(buf2);
	    free(buf3);
	    free(buf4);
	    free(buf5);
	    return (-1);
	  }
	printf("%s %i %i %lf %lf\n",name,*buf2,*buf3,*buf4,*buf5);
	x=fscanf(file,"%64s",buf1);
      }
  }
  printf("Exit");
  return 0;
}
