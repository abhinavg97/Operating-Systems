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
#include <sys/mman.h>
#include<vector>
#include"libmymem.hpp"
#include<mutex>
#include<errno.h>

using namespace std;

// The thread iterates for niterations and allocates objects of various sizes.
void threadmain(vector<int> &objsizelist, int niterations)
{
	// printf("niters %d\n", niterations);
	char *a;
	for(int i = 0;i<niterations; ++i)
	{
		// random pick obj_size from objsizelist;
		int obj_index = rand()%objsizelist.size();
		int obj_size = objsizelist[obj_index];

		//invoke mymalloc, bail if error
		a = (char*)mymalloc(obj_size);
		if((void*)a == MAP_FAILED)
		{
			printf("MAP FAILED!\n");
			return ;
		}


		// write to allocated mem
		*a = 'a';

		// sleep for small time
		usleep(100);
		myfree(a);
	}
}

int main(int argc, char *argv[])
{
	// read nthreads and niterations from terminal
	int nthreads = atoi(argv[1]);
	int niterations = atoi(argv[2]);
	thread myThreads[nthreads];

	// create a list of object sizes for this iththread
	vector<int> objsizelist;

	for(int i=1;i<8193;++i)
		objsizelist.push_back(i);


	
	// Create nthreads to do allocations/free concurrently
	for(int i = 0; i < nthreads; ++i)
		// Create a thread
		myThreads[i] = thread(threadmain,ref(objsizelist), ref(niterations) );
	
	// wait for all threads to join
	for(int i = 0; i<nthreads; ++i)
		myThreads[i].join();

	


	return 0;
}