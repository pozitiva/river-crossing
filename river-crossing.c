#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

#define PASSENGER 1
#define CAPTAIN 0		//dva moguca statusa koja putnik moze da ima


#define CAPACITY 4		//kapacitet broda


pthread_mutex_t mutex;		//mutex potreban za zakljucavanje kriticne sekcije svake niti 
int hackers = 0;		//broj koji pokazuje koliko hakera je doslo do reke 
int serfs = 0;			//broj koji pokazuje koliko inzenjera je doslo do reke 


sem_t hackerQueue;		//semafor koji pokazuje koliko hakera ceka u redu 
sem_t serfQueue;		//semafor koji pokazuje koliko inzenjera ceka u redu 

//semafori i promenljive potrebne za kreiranje barijere
sem_t turnstile1;
sem_t turnstile2;
pthread_mutex_t b_mutex;
int count = 0;

void* hackerArrives(void*);	//metoda za dolazak hakera
void* serfArrives(void*);	//metoda za dolazak inzenjera  
void board(void*, void*);	//metoda koja simulira ukrcavanje putnika 
void row(void*, void*);	//metoda koja simulira veslanje kapetana 

int main()
{
	//inicijalizacija svih kreiranih semafora     
	pthread_mutex_init(&mutex, NULL);
	sem_init(&serfQueue, 0, 0);
	sem_init(&hackerQueue, 0, 0);

	sem_init(&turnstile1, 0, 0);
	sem_init(&turnstile2, 0, 1);
	pthread_mutex_init(&b_mutex, NULL);

	pthread_t serfArray[100];	//niz niti inzenjera 
	pthread_t hackerArray[100];	//niz niti 

	for (int i = 0; i < 50; i++)
	{
		pthread_create(&serfArray[i], NULL, serfArrives, NULL);
		pthread_create(&hackerArray[i], NULL, hackerArrives, NULL);	// pravljenje niti koje simuliraju inzenjere i hakere 
	}

	for (int i = 0; i < CAPACITY; i++)
	{
		pthread_join(serfArray[i], NULL);	//spajanje niti         
		pthread_join(hackerArray[i], NULL);	//spajanje niti 
	}


	return 0;
}

void* hackerArrives()
{
	
	pthread_mutex_lock(&mutex);
	int status = PASSENGER;
	hackers += 1;

	//provera da li je dosao na red
	if (hackers == CAPACITY)
	{
		for (int i = 0; i < CAPACITY; i++)
			sem_post(&hackerQueue);
		hackers = 0;
		status = CAPTAIN;
	}
	else if (hackers == 2 && serfs >= 2)
	{
		int i;
		for (i = 0; i < 2; i++)
			sem_post(&hackerQueue);
		int j;
		for (j = 0; j < 2; j++)
			sem_post(&serfQueue);
		serfs -= 2;
		hackers = 0;
		status = CAPTAIN;
	}
	else
		pthread_mutex_unlock(&mutex);

	sem_wait(&hackerQueue);

	// barijera
	pthread_mutex_lock(&b_mutex);
	count += 1;
	if (count == CAPACITY)
	{
		sem_wait(&turnstile2);
		sem_post(&turnstile1);
	}
	pthread_mutex_unlock(&b_mutex);

	sem_wait(&turnstile1);
	sem_post(&turnstile1);

	char* Str = "Haker";
	board((void*)Str);

	pthread_mutex_lock(&b_mutex);
	count -= 1;
	if (count == 0)
	{
		sem_wait(&turnstile1);
		sem_post(&turnstile2);
	}
	pthread_mutex_unlock(&b_mutex);

	sem_wait(&turnstile2);
	sem_post(&turnstile2);

	if (status == CAPTAIN)
	{
		row((void*)Str);
		pthread_mutex_unlock(&mutex);
	}

}

void* serfArrives()
{
	
	pthread_mutex_lock(&mutex);
	int status = PASSENGER;
	serfs += 1;

	if (serfs == CAPACITY)
	{
		int i;
		for (i = 0; i < CAPACITY; i++)
			sem_post(&serfQueue);
		serfs = 0;
		status = CAPTAIN;
	}
	else if (serfs == 2 && hackers >= 2)
	{
		int i;
		for (i = 0; i < 2; i++)
			sem_post(&serfQueue);
		int j;
		for (j = 0; j < 2; j++)
			sem_post(&hackerQueue);
		hackers -= 2;
		serfs = 0;
		status = CAPTAIN;
	}
	else
		pthread_mutex_unlock(&mutex);

	sem_wait(&serfQueue);

	//barijera 
	pthread_mutex_lock(&b_mutex);
	count += 1;
	if (count == CAPACITY)
	{
		sem_wait(&turnstile2);
		sem_post(&turnstile1);
	}
	pthread_mutex_unlock(&b_mutex);

	sem_wait(&turnstile1);
	sem_post(&turnstile1);

	char* Str = "Serf";
	board((void*)Str);

	pthread_mutex_lock(&b_mutex);
	count -= 1;
	if (count == 0)
	{
		sem_wait(&turnstile1);
		sem_post(&turnstile2);
	}
	pthread_mutex_unlock(&b_mutex);

	sem_wait(&turnstile2);
	sem_post(&turnstile2);

	if (status == CAPTAIN)
	{
		row((void*)Str);
		pthread_mutex_unlock(&mutex);
	}
}

void board(void* Str)
{
	printf("%s je usao na brod\n", (char*)Str);	//na ekranu se ispisuje koji putnik se ukrcao 
}

void row(void* Str)
{
	printf("%s, kapetan, je poceo sa veslanjem\n", (char*)Str); //na ekranu se ispisuje ko od ukrcanih vesla 
}
