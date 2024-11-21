/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente all'esempio di PRODUTTORI E CONSUMATORI. 
 * La dimensione del buffer e' definita dalla costante N (per semplicita'). 
 * Il singolo messaggio e' una struct in cui un campo simula il dato del messaggio (considerato una stringa di lunghezza 4 corrispondente al numero d'ordine del thread produttore) e un campo che indica il numero d'ordine della interazione. 
 * Il numero di produttori e il numero di consumatori, per semplicita', e' uguale: prima vengono creati una meta' di thread classificati come produttori, poi un'altra meta' come consumatori: entrambi eseguono il loro codice in un ciclo con NTIMES ripetizioni */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define N 5 
#define NTIMES 10

typedef enum {false, true} Boolean;
typedef struct{
        char dato[5];   /* stringa che rappresenta il vero messaggio: per semplicita' ogni messaggio viene considerato di dimensione fissa e pari ad una stringa di 4 caratteri (piu' il terminatore di stringa)! */
        int iter;   	/* valore della iterazione corrente */
                } Messaggio;

/* variabili globali */
pthread_mutex_t MUTEX1 = PTHREAD_MUTEX_INITIALIZER;     /* semaforo binario per la mutua esclusione nell'accesso alla variabile punt1 */
pthread_mutex_t MUTEX2 = PTHREAD_MUTEX_INITIALIZER;     /* semaforo binario per la mutua esclusione nell'accesso alla variabile punt2 */
sem_t spazio_disponibile;                               /* semaforo contatore con valore iniziale uguale a N (dimensione del buffer di comunicazione */
sem_t messaggio_disponibile;                            /* semaforo contatore con valore iniziale uguale a 0 */
Messaggio buffer[N];  					/* per semplicita' il buffer viene definito di dimensione N, noto staticamente! */ 
int punt1 = 0, punt2 = 0;				/* punt1 viene usato dai produttori per inserire e punt2 viene usato dai consumatori per prelevare */

void INSERIMENTO(Messaggio messaggio)
{
	buffer[punt1] = messaggio;
 /* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO DALLA VARIABILE LOCALE messaggio DEL THREAD PRODUTTORE NEL BUFFER; RISULTA SCORRETTO INSERIRE NEL BUFFER SEMPLICEMENTE UN PUNTATORE AL MESSAGGIO POICHE' TALE MESSAGGIO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL
 CONSUMATORE! */
	punt1 = (punt1 + 1) % N;
}

void PRELIEVO(Messaggio *messaggio)
{
        *messaggio = buffer[punt2];
	/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO NELLA VARIABILE LOCALE messaggio DEL THREAD CONSUMATORE; RISULTA SCORRETTO TORNARE SEMPLICEMENTE UN PUNTATORE ALL'ELEMENTO DEL BUFFER POICHE' TALE ELEMENTO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
        punt2 = (punt2 + 1) % N;
}

void *eseguiProduttore(void *id)
{
   int *pi = (int *)id;
   int *ptr;
   int i;
   Messaggio messaggio;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   for (i = 0; i < NTIMES; i++) /* while (true)  i produttori dovrebbero essere cicli senza fine */
   {
	/* produzione del messaggio */
	sprintf(messaggio.dato, "%d", *pi);
	messaggio.iter = i;

	sem_wait(&spazio_disponibile);
   	pthread_mutex_lock(&MUTEX1);
   	/* inizio sezione critica */
	INSERIMENTO(messaggio);
	/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO NELLA VARIABILE LOCALE messaggio DEL THREAD CONSUMATORE; RISULTA SCORRETTO TORNARE SEMPLICEMENTE UN PUNTATORE ALL'ELEMENTO DEL BUFFER POICHE' TALE ELEMENTO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
   	printf("PRODUTTORE-Thread%d e identificatore %lu: ho depositato dato=%s e iter=%d\n", *pi, pthread_self(), messaggio.dato, messaggio.iter);
        /* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX1);
        sem_post(&messaggio_disponibile);
    }

   /* pthread torna il valore intero dell'indice */
   *ptr = *pi;
   pthread_exit((void *) ptr);
}

void *eseguiConsumatore(void *id)
{
   int *pi = (int *)id;
   int *ptr;
   int i;
   Messaggio messaggio;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   for (i = 0; i < NTIMES; i++) /* while (true)  i produttori dovrebbero essere cicli senza fine */
   {
	sem_wait(&messaggio_disponibile);
   	pthread_mutex_lock(&MUTEX2);
   	/* inizio sezione critica */
	PRELIEVO(&messaggio);
	/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO NELLA VARIABILE LOCALE messaggio DEL THREAD CONSUMATORE; RISULTA SCORRETTO TORNARE SEMPLICEMENTE UN PUNTATORE ALL'ELEMENTO DEL BUFFER POICHE' TALE ELEMENTO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
	printf("CONSUMATORE-Thread%d e identificatore %lu ha ricevuto dato=%s e iter=%d\n", *pi, pthread_self(), messaggio.dato, messaggio.iter);
        /* fine sezione critica */
   	pthread_mutex_unlock(&MUTEX2);
        sem_post(&spazio_disponibile);
   	
	/* consumo del messaggio */
	sleep(2);
    }

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
   if (argc != 2 ) /* Deve essere passato esattamente un parametro */
   {
   	sprintf(error, "Errore nel numero dei parametri %d\n", argc-1);
	perror(error);
        exit(1);
   }

   /* Calcoliamo il numero passato che sara' il numero di Pthread da creare: deve essere un numero positivo */
   NUM_THREADS = atoi(argv[1]);
   if (NUM_THREADS <= 0 || NUM_THREADS % 2 != 0) 
   {
   	sprintf(error, "Errore: Il primo parametro non e' un numero strettamente maggiore di 0 e non e' divisibile per 2, infatti e' %d\n", NUM_THREADS);
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

   /* prima di creare i thread, andiamo ad inizializzare il semaforo spazio_disponibile al valore N */
   if (sem_init(&spazio_disponibile, 0, N) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo spazio_disponibile\n");
        exit(5);
   }

   /* prima di creare i thread, andiamo ad inizializzare il semaforo messaggio_disponibile al valore 0 */
   if (sem_init(&messaggio_disponibile, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo messaggio_disponibile\n");
        exit(6);
   }

   /* per semplicità decidiamo di creare un numero uguale di lettori e di scrittori: prima meta' produttori, poi meta' consumatori */
   for (i=0; i < NUM_THREADS/2; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread PRODUTTORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiProduttore, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread PRODUTTORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(7);
        }
	printf("SONO IL MAIN e ho creato il Pthread PRODUTTORE %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread CONSUMATORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiConsumatore, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CONSUMATORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(8);
        }
	printf("SONO IL MAIN e ho creato il Pthread CONSUMATORE %i-esimo con id=%lu\n", i, thread[i]);
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
