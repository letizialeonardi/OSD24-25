/* OBIETTIVO: generare un thread 'generatore' di atomi di idrogeno e un thread 'generatore' di atomi di ossigeno corrispondente al PROBLEMA DELLE MOLECOLE DI H2O.
 * Entrambi i threads fumatori eseguono in un ciclo finito di iterazioni: il 'generatore' di atomi di idrogeno esegue 2*NTIMES iterazioni, mentre il 'generatore' di atomi di ossigeno solo NTIMES iterazioni; la fase di generazione di ogni atomo viene simulata con una sleep(2).
 * I thread usano i semafori per sincronizzare le stampe su standard output in modo che risulti una serie di NTIMES 'HHO' corrispondente a H2O */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NTIMES 10
#define NUM_THREADS 2	/* numero di threads: 1 'generatore' di atomi di idrogeno e 1 'generatore' di atomi di ossigeno */

/* variabili globali */
sem_t Hy;	 	/* semaforo per attendere la stampa di H (valore iniziale uguale a 0) */
sem_t Oxy;	 	/* semaforo per attendere la stampa di O (valore iniziale uguale a 2) */

void *eseguiIdrogeno(void *id)
{
   int *pi = (int *)id;		
   int *ptr;
   int i;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   printf("IDROGENO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

   for (i = 0; i < 2*NTIMES ; i++) /* while (true)  il 'generatore' di idrogeno dovrebbe essere un ciclo senza fine */
   {
	sleep(2); /* simula l'azione di creazione dell'atomo di idrogeno */
	/* si deve attendere che sia stato eseguito almeno un ciclo dal 'generatore' di atomi di ossigeno */
 	sem_wait(&Oxy);
   	printf("H");
	/* si deve segnalare il processo 'generatore' di atomi di ossigeno */
 	sem_post(&Hy);
   }

   *ptr = *pi;
   pthread_exit((void *) ptr);
}

void *eseguiOssigeno(void *id)
{
   int *pi = (int *)id;
   int *ptr;
   int i;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   printf("OSSIGENO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

   for (i = 0; i < NTIMES ; i++) /* while (true)  il 'generatore' di idrogeno dovrebbe essere un ciclo senza fine */
   {
	sleep(2); /* simula l'azione di creazione dell'atomo di ossigeno */
	/* si deve attendere che siano stati eseguiti almeno due cicli dal 'generatore' di atomi di idrogeno */
   	sem_wait(&Hy);	
   	sem_wait(&Hy);	
   	printf("O");
	/* si deve segnalare (due volte) il processo 'generatore' di atomi di idrogeno */
 	sem_post(&Oxy);
 	sem_post(&Oxy);
   }

   /* pthread passa il valore intero dell'indice */
   *ptr = *pi;
   pthread_exit((void *) ptr);
}

int main (int argc, char **argv)
{
   pthread_t *thread;
   int *taskids;
   int i;
   int *p;
   char error[250];

   /* Controllo sul numero di parametri */
   if (argc != 1 ) /* NON deve essere passato alcun parametro */
   {
   	sprintf(error, "Errore nel numero dei parametri %d\n", argc-1);
	perror(error);
        exit(1);
   }

   thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
   if (thread == NULL)
   {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(2);
   }
   taskids = (int *) malloc(NUM_THREADS * sizeof(int));
   if (taskids == NULL)
   {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(3);
   }

   /* prima di creare i thread, andiamo ad inizializzare il semaforo Hy a 0 */
   if (sem_init(&Hy, 0, 0) != 0)
   {
       	perror("Problemi con l'inizializzazione del semaforo Hy\n");
       	exit(4);
   }

   /* prima di creare i thread, andiamo ad inizializzare il semaforo Oxy a 2 */
   if (sem_init(&Oxy, 0, 2) != 0)
   {
       	perror("Problemi con l'inizializzazione del semaforo Oxy\n");
       	exit(5);
   }

   /* creiamo i due 'generatori' di atomi */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
	if (i == 0)
	{
   		printf("Sto per creare il thread GENERATORE DI IDROGENO %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiIdrogeno, (void *) (&taskids[i])) != 0)
                {
                        sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread GENERATORE DI IDROGENO  %d-esimo\n", taskids[i]);
                        perror(error);
                        exit(6);
                }
        }
        else
        {
                printf("Sto per creare il thread GENERATORE DI OSSIGENO %d-esimo\n", taskids[i]);
                if (pthread_create(&thread[i], NULL, eseguiOssigeno, (void *) (&taskids[i])) != 0)
                {
                        sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread GENERATORE DI OSSIGENO  %d-esimo\n", taskids[i]);
                        perror(error);
                        exit(7);
                }
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (i=0; i < NUM_THREADS; i++)
   {
	int ris;
	pthread_join(thread[i], (void**) & p);
	ris= *p;
	printf("Pthread %d-esimo restituisce %d\n", i, ris);
   }

   exit(0); /* quando il thread main termina, terminano anche i fumatori */
}
