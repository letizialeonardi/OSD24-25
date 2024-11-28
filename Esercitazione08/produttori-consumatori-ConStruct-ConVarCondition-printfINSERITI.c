/* VERSIONE CHE STAMPA ANCHE IL VALORE DELLA VARIABILE GLOBALE inseriti PER RENDERE PIU' EVIDENTE IL COMPORTAMENTO DEI PRODUTTORI E CONSUMATORI */
/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente all'esempio di PRODUTTORI E CONSUMATORI RISOLTO CON UNA SOLUZIONE CHE USA UN MUTEX E DUE VARIABILI CONDIZIONE.
* La dimensione del buffer e' definita dalla costante N (per semplicita'). 
* Il singolo messaggio e' una struct in cui un campo simula il dato del messaggio (considerato una stringa di lunghezza 4 corrispondente al numero d'ordine del thread produttore) e un campo che indica il numero d'ordine della interazione. 
* Il numero di produttori e il numero di consumatori, per semplicita', e' uguale: prima vengono creati una meta' di thread classificati come produttori, poi un'altra meta' come consumatori: entrambi eseguono il loro codice in un ciclo con NTIMES ripetizioni */
#include <pthread.h>
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
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;     	/* semaforo di mutua esclusione per l'accesso a tutte le variabili condivise (simula il semaforo di mutua esclusione associato ad una istanza di tipo monitor) */
pthread_cond_t non_pieno = PTHREAD_COND_INITIALIZER;   	/* condition variable su cui si sospendono i processi produttori */
pthread_cond_t non_vuoto = PTHREAD_COND_INITIALIZER;   	/* condition variable su cui si sospendono i processi consumatori */
Messaggio buffer[N];  					/* per semplicita' il buffer viene definito di dimensione N, noto staticamente! */ 
int punt1 = 0, punt2 = 0;				/* punt1 viene usato dai produttori per inserire e punt2 viene usato dai consumatori per prelevare */
int inseriti = 0;					/* tiene conto di quanti messaggi sono stati inseriti nel buffer */

void DEPOSITA(Messaggio messaggio, int id)
{
	pthread_mutex_lock(&mutex);     /* simulazione di inizio procedura entry del monitor */
        printf("DEPOSITA Thread%d-valore di inseriti %d\n", id, inseriti); 	/* AGGIUNTA STAMPA DI INSERITI IN MODO DA VERIFICARE MEGLIO IL FUNZIONAMENTO */
	while (inseriti == N)  /* e' necessario per il corretto funzionamento di questa soluzione, utilizzare un while per verificare nuovamente la condizione */
		pthread_cond_wait(&non_pieno, &mutex);
	buffer[punt1] = messaggio;
/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO DALLA VARIABILE LOCALE messaggio DEL THREAD PRODUTTORE NEL BUFFER; RISULTA SCORRETTO INSERIRE NEL BUFFER SEMPLICEMENTE UN PUNTATORE AL MESSAGGIO POICHE' TALE MESSAGGIO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
	punt1 = (punt1 + 1) % N;
	inseriti++;
   	printf("PRODUTTORE-Thread%d e identificatore %lu: ho depositato dato=%s e iter=%d\n", id, pthread_self(), messaggio.dato, messaggio.iter);
	pthread_cond_signal(&non_vuoto);
	pthread_mutex_unlock(&mutex);   /* simulazione di termine procedura entry del monitor */
}

void PRELEVA(Messaggio *messaggio, int id)
{
	pthread_mutex_lock(&mutex);     /* simulazione di inizio procedura entry del monitor */
        printf("PRELEVA Thread%d-valore di inseriti %d\n", id, inseriti); 	/* AGGIUNTA STAMPA DI INSERITI IN MODO DA VERIFICARE MEGLIO IL FUNZIONAMENTO */
	while (inseriti == 0)  /* e' necessario per il corretto funzionamento di questa soluzione, utilizzare un while per verificare nuovamente la condizione */
		 pthread_cond_wait(&non_vuoto, &mutex);
        *messaggio = buffer[punt2];
	/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO NELLA VARIABILE LOCALE messaggio DEL THREAD CONSUMATORE; RISULTA SCORRETTO TORNARE SEMPLICEMENTE UN PUNTATORE ALL'ELEMENTO DEL BUFFER POICHE' TALE ELEMENTO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
        punt2 = (punt2 + 1) % N;
	inseriti--;
	printf("CONSUMATORE-Thread%d e identificatore %lu ha ricevuto dato=%s e iter=%d\n", id, pthread_self(), messaggio->dato, messaggio->iter);
	pthread_cond_signal(&non_pieno);
	pthread_mutex_unlock(&mutex);   /* simulazione di termine procedura entry del monitor */
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

	DEPOSITA(messaggio, *pi);  	/* passiamo a DEPOSITA l'indice del processo produttore in modo da fare una stampa completa */ 
/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO DALLA VARIABILE LOCALE messaggio DEL THREAD PRODUTTORE NEL BUFFER; RISULTA SCORRETTO INSERIRE NEL BUFFER SEMPLICEMENTE UN PUNTATORE AL MESSAGGIO POICHE' TALE MESSAGGIO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
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
	PRELEVA(&messaggio, *pi);	/* passiamo a DEPOSITA l'indice del processo consumatore in modo da fare una stampa completa */
	/* NOTA BENE: IL MESSAGGIO DEVE ESSERE COPIATO NELLA VARIABILE LOCALE messaggio DEL THREAD CONSUMATORE; RISULTA SCORRETTO TORNARE SEMPLICEMENTE UN PUNTATORE ALL'ELEMENTO DEL BUFFER POICHE' TALE ELEMENTO POTREBBE POI ESSERE MODIFICATO E QUINDI NON ESSERE PIU' DISPONIBILE NELLA SUA 'FORMA' ORIGINALE PER GLI SCOPI DEL CONSUMATORE! */
   	
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

   /* per semplicità decidiamo di creare un numero uguale di lettori e di scrittori: prima meta' produttori, poi meta' consumatori */
   for (i=0; i < NUM_THREADS/2; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread PRODUTTORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiProduttore, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread PRODUTTORE %d-esimo\n", taskids[i]);
                perror(error);
                exit(5);
        }
	printf("SONO IL MAIN e ho creato il Pthread %i-esimo con id=%lu\n", i, thread[i]);
   }

   for (; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
   	printf("Sto per creare il thread CONSUMATORE %d-esimo\n", taskids[i]);
        if (pthread_create(&thread[i], NULL, eseguiConsumatore, (void *) (&taskids[i])) != 0)
        {
                sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CONSUMATORE %d-esimo\n", taskids[i]);
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
