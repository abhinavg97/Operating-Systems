#include<iostream>
#include<fstream>
#include<thread>
#include<ctime>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include<atomic>
#include<semaphore.h>
#include<fcntl.h>
#include<sys/types.h>

using namespace std;

int nw,nr,kw,kr;
int wait = 0;
unsigned int csSeed, remSeed;
FILE *fp;
time_t now,start,end1;
tm *ltm;

sem_t mutex, wrt;

int counter;

void writer(int id)
{

	for(int i=0; i<kw ;i++)
	{
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp,"%dth CS request by Writer Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		start = time(NULL);

		/*
		Write your code for
		* Readers_Writers() &
		* Fair Readers_Writers()
		Using Semaphores here.
		*/
		sem_wait(&wrt);


		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Entry by Writer Thread %d  at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		end1 = time(NULL);
		wait += (int) (end1 - start);

		srand(csSeed);
		int randCSTime = rand()%1000; // get random CS Time
		usleep(randCSTime); // simulate a thread executing in CS

		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Exit by Writer Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);

		sem_post(&wrt);

		/*
		* Your code for the thread to exit the CS.
		*/

		srand(remSeed);
		int randRemTime = rand()%1000; // get random Remainder Section Time
		usleep(randRemTime); // simulate a thread executing in CS
	}
}

void reader(int id)
{

	for(int i=0; i<kr ;i++)
	{
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp,"%dth CS request by Reader Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		start = time(NULL);
		/*
		Write your code for
		* Readers_Writers()
		* Fair Readers_Writers()
		Using Semaphores here.
		*/

		sem_wait(&mutex);
		counter++;
		if(counter == 1)
			sem_wait(&wrt);
		sem_post(&mutex);

		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Entry by Reader Thread %d  at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		end1 = time(NULL);
		wait += (int) (end1 - start);

		srand(csSeed);
		int randCSTime = rand()%1000; // get random CS Time
		usleep(randCSTime); // simulate a thread executing in CS


		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Exit by Reader Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);

		sem_wait(&mutex);
		counter--;
		if(counter == 0)
			sem_post(&wrt);
		sem_post(&mutex);

		/*

		* Your code for the thread to exit the CS.
		*/
		srand(remSeed);
		int randRemTime = rand()%1000; // get random Remainder Section Time
		usleep(randRemTime); // simulate a thread executing in CS
	}
}


int main()
{
	// create 'n' testCS threads
	ifstream in("inp-params.txt");
	
	fp = fopen ("RW-log.txt","w+");

	if(in.is_open())
	{
		in>>nw>>nr>>kw>>kr>>csSeed>>remSeed;
	    thread myWriters[nw], myReaders[nr];

	    // initialize the mutexes to 1
	    counter = 0;
	    sem_init(&mutex, 0, 1);  
		sem_init(&wrt, 0, 1);
	  
	    for (int i=0; i<nw; i++)
	    	myWriters[i] = thread(writer,i+1); 
	    
	    for(int i=0;i<nr;++i)
	    	myReaders[i] = thread(reader,i+1);

	    for (int i=0; i<nw; i++)
	    	myWriters[i].join();

	    for(int i=0; i<nr; ++i)
	    	myReaders[i].join();
	}

	else
		cout<<"Error opening input file!\n"<<flush;

	fclose(fp);
	FILE *fd;
	fd = fopen("Average_time.txt","a");
	fprintf(fd,"Average for rw (nw = %d,nr = ,%d,kw = %d,kr = %d) = %f\n",nw, nr, kw, kr, wait/(nw*kw*1.0 + nr*kr*1.0));
	
	return 0;
}
