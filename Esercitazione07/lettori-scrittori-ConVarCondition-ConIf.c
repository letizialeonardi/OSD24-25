/* OBIETTIVO: generare un numero non noto di threads che risolvono il problema LETTORI E SCRITTORI senza starvationi RISOLTO CON UNA SOLUZIONE CHE USA UN MUTEX E DUE VARIABILI CONDIZIONE.
 * Il numero di lettori e il numero di scrittori, per semplicita', e' uguale: prima vengono creati un quarto di thread classificati come lettori, poi un quarto come scrittori e poi, di nuovo, un quarto di lettori, e quindi l'ultimo quarto con scrittori. 
 * L'utilizzo della risorsa in lettura o in scrittura e' stato simulato con una sleep. 
 * Ogni thread torna al main il proprio numero d'ordine. */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {false, true} Boolean;

/* variabili globali */
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 	/* semaforo binario per la mutua esclusione nell'accesso alle variabili introdotte (simula il semaforo di mutua esclusione associato ad una istanza di tipo monitor) */
int num_lettori = 0;					/* numero lettori attivi */
Boolean occupato = false;				/* indica se la risorsa e' occupata da uno scrittore */
int lettori_bloccati = 0;				/* numero di lettori bloccati: N.B. nella soluzione presentata a lezione si era usata la funzione QUEUE su un variabile condizion! */
int scrittori_bloccati = 0;				/* numero di scrittori bloccati: N.B. nella soluzione presentata a lezione si era usata la funzione QUEUE su un variabile condizion! */
pthread_cond_t ok_lettura = PTHREAD_COND_INITIALIZER;	/* condition variable su cui si sospendono i processi lettori */
pthread_cond_t ok_scrittura = PTHREAD_COND_INITIALIZER;	/* condition variable su cui si sospendono i processi scrittori */

void Inizio_lettura()
{
   	pthread_mutex_lock(&mutex);			/* simulazione di inizio procedura entry del monitor */
	if (occupato || (scrittori_bloccati != 0)) 	/* N.B. ABBIAMO LASCIATO l'if presente nella soluzione presentata a lezione */
	{
		lettori_bloccati++;
		pthread_cond_wait(&ok_lettura, &mutex);
                lettori_bloccati--;
	}
	num_lettori++;
	pthread_cond_signal(&ok_lettura);
   	pthread_mutex_unlock(&mutex); 			/* simulazione di termine procedura entry del monitor */
}

void Fine_lettura()
{
        pthread_mutex_lock(&mutex);			/* simulazione di inizio procedura entry del monitor */
        num_lettori--;
	if (num_lettori == 0)
        	pthread_cond_signal(&ok_scrittura);
        pthread_mutex_unlock(&mutex);	 		/* simulazione di termine procedura entry del monitor */
}

void Inizio_scrittura()
{
   	pthread_mutex_lock(&mutex);			/* simulazione di inizio procedura entry del monitor */
	if (num_lettori != 0 || occupato)		/* N.B. ABBIAMO LASCIATO l'if presente nella soluzione presentata a lezione */
	{
		scrittori_bloccati++;
		pthread_cond_wait(&ok_scrittura, &mutex);
		scrittori_bloccati--;
	}
	occupato = true;
   	pthread_mutex_unlock(&mutex);			/* simulazione di termine procedura entry del monitor */
}

void Fine_scrittura()
{
        pthread_mutex_lock(&mutex);			/* simulazione di inizio procedura entry del monitor */
	occupato = false;
	if (lettori_bloccati > 0)
		pthread_cond_signal(&ok_lettura);
	else 
        	pthread_cond_signal(&ok_scrittura);
	pthread_mutex_unlock(&mutex);	 		/* simulazione di termine procedura entry del monitor */
}

void *eseguiLettura(void *id)
{
   int *pi = (int *)id;
   int *ptr;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   Inizio_lettura();
   printf("Thread%d e identificatore %lu ha ottenuto l'uso della risorsa in LETTURA\n", *pi, pthread_self());
   sleep(5); /* simuliamo l'uso della risorsa in lettura */
   Fine_lettura();

   /* pthread torna il valore intero dell'indice */
   *ptr = *pi;
   pthread_exit((void *) ptr);
}

void *eseguiScrittura(void *id)
{
   int *pi = (int *)id;
   int *ptr;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   Inizio_scrittura();
   printf("Thread%d e identificatore %lu ha ottenuto l'uso della risorsa in SCRITTURA\n", *pi, pthread_self());
   sleep(5); /* simuliamo l'uso della risorsa in scrittura */
   Fine_scrittura();

   /* pthread torna il valore intero dell'indice */
   *ptr = *pi;
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
   if (argc != 2) /* Deve essere passato esattamente un parametro */
   {
        sprintf(error, "Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
   }

   /* Calcoliamo il numero passato che sara' il numero di Pthread da creare che deve essere divisibile per 4! */
   NUM_THREADS = atoi(argv[1]);
   if (NUM_THREADS <= 0 || NUM_THREADS % 4 != 0) 
   {
   	sprintf(error, "Errore: Il primo parametro non e' un numero strettamente maggiore di 0 e non e' divisibile per 4, infatti e' %d\n", NUM_THREADS);
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

   /* per semplicità decidiamo di creare un numero uguale di lettori e di scrittori: prima un quarto di lettori, poi un quarto di scrittori e poi di nuovo */
   for (i=0; i < NUM_THREADS/4; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread LETTORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiLettura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread LETTORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS/2; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread SCRITTORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiScrittura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread SCRITTORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(6);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS*3/4; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread LETTORE %d-esimo (seconda passata)\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiLettura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread LETTORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(7);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread SCRITTORE %d-esimo (seconda passata)\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiScrittura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread SCRITTORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(8);
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
