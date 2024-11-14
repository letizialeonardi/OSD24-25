/* VERSIONE CON WRITE SU 1 */
/* OBIETTIVO: generare i 5 thread che rappresentano i filosofi: PROBLEMA DEI FILOSOFI RISOLTO CON UNA SOLUZIONE CHE USATA UN ARRAY DI SEMAFORI E UNA SOLUZIONE SIMMETRICA che chiaramente può portare al DEADLOCK!
 * La fase in cui il filosofo mangia e' stata simulata con una sleep mentre quella in cui pensa e' stata simulata con un descheduling.
 * Ogni thread torna al main il proprio numero d'ordine. */
#define _GNU_SOURCE             /* Per non avere warning utilizzando la pthread_yield - See feature_test_macros(7) */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define NTIMES 10

typedef enum {false, true} Boolean;

/* variabili globali */
sem_t S_BASTONCINO[5]; /* array di semafori ognuno con valore iniziale uguale a 1 */

void *eseguiFilosofo(void *id)
{
   int *pi = (int *)id;
   int *ptr;
   int i;
   char stampa[BUFSIZ];

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione dell'array ptr\n");
        exit(-1);
   }
   
   /* codice filosofo */
   sprintf(stampa, "FILOSOFO con indice %d\n", *pi);
   write(1, stampa, strlen(stampa));

   for (i = 0; i < NTIMES; i++) /* while (true)  i filosofi dovrebbero essere cicli senza fine */
   {
	sem_wait(&(S_BASTONCINO[*pi]));
	sem_wait(&(S_BASTONCINO[(*pi+1)%5]));
	sprintf(stampa, "FILOSOFO con indice %d e identificatore %lu ha ottenuto di poter mangiare (i=%d)\n", *pi, pthread_self(),i);
   	write(1, stampa, strlen(stampa));
	sleep(5); /* simuliamo l'azione di mangiare */
	sem_post(&(S_BASTONCINO[*pi]));
	sem_post(&(S_BASTONCINO[(*pi+1)%5]));
	sprintf(stampa,"FILOSOFO con indice %d e identificatore %lu ora pensa (i=%d)\n", *pi, pthread_self(), i);
   	write(1, stampa, strlen(stampa));
	pthread_yield(); /* simuliamo l'azione di pensare */
    }

   /* pthread torna al padre il valore intero dell'indice */
   *ptr = *pi;
   pthread_exit((void *) ptr);
}

int main ()
{
   pthread_t *thread;
   int *taskids;
   int i;
   int *p;
   int NUM_THREADS = 5;
   char error[250];
   char stampa[BUFSIZ];

   thread=(pthread_t *) malloc(NUM_THREADS * sizeof(pthread_t));
   if (thread == NULL)
   {
        perror("Problemi con l'allocazione dell'array thread\n");
        exit(1);
   }
   taskids = (int *) malloc(NUM_THREADS * sizeof(int));
   if (taskids == NULL)
   {
        perror("Problemi con l'allocazione dell'array taskids\n");
        exit(2);
   }

   /* prima di creare i thread, andiamo ad inizializzare i semafori, tutti al valore 1 */
   for (i=0; i < NUM_THREADS; i++)
   {
   	if (sem_init(&(S_BASTONCINO[i]), 0, 1) != 0)
   	{
        	sprintf(error, "Problemi con l'inizializzazione del semaforo %d-esimo\n", i);
		perror(error);
        	exit(3);
   	}
   }

   /* creazione dei thread filosofi */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	sprintf(stampa, "Sto per creare il thread %d-esimo\n", taskids[i]);
   	write(1, stampa, strlen(stampa));
	if (pthread_create(&thread[i], NULL, eseguiFilosofo, (void *) (&taskids[i])) != 0)
	{
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
   		write(1, stampa, strlen(stampa));
                perror(error);
                exit(4);
        }
	sprintf(stampa, "SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   	write(1, stampa, strlen(stampa));
   }

   for (i=0; i < NUM_THREADS; i++)
   {
        int ris;
        pthread_join(thread[i], (void**) & p);
        ris= *p;
        sprintf(stampa, "Pthread %d-esimo restituisce %d\n", i, ris);
   	write(1, stampa, strlen(stampa));
   }

   exit(0);
}

