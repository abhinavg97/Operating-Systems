#include<iostream>
#include<fstream>
#include<thread>
#include<ctime>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include <atomic>
#include<vector>

using namespace std;


int n,k;
atomic_flag lock = ATOMIC_FLAG_INIT;
unsigned int csSeed, remSeed;
FILE *fp;
time_t now,start,end1;
tm *ltm;
vector<int> waiting;  // vector array of n threads
int wait = 0;

void testCS(int id)
{

	bool key;
	for(int i=1; i<=k ;i++)
	{
		waiting[id] = 1;  
		key = true;
		now = time(0);
		ltm = localtime(&now);
		
		fprintf(fp,"%dth CS request by Thread %d at %d:%d:%d\n",i,id+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		start  = time(NULL);

        while(waiting[id] && key) // id is from o to n-1 inclusive
        	key = atomic_flag_test_and_set_explicit(&lock, memory_order_acquire);

        waiting[id] = 0;
		/*end1 of custom code*/

		now = time(0);
		ltm = localtime(&now);
		end1 = time(NULL);
		fprintf(fp, "%dth CS Entry by Thread %d  at %d:%d:%d\n",i,id+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		wait += (int)(end1 - start);
		srand(csSeed);
		int randCSTime = rand()%100; // get random CS Time
		usleep(randCSTime); // simulate a thread executing in CS

		now = time(0);
		ltm = localtime(&now);
		fprintf(fp, "%dth CS Exit by Thread %d at %d:%d:%d\n",i,id+1, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		/*
		* Your code for the thread to exit the CS.
		*/
		int j = (id+1) %n;
		while((j != id) && waiting[j] == 0 )
			j = (j+1)%n;

		if (j==id)
			atomic_flag_clear_explicit(&lock, memory_order_release); // release the lock
		else 
			waiting[j] = 0;
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
	fp = fopen ("TAS_bounded-log.txt","a");

	if(in.is_open())
	{
		in>>n>>k>>csSeed>>remSeed;
	    thread myThreads[n];

	    for (int i=0; i<n; i++)
	    {
	    	waiting.push_back(0);
	    	myThreads[i] = thread(testCS,i); 
	    }

	    for (int i=0; i<n; i++)
	    	myThreads[i].join();
	}
	else
		cout<<"Error opening input file!\n"<<flush;

	fclose(fp);
	
	FILE *fd;
	fd = fopen("Average-time.txt","a");

	fprintf(fd,"Average for bounded wait = %f\n",wait/(n*k*1.0));
	return 0;
}


