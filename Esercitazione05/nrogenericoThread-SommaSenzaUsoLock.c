/* OBIETTIVO: generare un numero non noto di threads che eseguono la modifica di una variabile globale in un ciclo, all'interno del quale stampano il valore corrente di tale variabile, 
il loro numero d'ordine e il loro identificatore; il risultato voluto sarebbe quello che ogni thread dovrebbe eseguire il ciclo di modifica in mutua esclusione 
==> poiche' il ciclo di modifica NON e' considerato una sezione critica, il risultato della esecuzione NON e' quello sperato */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

int x = 0; /* variabile globale e quindi condivisa da tutti i Pthread creati (sarebbe di default inizializzata a 0, ma lo facciamo esplicitamente per rendere il codice piu' chiaro */

void *Incremento(void *id)  /* in questo caso alla START_ROUTINE e' stato dato un nome specifico relativo al 'compito' che i vari thread figli devono svolgere */
{
   int *pi = (int *)id;
   int *ptr;
   int j; 	/* variabile per il ciclo */

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   /* ogni thread figlio incrementa di 1 il valore della variabile globale x (CONDIVISA fra tutti i thread) per 10 volte 
   N.B. invece che il valore costante 10 potrebbe essere passato un altro parametro all'atto della invocazione */
   for (j=0; j < 10; j++)
   {
   	x = x + 1;
   	printf("Thread%d e identificatore %lu: il valore di x e' = %d\n", *pi, pthread_self(), x);
	/* stampa del numero del thread (passato come parametro), del TID e del valore corrente di x */
   }

   /* pthread vuole tornare al padre un valore intero, ad es 1000+id */
   *ptr = 1000+*pi;
   pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
   pthread_t *thread;
   int *taskids;
   int i;
   int *p;
   int NUM_THREADS;
   char error[250];

   /* Controllo sul numero di parametri */
   if (argc != 2 ) /* Deve essere passato esattamente un parametro */
   {
   	sprintf(error,"Errore nel numero dei parametri %d\n", argc-1);
	perror(error);
        exit(1);
   }

   /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
   NUM_THREADS = atoi(argv[1]);
   if (NUM_THREADS <= 0) 
   {
        sprintf(error,"Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
        perror(error);
        exit(2);
   }

   thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
   if (thread == NULL)
   {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(3);
   }
   taskids = (int *) malloc(NUM_THREADS * sizeof(int));
   if (taskids == NULL)
   {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(4);
    }

   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, Incremento, (void *) (&taskids[i])) != 0)  /* N.B. si deve passare il nome giusto della START_ROUTINE */
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
    }

   for (i=0; i < NUM_THREADS; i++)
   {
	int ris;
        /* attendiamo la terminazione di tutti i thread generati */
   	pthread_join(thread[i], (void**) & p);
	ris= *p;
	printf("Pthread %d-esimo restituisce %d\n", i, ris);
   }
 
   exit(0);
}
