/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente al PROBLEMA DEL CUOCO e dei CANNIBALI.
 * Prima viene creato il cuoco (che esegue un ciclo senza fine) e poi vengono creati gli N cannibali in numero uguale al numero passato come parametro - 1.
 * Il riempimento della pentola da parte del cuoco e' stato simulato con una sleep, cosi' come il mangiare la porzione da parte di un cannibale.
 * Ogni thread cannibale torna al main il proprio numero d'ordine. */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define M 5

/* variabili globali */
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER; 	/* semaforo binario per la mutua esclusione nell'accesso alle variabili comuni introdotte */
sem_t PIENA;	 					/* valore iniziale uguale a 0 ==> semaforo privato per i thread cannibali */
sem_t VUOTA;	 					/* valore iniziale uguale a 0 ==> semaforo privato per il thread cuoco */
int porzioni = M;					/* numero di porzioni presenti nella pentola: all'inizio si suppone che ce ne siano il massimo possibile */

void *eseguiCuoco(void *id)
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

   printf("CUOCO-[Thread%d e identificatore %lu] STO ARRIVANDO\n", *pi, pthread_self());

   for (i = 0; ; i++) /* while (true)  il cuoco DEVE essere un ciclo senza fine */
   {
	/* il cuoco si sospende se la pentola e' ancora piena */   
   	sem_wait(&VUOTA);
	printf("CUOCO-[Thread%d e identificatore %lu] STO RIEMPIENDO LA PENTOLA (iter. %d)\n", *pi, pthread_self(), i);
   	sleep(5); /* simuliamo il riempimento della pentola */
        /* NOTA BENE: il cuoco viene svegliato mentre e' acquisito il mutex da parte del cannibale che non ha trovato cibo e quindi la sezione critica rappresentata dalla modifica della variabile porzioni e' gia' protetta */
	porzioni = M; 	/* aggiorniamo il numero di porzioni */ 
	/* risveglio del cannibale che ha risvegliato il cuoco */ 
 	sem_post(&PIENA);
   }

   /* ESSENDO UN CICLO SENZA FINE NON SI ARRIVERA' MAI QUI!*/
   *ptr = *pi;
   pthread_exit((void *) ptr);
}

void *eseguiCannibale(void *id)
{
   int *pi = (int *)id;
   int *ptr;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   printf("CANNIBALE-[Thread%d e identificatore %lu] HO FAME\n", *pi, pthread_self());

   /* il cannibale deve verificare se c'e' una porzione nella pentola */
   pthread_mutex_lock(&MUTEX);
   /* inizio sezione critica */
   if (porzioni == 0)
   {
        /* il cannibale sveglia il cuoco e aspetta che venga riempita la pentola */
        sem_post(&VUOTA);
        sem_wait(&PIENA); /* NOTA BENE: il cannibale NON rilascia la sezione critica */
   }
   /* il cannibale puo' mangiare a quindi decrementa il numero di porzioni */
   porzioni--;
   printf("CANNIBALE-[Thread%d e identificatore %lu] STO MANGIANDO\n", *pi, pthread_self());
   /* fine sezione critica */
   pthread_mutex_unlock(&MUTEX);
   sleep(5); /* simuliamo il mangiare */
   printf("CANNIBALE-[Thread%d e identificatore %lu] VA A DORMIRE DOPO MANGIATO\n", *pi, pthread_self());
   /* si noti che non essendo nella formulazione previsto un ciclo infinito per i cannibali, non e' stato neanche previsto un ciclo iterativo */
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
   int NUM_THREADS; /* N del testo dell'esercizio corrisponde a NUM_THREADS - 1 */
   char error[250];

   /* Controllo sul numero di parametri */
   if (argc != 2 ) /* Deve essere passato esattamente un parametro */
   {
   	sprintf(error, "Errore nel numero dei parametri %d\n", argc-1);
        perror(error);
        exit(1);
   }

   /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
   NUM_THREADS = atoi(argv[1]);
   if (NUM_THREADS <= 0) 
   {
   	sprintf(error, "Errore: Il primo parametro non e' un numero strettamente maggiore di 0, infatti e' %d\n", NUM_THREADS);
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

   /* prima di creare i thread, andiamo ad inizializzare il semaforo PIENA al valore 0 */
   if (sem_init(&PIENA, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo PIENA\n");
        exit(5);
   }

   /* prima di creare i thread, andiamo ad inizializzare il semaforo VUOTA al valore 0 */
   if (sem_init(&VUOTA, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo VUOTA\n");
        exit(6);
   }

   /* creiamo prima il cuoco e poi i cannibli */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
	if (i == 0)
	{
   		printf("Sto per creare il thread CUOCO %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiCuoco, (void *) (&taskids[i])) != 0)
       		{
			sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CUOCO %d-esimo\n", taskids[i]);
                	perror(error);
                	exit(7);
		}
	}
	else
	{
   		printf("Sto per creare il thread CANNIBALE %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiCannibale, (void *) (&taskids[i])) != 0)
                {
                        sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CANNIBALE %d-esimo\n", taskids[i]);
                        perror(error);
                        exit(8);
                }
	}

	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (i=0; i < NUM_THREADS; i++)
   {
	int ris;
	if (i == 0)
		printf("Dato che il cuoco è un ciclo infinito NON possiamo aspettarlo\n");
		/* si ricorda che all'exit del main si produrra' la terminazione comunque del cuoco */
	else
   	{
		pthread_join(thread[i], (void**) & p);
		ris= *p;
		printf("CANNIBALE-Pthread %d-esimo restituisce %d\n", i, ris);
        }
   }

   exit(0); /* quando il thread main termina, termina anche il thread cuoco! */
}
