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
#include <semaphore.h>
#include <stddef.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#define N 100                         /* nombre de places dans le tampon */

done1=0;
done2=0;

semaphore_t mutex = sema_init (1) ;   /* contrôle d'accès au tampon */ 
semaphore_t vide  = sema_init (N) ;   /* nb de places libres */
semaphore_t plein = sema_init (0) ;   /* nb de places occupées */

semaphore_t mutex2 = sema_init (1) ;   /* contrôle d'accès au tampon */ 
semaphore_t vide2  = sema_init (N) ;   /* nb de places libres */
semaphore_t plein2 = sema_init (0) ;   /* nb de places occupées */

void producteur (void)
{
  objet_t objet ; 

  while (!done1) {
    produire_objet (&objet) ;         /* produire l'objet suivant */  
    down (&vide) ;                    /* déc. nb places libres */ 
    down (&mutex) ;                   /* entrée en section critique */ 
    mettre_objet (objet) ;            /* mettre l'objet dans le tampon */ 
    up (&mutex) ;                     /* sortie de section critique */ 
    up (&plein) ;                     /* inc. nb place occupées */ 
  }
} 

void consommateur (void) 
{
  objet_t objet ; 

  while (!done2) {
    down (&plein) ;                   /* déc. nb emplacements occupés */ 
    down (&mutex) ;                   /* entrée section critique */ 
    retirer_objet (&objet) ;          /* retire un objet du tampon */ 
    up (&mutex) ;                     /* sortie de la section critique */ 
    up (&vide) ;                      /* inc. nb emplacements libres */ 
    utiliser_objet (objet) ;          /* utiliser l'objet */  
  }
}