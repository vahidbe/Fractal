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
	int fdi=0;
	int done=0;
	char* fileName=argv[1];
	printf("%s\n",fileName);
	fdi=open(fileName,O_RDONLY);
	if(fdi<0)
	{
		printf("Erreur open\n");
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
		if(close(fdi)!=0)
		{
			printf("Erreur close\n");
			return -1;
		}
		return -1;
	}	
	ssize_t x=0;
	int i;
	int j;
	printf("About to read\n");
	x=read(fdi,buf1,sizeof(char));
	printf("First read\n");
	for(i=0;(!done);i++){
		if(x!=sizeof(char))
		{
			printf("End of file\n");
			if(close(fdi)!=0)
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
			printf("About to set done to 1\n");
			done=1;
			printf("Done=1\n");
		}
		else
		{
			bufName[0]=*buf1;
			j=1;
			for(x=read(fdi,buf1,sizeof(char));((*buf1)!=' ');x=read(fdi,buf1,sizeof(char)))
			{
				if(x==-1)
				{
					printf("Erreur read char*\n");
					if(close(fdi)!=0)
					{
						printf("Erreur close\n");
						return (-1);
					}
					free(buf1);
					free(buf2);
					free(buf3);
					free(buf4);
					free(buf5);
					return (-1);
				}
				bufName[j]=*buf1;
				j++;
			}
			char name[j];
			int k;
			for(k=0;k<j;k++)
			{
				name[k]=bufName[k];
			}
			x=read(fdi,buf2,sizeof(int));
			printf("%d/n",*buf2);
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					return (-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (-1);
			}
			x=read(fdi,buf3,sizeof(int));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					return (-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (-1);
			}
			x=read(fdi,buf4,sizeof(double));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					return (-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (-1);
			}
			x=read(fdi,buf5,sizeof(double));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					return (-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				return (-1);
			}
			printf("%s %i %i\n",name,*buf2,*buf3);
		}
	}
	return 0;
}
