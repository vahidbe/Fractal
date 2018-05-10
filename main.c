#include "./libfractal/fractal.h"
#include "./FractalMaker.h"

int main(int argc, char *argv[])
{
  if(argc<2)
  {
    fprintf(stderr,"Not enough arguments\n");
    return -1;
  }
  printf("\n=== Début du programme ===\n");
  numberThreads=0;
  countProd = 0;
  countCons = 0;
  countWrit = 0;
  countEleves = 0;
  average = 0.0;
  maxEgal = 0;
  pthread_mutex_init(&tuteur1,NULL);
  pthread_mutex_init(&tuteur2,NULL);
  pthread_mutex_init(&professor,NULL);
  pthread_mutex_init(&gardien,NULL);
  int count;
  int optionsCount=0;
  optionD=0;
  bufIn=(struct sbuf*)malloc(sizeof(struct sbuf)); 
  bufOut=(struct sbuf*)malloc(sizeof(struct sbuf)); 
  highestF=(struct sbuf*)malloc(sizeof(struct sbuf));
  bufFName=(struct charbuf*)malloc(sizeof(struct charbuf));
	
  if((bufIn==NULL)|(bufOut==NULL)|(highestF==NULL)|(bufFName==NULL))
    {
      fprintf(stderr,"\nMalloc error!\n");
      fflush(stderr);
      exit(-1);
    }

  if((*argv[1]=='-')&(*(argv[1]+1)=='d'))
    {
      optionD=1;
      optionsCount++;
      if(argc>2)
	{
	  if((*(argv[2])=='-')&(*(argv[2]+1)=='-'))
	    {  
	      if ((*(argv[2]+2)=='m')&(*(argv[2]+3)=='a')&(*(argv[2]+4)=='x')&(*(argv[2]+5)=='t')&(*(argv[2]+6)=='h')&(*(argv[2]+7)=='r')&(*(argv[2]+8)=='e')&(*(argv[2]+9)=='a')&(*(argv[2]+10)=='d')&(*(argv[2]+11)=='s'))
		{
		  numberThreads = atoi(argv[3]);
		  optionsCount++;
		  optionsCount++;
		}
	      else
		{
		  fprintf(stderr,"\nWrong option, verify the option --maxthreads\n");
		  fflush(stderr);
		  optionsCount++;
		  optionsCount++;
		}
	    }
	}
    }
  else{
    if((*argv[1]=='-')&(*(argv[1]+1)=='-'))
      {  
	if ((*(argv[1]+2)=='m')&(*(argv[1]+3)=='a')&(*(argv[1]+4)=='x')&(*(argv[1]+5)=='t')&(*(argv[1]+6)=='h')&(*(argv[1]+7)=='r')&(*(argv[1]+8)=='e')&(*(argv[1]+9)=='a')&(*(argv[1]+10)=='d')&(*(argv[1]+11)=='s'))
	  {
	    numberThreads = atoi(argv[2]);
	    optionsCount++;
	    optionsCount++;
	  }
	else
	  {
	    fprintf(stderr,"\nWrong option, verify the option --maxthreads\n");
	    fflush(stderr);
	    optionsCount++;
	    optionsCount++;
	  }
      }
  }	
  if(numberThreads==0)
    {
      numberThreads=argc-2-optionsCount; 
    }
  numberProd = argc-2-optionsCount;
  if(numberProd==0)
    {
      fprintf(stderr,"You did'nt enter a file!\n");
      fflush(stderr);
      free(bufIn);
      free(bufOut);
      free(highestF);
      free(bufFName);
      exit(-1);
    }
	
  char* string;
  if(optionD)
    {
      string="OUI";
    }
  else
    {
      string="NON";
    }
	
  /**/printf("\n\n - Affichage de toutes les fractales : %s -\n",string);
  /**/fflush(stdout);
  /**/printf("\n - Nombre de threads qui vont être utilisés : %d -\n\n",numberThreads);
  /**/fflush(stdout);
  sem_init(&directeur, 0 ,numberThreads);
  sbuf_init(bufIn, 50);        						
  sbuf_init(bufOut, 50);
  sbuf_init(highestF, 50);   
  charbuf_init(bufFName, 200);
	
  if(numberThreads==0)
    {
      fprintf(stderr,"You did not enter a file!\n");
      fflush(stderr);
      exit(-1);
    }
  pthread_t prod[numberProd];
  pthread_t cons[numberThreads];
  pthread_t writ[numberThreads];
	
  /**/printf("--- Initialisation du main terminée ---\n");
  /**/fflush(stdout);
	
  for(count=optionsCount+1;count<argc;count++)
    {
      if(((*argv[count])=='-')&(count!=(argc)))
	{ 	
	  int endOfInput=0;
	  while(!endOfInput)
	    {
	      char* name=malloc(sizeof(char));
	      if(name==NULL)
		{
		  sbuf_clean(bufIn);
		  sbuf_clean(bufOut);
		  sbuf_clean(highestF);
		  charbuf_clean(bufFName);
		  exit(-1);
		}
	      int* height=malloc(sizeof(int));
	      if(height==NULL)
		{
		  free(name);
		  sbuf_clean(bufIn);
		  sbuf_clean(bufOut);
		  sbuf_clean(highestF);
		  charbuf_clean(bufFName);
		  exit(-1);
		}
	      int* width=malloc(sizeof(int));
	      if(width==NULL)
		{
		  free(name);
		  free(height);
		  sbuf_clean(bufIn);
		  sbuf_clean(bufOut);
		  sbuf_clean(highestF);
		  charbuf_clean(bufFName);
		  exit(-1);
		}
	      double* a=malloc(sizeof(double));
	      if(a==NULL)
		{
		  free(name);
		  free(width);
		  free(height);
		  sbuf_clean(bufIn);
		  sbuf_clean(bufOut);
		  sbuf_clean(highestF);
		  charbuf_clean(bufFName);
		  exit(-1);
		}
	      double* b=malloc(sizeof(double));
	      if(b==NULL)
		{
		  free(name);
		  free(width);
		  free(height);
		  free(a);
		  sbuf_clean(bufIn);
		  sbuf_clean(bufOut);
		  sbuf_clean(highestF);
		  charbuf_clean(bufFName);
		  exit(-1);
		}
	      char* y=malloc(sizeof(char));
	      if(y==NULL)
		{
		  free(name);
		  free(width);
		  free(height);
		  free(a);
		  free(b);
		  sbuf_clean(bufIn);
		  sbuf_clean(bufOut);
		  sbuf_clean(highestF);
		  charbuf_clean(bufFName);
		  exit(-1);
		}
	      int bonNom = 0;
	      sleep(1);
	      do{
	        puts("\nDonnez le nom de la fractale :");
	        scanf("%64s", name);
 	        if(charbuf_already_used(bufFName,name))
		  {
		    fprintf(stderr,"\nCe nom est déjà utilisé par une autre fractale, veuillez en choisir un autre\n");
		    fflush(stderr);
		  }
	        else
	          {
		    bonNom=1;
		  }
	      } while(!bonNom);
	      puts("Donnez la largeur de la fractale :");
	      scanf("%d", width);
	      puts("Donnez la hauteur de la fractale :");
	      scanf("%d", height);
	      puts("Donnez la partie réelle du c :");
	      scanf("%lf", a);
	      puts("Donnez la partie imaginaire du c :");
	      scanf("%lf", b);
	      puts("Voulez-vous entrer une autre fractale? (y/n) :");
	      scanf("%1s", y);
	      struct fractal* f=fractal_new(name,*width,*height,*a,*b);
	      sbuf_insert(bufIn,f);
	      fractCountP++;	  
	  if(*y!='y')
	    {
	      endOfInput=1;
	      free(name);
	      free(height);
	      free(width);
	      free(a);
	      free(b);
	      free(y);
	    }
	}			
      countProd++;	
    }
  else
    {
      if(count!=(argc-1))
	{
	  struct args* arguments=malloc(sizeof(struct args));
	  if(arguments==NULL)
	    {
	      goto end;
	    }
	  arguments->charP_arg=argv[count];
	  sem_wait(&directeur);
	  pthread_create(&(prod[count-optionsCount]), NULL, (void*) &producer, arguments);
	}
      else
	{
	  fileOutName=argv[count];
	}	
    }
}
	
/**/printf("--- Initialisation des producteurs terminée ---\n");
/**/fflush(stdout);
	
int i;
for(i=0;(i<numberThreads);i++)
  {
    sem_wait(&directeur);
    pthread_create(&(cons[i]), NULL, (void*) &consumer, NULL);
  }
	
/**/printf("--- Initialisation des consommateurs terminée ---\n");
/**/fflush(stdout);

for(i=0;i<numberThreads;i++)
  {
    sem_wait(&directeur);
    pthread_create(&(writ[i]), NULL, (void*) &writer, NULL);
  }	
/**/printf("--- Initialisation des writers terminée ---\n");
/**/fflush(stdout);
/**/printf("=== Initialisation terminée ===\n");
/**/fflush(stdout);

while(countWrit<numberThreads)
  {
  }
end:
sbuf_clean(bufIn);
sbuf_clean(bufOut);
sbuf_clean(highestF);
	
/**/printf("\n\n=== Fin du programme ===\n\n");
/**/fflush(stdout);
	
return 0;
}
