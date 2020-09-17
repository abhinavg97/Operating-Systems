#include<iostream>
#include<fstream>
#include<thread>
#include<ctime>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include <atomic>

using namespace std;


int n,k;
atomic_flag lock = ATOMIC_FLAG_INIT;
unsigned int csSeed, remSeed;
FILE *fp;
time_t now,start,end1;
tm *ltm;
int wait = 0;


void testCS(int id)
{


	for(int i=1; i<=k ;i++)
	{
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp,"%dth CS request by Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		

		start = time(NULL);
		// wait until lock is released
        while(atomic_flag_test_and_set_explicit(&lock, memory_order_acquire))
            ;
        // spin lock

		/*end1 of custom code*/

		end1 = time(NULL);
		wait += (int)(end1 - start);

		now = time(0);
		ltm = localtime(&now);
		
		srand(csSeed);
		int randCSTime = rand()%100; // get random CS Time within 10 seconds
		
		fprintf(fp, "%dth CS Entry by Thread %d  at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		usleep(randCSTime); // simulate a thread executing in CS, usleep for that much time

		/*
		* Your code for the thread to exit the CS.
		*/
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Exit by Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		atomic_flag_clear_explicit(&lock, memory_order_release);  // releases the lock , set value to false
		/*end1 of exit*/


		
		srand(remSeed);
		int randRemTime = rand()%100; // get random Remainder Section Time
		usleep(randRemTime); // simulate a thread executing in CS
	}

}

int main()
{
	// create 'n' testCS threads
	ifstream in("inp-params.txt");

	
	fp = fopen ("TAS-log.txt","a");

	if(in.is_open())
	{
		in>>n>>k>>csSeed>>remSeed;
	    thread myThreads[n];

	    for (int i=0; i<n; i++)
	    	myThreads[i] = thread(testCS,i+1); 
	    

	    for (int i=0; i<n; i++)
	    	myThreads[i].join();
	}
	else
		cout<<"Error opening input file!\n"<<flush;
	
	fclose(fp);
	
	FILE *fd;
	fd = fopen("Average-time.txt","a");

	fprintf(fd,"Average for test and set = %f\n",wait/(n*k*1.0));
	return 0;
}
