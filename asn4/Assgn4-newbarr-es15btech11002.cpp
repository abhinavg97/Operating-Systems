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
#include<vector>

using namespace std;

int n,k,prel,postl;
FILE *fp;
time_t now,start,end1;
tm *ltm;

int counter;

sem_t mutex, turnstile, turnstile2;

vector<float> wait;

void barrier_point()
{

	sem_wait(&mutex);

	counter++;

	if(counter == n)
	{
		sem_wait(&turnstile2);
		sem_post(&turnstile);
	}

	sem_post(&mutex);

	sem_wait(&turnstile);
	sem_post(&turnstile);

	sem_wait(&mutex);

	counter--;

	if(counter == 0)
	{
		sem_wait(&turnstile);
		sem_post(&turnstile2);
	}
	sem_post(&mutex);

	sem_wait(&turnstile2);
	sem_post(&turnstile2);

}

void init()
{
	// initialize the attributes
	counter = 0;
	sem_init(&turnstile, 0, 0);
	sem_init(&turnstile2, 0, 1);
	sem_init(&mutex, 0, 1);
	wait.resize(n+1, 0);
}


void testBarrier(int id)
{
	start = time(NULL);
	for(int i = 0; i<k; ++i)
	{
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "threadid = %d  Going to sleep before the %dth barrier invocation at time: %d:%d:%d\n",id,i+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		// Simulate thread doing some time consuming task before the barrier
		srand(prel);
		int beforeBarrSleep = rand()%100000; 
		usleep(beforeBarrSleep); 

		////////////////////////////////////////////////////
		// implementation	
		//////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "threadid = %d Before the %dth barrier invocation at time: %d:%d:%d\n",id,i+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		barrier_point();

		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "threadid = %d After the %dth barrier invocation at time: %d:%d:%d\n",id,i+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "threadid = %d  Going to sleep after the %dth barrier invocation at time: %d:%d:%d\n",id,i+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		// Simulate thread doing some time consuming task after the barrier
		srand(postl);
		int afterBarrSleep = rand()%100000; 
		usleep(afterBarrSleep); 
	}
	end1 = time(NULL);

	wait[id] = (float)(end1 - start);
}

int  main()
{
	// create 'n' testCS threads
	ifstream in("inp-params.txt");
	
	fp = fopen ("new-barr-log.txt","w+");

	if(in.is_open())
	{
		in>>n>>k>>prel>>postl;
	    thread myThreads[n];

	    init();
	  
	    for (int i=0; i<n; ++i)
	    	myThreads[i] = thread(testBarrier,i+1); 

	    for(int i=0; i<n; ++i)
	    	myThreads[i].join();
	}

	else
		cout<<"Error opening input file!\n"<<flush;

	fclose(fp);
	FILE *fd;
	int totalwait = 0;
	fd = fopen("Average_time.txt","a");
	// fprintf(fd,"Average for rw (nw = %d,nr = ,%d,kw = %d,kr = %d) = %f\n",nw, nr, kw, kr, wait/(nw*kw*1.0 + nr*kr*1.0));
	fprintf(fd, "\nNew barr n = %d, k = %d\n",n,k);
	for(int i = 1; i<=n;++i)
		fprintf(fd,"Thread id:%d Time = %f\n", i, wait[i]), totalwait += wait[i];

	fprintf(fd, "Average wait for n threads = %f\n", totalwait*1.0/n);
	fclose(fd);
	return 0;

}