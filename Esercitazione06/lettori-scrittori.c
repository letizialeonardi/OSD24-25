/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente al secondo esempio di uso dei semafori: LETTORI E SCRITTORI senza starvation. 
 * Il numero di lettori e il numero di scrittori, per semplicita', e' uguale: prima vengono creati un quarto di thread classificati come lettori, poi un quarto come scrittori e poi, di nuovo, un quarto di lettori, e quindi l'ultimo quarto con scrittori. 
 * L'utilizzo della risorsa in lettura o in scrittura e' stato simulato con una sleep. 
 * Ogni thread torna al main il proprio numero d'ordine. */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {false, true} Boolean;

/* variabili globali */
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER; 	/* semaforo binario per la mutua esclusione nell'accesso alle variabili introdotte */
sem_t S_LETTORI; 					/* valore iniziale uguale a 0 ==> semaforo privato per i thread lettori */
sem_t S_SCRITTORI; 					/* valore iniziale uguale a 0 ==> semaforo privato per i thread scrittori */
int lettori_attivi = 0;					/* numero lettori attivi */
Boolean scrittori_attivi = false;			/* booleano che se true significa che uno scrittore e' attivo */
int lettori_bloccati = 0;				/* numero di lettoti bloccati */
int scrittori_bloccati = 0;				/* numero di scrittori bloccati */

void Inizio_lettura()
{
   	pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
	if (!scrittori_attivi && scrittori_bloccati == 0)	/* !scrittori_attivi serve verificare che non ci sia uno scrittore in accesso (garantisce la mutua esclusione fra lettori e scrittori), mentre scrittori_bloccati==0 serve per verificare che non ci siano scrittoti bloccati (garantisce l'assenza di starvation degli scrittori nei confronti dei lettori) */ 
	{
		sem_post(&S_LETTORI);				/* se le due condizioni sono verificate allora il lettore potrà passare e quindi si fa una "signal" sul semaforo primato in modo che la seguente wait sia passante */
		lettori_attivi++;				/* di conseguenza si incrementa il numero di lettori attivi */
	}
	else lettori_bloccati++;				/* se una delle due condizioni NON è verificata allora, il lettore sara' costretto a bloccarsi e quindi incrementiamo il contatore dei lettori bloccati */
   	/* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX);
	sem_wait(&S_LETTORI);					/* questa wait sarà passante o bloccante sulla base di quale strada è stata presa nel precedente if */
}

void Fine_lettura()
{
        pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
        lettori_attivi--;					/* il lettore sta uscendo e quindi decrementa il numero di lettori attivi */
	if (lettori_attivi == 0 && scrittori_bloccati > 0)	/* se e' l'ultimo lettore e se ci sono scrittori bloccati */
   	{
                scrittori_attivi = true;			/* si aggiorna il valore del booleano dato che si sta per riattivare uno scrittore */ 
                scrittori_bloccati--;				/* si decrementa il numero di scrittori bloccati */
		sem_post(&S_SCRITTORI);				/* si "segnala" uno degli scrittori */
        }
        /* fine sezione critica */
        pthread_mutex_unlock(&MUTEX);
}

void Inizio_scrittura()
{
   	pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
	if (lettori_attivi == 0 && !scrittori_attivi)		/* lettori_attivi == 0 serve a verificare che non ci siano lettori attivi (garantisce la mutua esclusione fra lettori e scrittori), mentre !scrittori_attivi serve verificare che non ci sia un altro scrittore in accesso (garantisce la mutua esclusione fra scrittori) */
	{
		sem_post(&S_SCRITTORI);				/* se le due condizioni sono verificate allora il scrittore potrà passare e quindi si fa una "signal" sul semaforo primato in modo che la seguente wait sia passante */
		scrittori_attivi = true;			/* di conseguenza si modifica il booleano */
	}
	else scrittori_bloccati++;				/* se una delle due condizioni NON è verificata allora, lo scrittore sara' costretto a bloccarsi e quindi incrementiamo il contatore dei lettori bloccati */
   	/* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX);
	sem_wait(&S_SCRITTORI);					/* questa wait sarà passante o bloccante sulla base di quale strada è stata presa nel precedente if */
}

void Fine_scrittura()
{
        pthread_mutex_lock(&MUTEX);
   	/* inizio sezione critica */
	scrittori_attivi = false;				/* lo scrittore sta uscendo e quindi si aggiorna il booleano */
	if (lettori_bloccati > 0)				/* si controlla se ci sono lettori bloccati (garantisce l'assenza di starvation dei lettori nei confronti degli scrittori) */
	do
   	{
		lettori_attivi++;				/* si incrementa il numero di lettori attivi dato che si sta per riattivare un lettore */
		lettori_bloccati--;				/* si decrementa il numero di lettori bloccati */
		sem_post(&S_LETTORI);				/* si "segnala" uno dei lettori */
        }
	while (lettori_bloccati != 0);				/* si ripete fino a che non si esaurisce coda di lettori */
	else 							/* se non ci sono lettori bloccati */
		if (scrittori_bloccati > 0)			/* ma ci sono scrittori bloccati */
 		{	
                	scrittori_attivi = true;		/* si aggiorna il valore del booleano dato che si sta per riattivare uno scrittore */
			scrittori_bloccati--;			/* si decrementa il numero di scrittori bloccati */
                	sem_post(&S_SCRITTORI);			/* si "segnala" uno degli scrittori */
        }
        /* fine sezione critica */
        pthread_mutex_unlock(&MUTEX);
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

   /* pthread torna al padre il valore intero dell'indice */
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

   /* pthread torna al padre il valore intero dell'indice */
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
   if (argc != 2 ) /* Deve essere passato esattamente un parametro */
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

   /* prima di creare i thread, andiamo ad inizializzare il semaforo S_LETTORI al valore 0 */
   if (sem_init(&S_LETTORI, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo S_LETTORI\n");
        exit(5);
   }

   /* prima di creare i thread, andiamo ad inizializzare il semaforo S_SCRITTORI al valore 0 */
   if (sem_init(&S_SCRITTORI, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo S_SCRITTORI\n");
        exit(6);
   }

   /* per semplicità decidiamo di creare un numero uguale di lettori e di scrittori: prima un quarto di lettori, poi un quarto di scrittori e poi di nuovo */
   for (i=0; i < NUM_THREADS/4; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread LETTORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiLettura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(7);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS/2; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread SCRITTORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiScrittura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(8);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS*3/4; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread LETTORE %d-esimo (seconda passata)\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiLettura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(9);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread SCRITTORE %d-esimo (seconda passata)\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiScrittura, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread %d-esimo\n", taskids[i]);
                perror(error);
                exit(10);
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
