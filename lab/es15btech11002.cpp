#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <math.h>
#include <cmath>
#include <cstring>
#include <assert.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <sys/mman.h>
#include <math.h>
#include <cmath>
#include <cstring>
#include <assert.h>
#include <mutex>
#include<iostream>
#include<iostream>
#include<fstream>
#include<thread>
#include<ctime>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>
#include <atomic>
#include<random>

using namespace std;

mutex mtx, tribemtx;

FILE *fd;
time_t now,start,end1;
tm *ltm;

int totcount = 0;
double wait = 0;

int k,m,n,as,bs,ls,mu,food;
int notified = 0;
double randCSTime;

void notify()
{
	mtx.unlock(); // unlock, gets locked by cook 
	notified = 1;
}

double ran_exp(float lam)
{
	default_random_engine generate;
	exponential_distribution<double> distribution(1.0/lam);
	return distribution(generate);
}

void makefood()
{
	while(1)
	{
		if(totcount == k*n)
			break;//done, no more cooking please

		now = time(0);
		ltm = localtime(&now);
		fprintf(fd,"cook checking checking pot at %d:%d:%d\n",1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);

		mtx.lock();

		if(food == 0)
		{
			now = time(0);
			ltm = localtime(&now);
			fprintf(fd,"cook cooking food at %d:%d:%d\n",1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);


			randCSTime = ran_exp(ls); // get random CS Time
			usleep(randCSTime*1000); // simulate a thread executing in CS

			food = m;

			now = time(0);
			ltm = localtime(&now);
			fprintf(fd,"cook cooked food at %d:%d:%d\n",1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);

			if(notified == 0)
			{
				mtx.unlock(); // cook cooking on his own
			}
			else
			{
				notified = 0; // cooking when notified, dont unock tribe guy will unlock
			}
		}
		else
		{
			if(notified == 1)
				printf("shouldnt have happened\n");
			mtx.unlock();
		}


		
		randCSTime =  ran_exp(mu); // get random CS Time

		int sleepcounter = randCSTime;
		while(1)
		{
			if(sleepcounter>0)
			{	
				sleepcounter--;
				usleep(1000); // simulate a thread executing in CS
			}
			else
				break;
			if(notified == 1)
			{
				break; // get out and cook
			}
		}

	}

}

void getfood(int id)
{
	for(int i=1; i<=n ;i++)
	{
		now = time(0);
		ltm = localtime(&now);
		fprintf(fd,"Tribesman number %d becomes hungry at %d:%d:%d\n",id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		start = time(NULL);

		// put a lock here
		//wait

		tribemtx.lock();// no other tribe person 
		mtx.lock();
		totcount++;

		if(food==0)	
			notify(); // mutex gets unlocked and locked by cook which we dont unlock again, this person unlocks

		while(notified == 1)
			;  // wait till cook cooks

		food--;
		/*end1 of custom code*/
		now = time(0);
		ltm = localtime(&now);
		end1 = time(NULL);
		wait += (int) (end1 - start);
		printf("%f\n", wait);
		fprintf(fd,"Tribesman number %d eats from the pot at %d:%d:%d\n",id, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);


		randCSTime = ran_exp(as);; // get random CS Time
		// printf("%f", randCSTime);
		usleep(randCSTime*1000); // simulate a thread executing in CS

		now = time(0);
		ltm = localtime(&now);
		fprintf(fd,"Tribesman number %d has eaten %d times from the pot at %d:%d:%d\n",id, i, 1 + ltm->tm_hour, 1 + ltm->tm_min, 1 + ltm->tm_sec);
		
		mtx.unlock();
		tribemtx.unlock();
		
		/*
		* Your code for the thread to exit the CS.
		*/

		randCSTime = ran_exp(bs); // get random CS Time
		usleep(randCSTime*1000); // simulate a thread executing in CS

		/*end1 of exit*/
	}

}


int  main()
{
	// create 'k' testCS threads
	ifstream in("inp-params.txt");
	
	// write stats to file
	fd = fopen ("log.txt","w+");

	if(in.is_open())
	{
		// read the variables from here
		in>>k>>m>>n>>as>>bs>>ls>>mu;

		food = m; // initial food
	    thread myThreads[k+1];  // n threads
	  
	    for (int i=0; i<k; ++i)
	    	myThreads[i] = thread(getfood,i+1); 

	    myThreads[k] = thread(makefood);

	    for(int i=0; i<k+1; ++i)
	    	myThreads[i].join();
	}

	else
		cout<<"Error opening input file!\n"<<flush;

	fclose(fd);
	FILE *fd;
	fd = fopen("Average_time.txt","a");

	fprintf(fd, "Average wait for %d threads = %f\n",k, wait*1.0/k);
	fclose(fd);
	return 0;
}