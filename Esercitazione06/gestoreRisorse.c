/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente al primo esempio di uso dei semafori: GESTIONE DI UN INSIEME DI RISORSE EQUIVALENTI. 
 * Il numero di risorse equivalenti (per semplicita') e' definito dalla costante m: chiaramente, e' possibile variare tale numero a piacimento; si segnala che il caso piu' interessante e' comunque quello in cui il numero di thread sia significativamente maggiore del numero di risorse: ad esempio, 10 thread per 5 risorse. 
 * L'utilizzo della risorsa e' stato simulato con una sleep. 
 * Ogni thread torna al main il numero d'ordine della risorsa che e' stata assegnata e quindi utilizzata. */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define m 5	/* numero di risorse che puo' essere modificato a piacimento o addirittura passato come parametro al main! */

typedef enum {false, true} Boolean;

/* variabili globali */
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER;	/* semaforo binarioi (utilizzato direttamente il tipo mutex dei Pthreads) per la mutua esclusione nell'accesso all'array LIBERO */
sem_t RISORSE; 						/* semaforo contatore il cui valore iniziale dovra' essere uguale a m (cioe' quante sono le risorse equivalenti) */
Boolean LIBERO[m];					/* array di boolean: nel caso in cui m non sia una costante, andra' definito un puntatore a boolean e quindi poi allocato l'array dinamicamente */
							/* gli elementi dell'array vanno inizializzati tutti a true */

int RICHIESTA()
{
	int i;
	sem_wait(&RISORSE);				/* con questa wait si verifica che esista almeno una risorsa libera */
   	pthread_mutex_lock(&MUTEX);			/* si deve quindi entrare nella sezione critica che va a individuare una delle risorse libere (o la risorsa libera) */
   	/* inizio sezione critica */
	for (i=0; !LIBERO[i]; i++);			/* si continua il ciclo fino a che !LIBERO[i] e' true e quindi LIBERO[i] e' false e quindi la corrispondente risorsa e' gia' in uso da parte di un processo */
	/* trovato un indice i per cui LIBERO[i] e' true */
	LIBERO[i] = false;				/* segnaliamo l'occupazione della risorsa i-esima */
   	/* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX);
	return i;					/* ritorniamo l'indice della risorsa che e' stata trovata libera e quindi potra' essere usata dal processo */
}

void RILASCIO(int x)
{
        pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
	LIBERO[x] = true;				/* la risorsa x-esima deve essere dichiarata non piu' in uso */
   	/* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX);
	sem_post(&RISORSE);				/* deve essere effettuata una "signal" sul semaforo contatore RISORSE */
}

void *esegui(void *id)					/* START_ROUTINE che viene eseguita da ogni thread figlio */
{
   int *pi = (int *)id;
   int *ptr;
   int x;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   x = RICHIESTA();					/* per prima cosa il processo deve tentare di acquisire una risorsa libera */
   printf("Thread%d e identificatore %lu ha ottenuto l'uso della risorsa di indice %d\n", *pi, pthread_self(), x);
   sleep(5); 						/* simuliamo l'uso della risorsa x-esima */
   RILASCIO(x);

   /* pthread torna al padre il valore intero di x */
   *ptr = x;
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
   	sprintf(error, "Errore nel numero dei parametri %d\n", argc-1);
	perror(error);
        exit(1);
   }

   /* Calcoliamo il numero passato che sara' il numero di Pthread da creare */
   NUM_THREADS = atoi(argv[1]);
   if (NUM_THREADS <= 0) 
   {
   	sprintf(error, "Errore: Il primo parametro non e' un numero strettamente maggiore di 0 ma e' %d\n", NUM_THREADS);
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

   /* prima di creare i thread, andiamo ad inizializzare il semaforo RISORSE al valore m */
   if (sem_init(&RISORSE, 0, m) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo RISORSE\n");
        exit(5);
   }

   /* prima di creare i thread, andiamo ad inizializzare gli elementi dell'array LIBERO */
   for (i=0; i < m; i++)
  	LIBERO[i] = true;

   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, esegui, (void *) (&taskids[i])) != 0)
	{
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(6);
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
