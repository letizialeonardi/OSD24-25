/* OBIETTIVO: generare i 5 thread che rappresentano i filosofi: PROBLEMA DEI FILOSOFI RISOLTO CON UNA SOLUZIONE CHE USA UN MUTEX E UN ARRAY DI VARIABILI ONDIZIONE. 
 * La fase in cui il filosofo mangia e' stata simulata con una sleep mentre quella in cui pensa e' stata simulata con un descheduling.
 * Ogni thread torna al main il proprio numero d'ordine. */
#define _GNU_SOURCE             /* Per non avere warning utilizzando la pthread_yield - See feature_test_macros(7) */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define NTIMES 10

typedef enum {false, true} Boolean;
typedef enum {thinking, hungry, eating} Stati;

/* variabili globali */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; /* semaforo di mutua esclusione per l'accesso a tutte le variabili condivise (simula il semaforo di mutua esclusione associato ad una istanza di tipo monitor) */
Stati STATO[5] = { thinking, thinking, thinking, thinking, thinking }; /* array che mantiene lo stato di ogni filosofo: all'inizio sono tutti in stato thinking */
pthread_cond_t COND[5] = { PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }; /* array di variabili condizione ognuna con valore iniziale di default: ogni filosofo si sospende sulla sua variabile condizione se e' in stato hungry ma i due bastoncini sono occupati */

void test(int); /* funzione interna */

void Pick_up(int i)
{
	pthread_mutex_lock(&mutex); 		/* simulazione di inizio procedura entry del monitor */
	STATO[i] = hungry;
	test(i);
	if (STATO[i] != eating)		 	/* N.B. ABBIAMO LASCIATO l'if presente nella soluzione presentata a lezione */ 
		pthread_cond_wait(&(COND[i]), &mutex);
	pthread_mutex_unlock(&mutex);		/* simulazione di termine procedura entry del monitor */
}

void Put_down(int i)
{
	pthread_mutex_lock(&mutex);		/* simulazione di inizio procedura entry del monitor */
	STATO[i] = thinking;
	test((i+4)%5);
	test((i+1)%5);
	pthread_mutex_unlock(&mutex);		/* simulazione di termine procedura entry del monitor */
}

void test(int k)
{
	if (STATO[(k+4)%5] != eating && STATO[k] == hungry && STATO[(k+1)%5] != eating)
	{
		STATO[k] = eating;
		pthread_cond_signal(&(COND[k]));
	}
}

void *eseguiFilosofo(void *id)
{
   int *pi = (int  *)id;
   int *ptr;
   int i;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione dell'array ptr\n");
        exit(-1);
   }
   
   for (i = 0; i < NTIMES; i++) /* while (true)  i filosofi dovrebbero essere cicli senza fine */
   {
	Pick_up(*pi); /* tentiamo di acquisire i due bastoncini */
   	printf("FILOSOFO con indice %d e identificatore %lu ha ottenuto di poter mangiare (i=%d)\n", *pi, pthread_self(), i);
   	sleep(5); /* simuliamo l'azione di mangiare */
	Put_down(*pi); /* rilasciamo i due bastoncini */
	printf("FILOSOFO con indice %d e identificatore %lu ora pensa (i=%d)\n", *pi, pthread_self(), i);
   	pthread_yield(); /* sleep(5); simuliamo l'azione di pensare */
    }

   /* pthread passa il valore intero dell'indice */
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

   /* creazione dei thread filosofi */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread %d-esimo\n", taskids[i]);
	if (pthread_create(&thread[i], NULL, eseguiFilosofo, (void *) (&taskids[i])) != 0)
	{
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread FILOSOFO %d-esimo\n", taskids[i]);
                perror(error);
                exit(3);
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

   exit(0);
}

