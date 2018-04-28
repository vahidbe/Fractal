int main(int argc, char *argv[]){	
	int fdi=0;
	int done = 0;
	char* fileName=argv[0];
	if(open(fileName,fdi)<0)
		exit(-1);
		//TODO: gérer les erreurs/la sortie
	char* buf1 = (char*) malloc(sizeof(char));
	int* buf2 = (int*) malloc(sizeof(int));
	int* buf3 = (int*) malloc(sizeof(int));
	double* buf4 = (double*) malloc(sizeof(double));
	double* buf5 = (double*) malloc(sizeof(double));
	char bufName[64];
	if((buff==NULL)|(buff==NULL)|(buff==NULL)|(buff==NULL)|(buff==NULL))
	{
		if(close(fdi)!=0)
			exit(-1);		
		//TODO: gérer les erreurs/la sortie
		exit(-1);
	}	
	size_t x=0;
	int i;
	int j;
	x=read(fdi,buf1,sizeof(char));
	for(i=0;!done;i++){
		if(x!=sizeof(char))
		{
			if(close(fdi)!=0)
			{
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
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
			bufName[0]=*buf1;
			j=1;
			for(x=read(fdi,buf1,sizeof(char));*buf1!=' ');x=read(fdi,buf1,sizeof(char)))
			{
				if(x==-1)
				{
					//TODO: gérer les erreurs/la sortie
					if(close(fdi)!=0)
						exit(-1);
					free(buf1);
					free(buf2);
					free(buf3);
					free(buf4);
					free(buf5);
					exit(-1);
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
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			x=read(fdi,buf3,sizeof(int));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			x=read(fdi,buf4,sizeof(double));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			x=read(fdi,buf5,sizeof(double));
			if(x==-1)
			{
				//TODO: gérer les erreurs/la sortie
				if(close(fdi)!=0)
					exit(-1);
				free(buf1);
				free(buf2);
				free(buf3);
				free(buf4);
				free(buf5);
				exit(-1);
			}
			printf("%s %i %i\n",name,*buf2,*buf3);
		}
	}
	return 0;
}