/* OBIETTIVO: generare un thread tabaccaio e tre threads fumatori che eseguono il codice corrispondente al PROBLEMA DEI FUMATORI DI SIGARETTE.
 * I threads fumatori eseguono un ciclo senza fine, mentre il thread tabaccaio esegue NTIMES iterazioni.
 * Il tabaccaio usa una funzione random, per stabilire quali dei due 'ingredienti' deve mettere sul tavolo: fare attenzione che l'uso di tale funzione richiede l'inizializzazione del seme. */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define NTIMES 10
#define NUM_THREADS 4	/* numero di threads costante: 1 tabaccaio e 3 fumatori */
#define MATCHES 1	/* costante che individua il fumatore che ha i fiammiferi */
#define TOBACCO 2	/* costante che individua il fumatore che ha il tabacco */   
#define PAPER 3		/* costante che individua il fumatore che ha la cartina */  

/* variabili globali */
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER; 	/* semaforo binario per la mutua esclusione nell'accesso al tavolo */		
sem_t SEM[NUM_THREADS];	 				/* array di semafori privati (valore iniziale uguale a 0): il primo lo usiamo per il tabaccaio e gli altri per i fumatori */

int mia_random(int n)
{
int casuale;
casuale = rand() % n;
casuale++;		/* si incrementa dato che la rand produce un numero random fra 0 e n-1, mentre a noi serve un numero fra 1 e n */
return casuale;
}

void *eseguiTabaccaio(void *id)
{
   int *pi = (int *)id;		/* N.B. l'indice di creazione del thread determina lo specifico fumatore: si veda valore delle costanti appositamente definite */
   int *ptr;
   int i;
   int r;                    	/* variabile usata per calcolare i valori random */

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   srand(time(NULL)); /* inizializziamo il seme per la generazione random di numeri  */
   printf("TABACCAIO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

   for (i = 0; i < NTIMES ; i++) /* while (true)  il tabaccaio dovrebbe essere un ciclo senza fine */
   {
	pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
	r = mia_random(3); 	/* viene generato un numero random che individua quali due 'ingredienti' il tabaccaio pone sul tavolo */
	if (r == MATCHES)
	       printf("TABACCAIO-[generato %d per l'iter. %i]: Metto sul tavolo il tabacco e la cartina, quindi posso risvegliare il fumatore che ha i fiammiferi\n", r, i);
	else 
		if (r == TOBACCO)
	       		printf("TABACCAIO-[generato %d per l'iter. %i]: Metto sul tavolo la cartina e i fiammiferi, quindi posso risvegliare il fumatore che ha il tabacco\n", r, i);
		else 	/* r == PAPER */ 
			printf("TABACCAIO-[generato %d per l'iter. %i]: Metto sul tavolo il tabacco e i fiammiferi, quindi posso risvegliare il fumatore che ha la cartina\n", r, i);
	/* si deve segnalare al fumatore 'giusto' */
 	sem_post(&SEM[r]);
   	/* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX);

	/* si deve aspettare che servano nuovi 'ingredienti' */
 	sem_wait(&SEM[0]);
   }

   *ptr = *pi;
   pthread_exit((void *) ptr);
}

void *eseguiFumatore(void *id)
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

   printf("FUMATORE-[Thread%d e identificatore %lu] VOGLIO FUMARE\n", *pi, pthread_self());

   for (i = 0; ; i++) /* while (true)  il fumatore DEVE essere un ciclo senza fine */
   {
   	sem_wait(&SEM[*pi]);	/* il fumatore aspetta che il tabaccaio abbia messo sul tavolo i due 'ingredienti' che gli mancano per poi fumare */
	pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
	if (*pi == MATCHES)
	       printf("FUMATORE-MATCHES: Prendo dal tavolo il tabacco e la cartina, quindi posso confezionare la sigaretta\n");
	else 
		if (*pi == TOBACCO)
	       		printf("FUMATORE-TOBACCO: Prendo dal tavolo la cartina e i fiammiferi, quindi posso confezionare la sigaretta\n");
		else 	/* r == PAPER */ 
			printf("FUMATORE-PAPER: Prendo dal tavolo il tabacco e i fiammiferi, quindi posso confezionare la sigaretta\n");
	/* si deve segnalare al tabaccaio che sono stati presi i due 'ingredienti' dal tavolo e che ne vanno messi altri sul tavolo */
 	sem_post(&SEM[0]);
   	/* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX);
	sleep(5); /* simula l'azione di fumare */
   }

   /* ESSENDO UN CICLO SENZA FINE NON SI ARRIVERA' MAI QUI!*/
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

   /* prima di creare i thread, andiamo ad inizializzare i semafori privati, tutti al valore 0 */
   for (i=0; i < NUM_THREADS; i++)
   {
   	if (sem_init(&SEM[i], 0, 0) != 0)
   	{
        	sprintf(error, "Problemi con l'inizializzazione del semaforo %d-iesimo\n", i);
        	perror(error);
        	exit(4);
   	}
   }

   /* creiamo prima il tabaccaio e poi i fumatori */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
	if (i == 0)
	{
   		printf("Sto per creare il thread TABACCAIO %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiTabaccaio, (void *) (&taskids[i])) != 0)
	        {
	                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread TABACCAIO %d-esimo\n", taskids[i]);
        	        perror(error);
                	exit(5);
        	}
	}
	else
	{
   		printf("Sto per creare il thread FUMATORE %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiFumatore, (void *) (&taskids[i])) != 0)
	        {
	                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread FUMATORE %d-esimo\n", taskids[i]);
        	        perror(error);
                	exit(6);
        	}
	}

	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   /* tutti i fumatori eseguono cicli infiniti, quindi il main non li aspetta */

   for (i=0; i < NUM_THREADS; i++)
   {
	if (i == 0) /* si aspetta solo il TABACCAIO */
   	{
		int ris;
		pthread_join(thread[i], (void**) & p);
		ris= *p;
		printf("TABACCAIO-Pthread %d-esimo restituisce %d\n", i, ris);
   	}
   }

   exit(0); /* quando il thread main termina, terminano anche i fumatori */
}
