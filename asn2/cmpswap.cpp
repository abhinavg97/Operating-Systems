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
int wait = 0;
atomic<int> lock ;
unsigned int csSeed, remSeed;
FILE *fp;
time_t now,start,end1;
tm *ltm;



void testCS(int id)
{
	for(int i=1; i<=k ;i++)
	{
		now = time(0);
		ltm = localtime(&now);
		fprintf(fp,"%dth CS request by Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		start = time(NULL);
		int expected ,new_val;
		expected = 0;  // setting expected and new_val as given in the book
		new_val = 1;

		// this function returns true only if lock value is 0, we want the condition of while loop to be false when lock is 0
		while(!lock.compare_exchange_strong(expected,new_val)) 
			expected = 0, new_val = 1;  // since this function changes the value of expected val, reinitializing it
		
		/*end1 of custom code*/
		now = time(0);
		ltm = localtime(&now);
		end1 = time(NULL);
		wait += (int) (end1 - start);
		fprintf(fp, "%dth CS Entry by Thread %d  at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);

		srand(csSeed);
		int randCSTime = rand()%100; // get random CS Time
		usleep(randCSTime); // simulate a thread executing in CS

		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Exit by Thread %d at %d:%d:%d\n",i,id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		/*
		* Your code for the thread to exit the CS.
		*/
		lock = 0; // setting lock 0 upon exit
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
	
	fp = fopen ("CAS-log.txt","w+");

	if(in.is_open())
	{
		in>>n>>k>>csSeed>>remSeed;
	    thread myThreads[n];

	    lock = 0;  // initializing the lock to false
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

	fprintf(fd,"Average for compare and swap = %f\n",wait/(n*k*1.0));
	return 0;
}
