/* OBIETTIVO: generare un numero non noto di threads che eseguono il codice corrispondente al PROBLEMA DEL BARBIERE ADDORMENTATO: un thread sara' il barbiere e gli altri saranno i clienti.
 * Prima viene creato il barbiere (che esegue un ciclo senza fine) e poi vengono creati i clienti in numero uguale al numero passato come parametro - 1.
 * Il taglio della barba o dei capelli e' stato simulato con una sleep.
 * Se un cliente non ha trovato posto ritenta in un altro momento: quindi il cliente effettua un ciclo fino a che la sua richiesta di taglio barba/capelli non e' stata soddisfatta! 
 * Ogni thread cliente torna al main il proprio numero d'ordine. */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define N 5

/* variabili globali */
pthread_mutex_t MUTEX = PTHREAD_MUTEX_INITIALIZER; 	/* semaforo binario per la mutua esclusione nell'accesso alla variabile introdotta */
sem_t BARBER;	 					/* valore iniziale uguale a 0 ==> semaforo privato per il thread barbiere */
sem_t CUSTOMERS; 					/* valore iniziale uguale a 0 ==> semaforo privato per i thread clienti */
int FreeSeats = N;					/* numero di sedie nella sala di attesa del negozio del barbiere */

void *eseguiBarbiere(void *id)
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

   printf("BARBIERE-[Thread%d e identificatore %lu] STO APRENDO IL NEGOZIO\n", *pi, pthread_self());

   for (i = 0; ; i++) /* while (true)  il barbiere DEVE essere un ciclo senza fine */
   {
   	sem_wait(&CUSTOMERS);
	printf("BARBIERE-[Thread%d e identificatore %lu] SONO SVEGLIO (iter. %d)\n", *pi, pthread_self(), i);
	/* il barbiere puo' prendere in carico un cliente: questo viene rappresentato dal fatto che, facendo sedere il cliente sulla sedia del barbiere, si libera un posto nella sala di attesa e chiaramente si va a risvegliare uno dei clienti in attesa */ 
	pthread_mutex_lock(&MUTEX);
        /* inizio sezione critica */
        FreeSeats++;
	/* risveglio di uno dei clienti secondo la politica FIFO */ 
 	sem_post(&BARBER);
	/* fine sezione critica */
        pthread_mutex_unlock(&MUTEX);
	printf("BARBIERE-[Thread%d e identificatore %lu] STO PER TAGLIARE BARBA/CAPELLI (iter. %d)\n", *pi, pthread_self(), i);
   	sleep(5); /* simuliamo il taglio della barba o dei capelli */
	printf("BARBIERE-[Thread%d e identificatore %lu] HO TERMINATO DI TAGLIARE BARBA/CAPELLI (iter. %d)\n", *pi, pthread_self(), i);
   }

   /* ESSENDO UN CICLO SENZA FINE NON SI ARRIVERA' MAI QUI!*/
   *ptr = *pi;
   pthread_exit((void *) ptr);
}

void *eseguiCliente(void *id)
{
   int *pi = (int *)id;
   int *ptr;
   int taglio=1; 	/* variabile locale ad ogni cliente che serve per sapere se un cliente e' riuscito ad usufruire dei servizi del barbiere */
   int i;

   ptr = (int *) malloc( sizeof(int));
   if (ptr == NULL)
   {
        perror("Problemi con l'allocazione di ptr\n");
        exit(-1);
   }

   printf("CLIENTE-[Thread%d e identificatore %lu] STO ANDANDO DAL BARBIERE\n", *pi, pthread_self());

   for (i = 0; taglio; i++) /* il cliente vuole assolutamente tagliarsi barba/capelli: quindi impostiamo un ciclo fino a che taglio non diventa 0! */
   {
        /* il cliente deve verificare se c'e' posto nel negozio del barbiere */
        pthread_mutex_lock(&MUTEX);
        /* inizio sezione critica */
        if (FreeSeats > 0)
        {
                FreeSeats--;
                /* il cliente prova a svegliare il barbiere se e' addormentato, altrimenti aspetta che il barbiere sia libero */
                sem_post(&CUSTOMERS);
                /* fine sezione critica */
                pthread_mutex_unlock(&MUTEX);
                sem_wait(&BARBER);
                printf("CLIENTE-[Thread%d e identificatore %lu] SONO SULLA SEDIA DEL BARBIERE (iter. %d)\n", *pi, pthread_self(), i);
		taglio = 0; /* si resetta il valore di taglio perche' il cliente e' stato servito */
        }
        else
        {
                /* fine sezione critica */
                pthread_mutex_unlock(&MUTEX);
                printf("CLIENTE-[Thread%d e identificatore %lu] NON HO TROVATO POSTO E QUINDI VADO A CASA (iter. %d)\n", *pi, pthread_self(), i);
        }
        printf("CLIENTE-[Thread%d e identificatore %lu] ARRIVATO A CASA (iter. %d)\n", *pi, pthread_self(), i);
        sleep(5); /* simuliamo un'attesa prima di tornare dal barbiere per riprovare nel caso non si sia riuscito ad usufruire dei servizi del barbiere */
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

   /* prima di creare i thread, andiamo ad inizializzare il semaforo BARBER al valore 0 */
   if (sem_init(&BARBER, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo BARBER\n");
        exit(5);
   }

   /* prima di creare i thread, andiamo ad inizializzare il semaforo COSTUMERS al valore 0 */
   if (sem_init(&CUSTOMERS, 0, 0) != 0)
   {
        perror("Problemi con l'inizializzazione del semaforo COSTUMERS\n");
        exit(6);
   }

   /* creiamo prima il barbiere e poi i clienti */
   for (i=0; i < NUM_THREADS; i++)
   {
        taskids[i] = i;
	if (i == 0)
	{
   		printf("Sto per creare il thread BARBIERE %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiBarbiere, (void *) (&taskids[i])) != 0)
                {
                        sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread BARBIERE %d-esimo\n", taskids[i]);
                        perror(error);
                        exit(7);
                }
	}
	else
	{
   		printf("Sto per creare il thread CLIENTE %d-esimo\n", taskids[i]);
        	if (pthread_create(&thread[i], NULL, eseguiCliente, (void *) (&taskids[i])) != 0)
                {
                        sprintf(error,"SONO IL MAIN E CI SONO STATI PROBLEMI NELLA CREAZIONE DEL thread CLIENTE %d-esimo\n", taskids[i]);
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
		printf("Dato che il barbiere è un ciclo infinito NON possiamo aspettarlo\n");
		/* si ricorda che all'exit del main si produrra' la terminazione comunque del thread BARBIERE */
	else
   	{
		pthread_join(thread[i], (void**) & p);
		ris= *p;
		printf("CLIENTE-Pthread %d-esimo restituisce %d\n", i, ris);
        }
   }

   exit(0); /* quando il thread main termina, termina anche il thread barbiere! */
}
