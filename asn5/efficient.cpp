#include "libmymem.hpp"
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

using namespace std;

#define SZ (1024*64)

#define TOTOBJS 16384

mutex head[12];
mutex read_lock;


// slab structure
struct slab
{
	int totobj;
	int freeobj;
	bool bitmap[TOTOBJS];
	struct bucket *bucket;
	struct slab *nxtslab;
	struct slab *prevslab;
	mutex safe; // for node wise safety

};

// object structure
struct object
{
	struct slab *slab;
};

// bucket structure
struct bucket
{
	int objsz;
	struct slab *firstslab;
};

// 12 busktes as mentioned in the question pdf
struct bucket bucket_hash_table[12] = { {4,NULL}, {8, NULL}, {16, NULL}, {32, NULL}, {64, NULL}, {128, NULL}, {256, NULL}, {512, NULL}, {1024, NULL}, {2048, NULL}, {4096, NULL}, {8192, NULL} } ;


// nearest power 2 of the size request
unsigned int nextPowerOf2(unsigned int n)
{
    unsigned int p = 1;
    if (n && !(n & (n - 1)))
        return n;
 
    while (p < n) 
        p <<= 1;
     
    return p;
}


// initailaizing the newly created slab pointed to by ptr
void slab_init(void* ptr, int obj_size)
{
	struct slab *temp_slab_ptr;
	temp_slab_ptr = (struct slab*) ptr;

	temp_slab_ptr->totobj = (SZ - sizeof(struct slab))/(obj_size + sizeof(struct slab*));
	
	memset(temp_slab_ptr->bitmap, 0, sizeof(bool) * TOTOBJS);  // initialize bool array
	temp_slab_ptr->bitmap[0] = 1; // set the first free object

	temp_slab_ptr->freeobj = temp_slab_ptr->totobj - 1;        // 1 object already allocated, that's why we made a slab!
	temp_slab_ptr->bucket = &(bucket_hash_table[(int)log2(obj_size) - 2]); 
	temp_slab_ptr->nxtslab = NULL;
	
	ptr = (char*)ptr + sizeof(struct slab);
	
	struct object *temp_obj_ptr;
	
	for(int i = 0; i < temp_slab_ptr->totobj; ++i)
	{
		temp_obj_ptr = (struct object*) ptr;                  
		temp_obj_ptr->slab = temp_slab_ptr;                        // poiniting each objects' slab ptr to starting of the slab
		ptr = (char*)ptr + sizeof(struct slab*) + obj_size ;       // moving the ptr to the start of next object
	}
}

//0(1) function to return the  object which is free
void* slab_traverse(struct slab *slab_ptr, int obj_size)
{
	void* returnptr = (void*)slab_ptr;
	int traverse_offset ;
	int totobj = slab_ptr->totobj;
	slab_ptr->freeobj--;
	int counter;
	for(counter = 0;counter<totobj;++counter) // definetly we will get a free element here, as freeobj was > 0
	{
		if((slab_ptr->bitmap)[counter] == 0)
		{
			(slab_ptr->bitmap)[counter] = 1;
			break;
		}
	}
	traverse_offset = counter;
	int offset;

	offset = sizeof(struct slab) + traverse_offset*(obj_size + sizeof(struct slab*)) + sizeof(struct slab*);

	returnptr = (char*)returnptr + offset;
	
	return returnptr;
}

void* mymalloc(unsigned size_request)
{
	// printf("mymalloc size_request = %d\n", size_request);
	int size = (int)size_request;
	int obj_size = nextPowerOf2(size);
	int bucket_hash = log2(obj_size);
	void *void_slab_ptr;
	struct slab *slab_ptr;
	void *returnptr;

	if(bucket_hash < 2)
		bucket_hash = 2;
	
	bucket_hash -= 2;

	// lock the array location!
	head[bucket_hash].lock();

	slab_ptr = bucket_hash_table[bucket_hash].firstslab;

	// no slab allocated yet
	if(slab_ptr == NULL)
	{
		if ((void_slab_ptr  = (void *)mmap(NULL, SZ, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)) == MAP_FAILED) 
		{
			head[bucket_hash].unlock();
			perror("mmap");
			return void_slab_ptr;  // this is MAP_FAILED
		}
		slab_init(void_slab_ptr, obj_size);
		slab_ptr = (struct slab *)void_slab_ptr;
		slab_ptr->prevslab = NULL;
		bucket_hash_table[bucket_hash].firstslab = slab_ptr;
		void *temp_ptr = void_slab_ptr;
		temp_ptr = (char*)temp_ptr + sizeof(struct slab) + sizeof(struct slab*);
		returnptr = temp_ptr;
		head[bucket_hash].unlock();
		return returnptr;
	}
	// traverse the slabs, hopefully the first slab is a partial slab
	else
	{
		//unlock that position
		head[bucket_hash].unlock();
		struct slab *pred = slab_ptr;

		(pred->safe).lock();
		// search the first slab
		if(pred->freeobj > 0)
		{
			// search within a slab
			returnptr = slab_traverse(pred, obj_size);
			(pred->safe).unlock();
			return returnptr;
		}
		// if no free objects in the first slab
		else
		{	
			struct slab *curr = pred->nxtslab;
			if(curr != NULL)
				(curr->safe).lock();
			// only 1 slab present till now and that was full
			else
			{
				// create a new slab
				if ((void_slab_ptr  = (void*)mmap(NULL, SZ, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)) == MAP_FAILED) 
				{
					(pred->safe).unlock();
					perror("mmap");
					return void_slab_ptr; // this is MAP_FAILED
				}
				slab_init(void_slab_ptr, obj_size);
				curr = (struct slab*)void_slab_ptr;
				curr->prevslab = pred;
				// slab_ptr = slab_ptr->nxtslab;
				void *temp_ptr = void_slab_ptr;
				temp_ptr = (char*)temp_ptr + sizeof(struct slab) + sizeof(struct slab*);
				returnptr = temp_ptr;
				(pred->safe).unlock();
				return returnptr;
			}
			// this loop points to a slab with free space or points to null otherwise
			while(curr->freeobj <=0)
			{
				(pred->safe).unlock();
				pred = curr;
				curr = curr->nxtslab;
				if(curr == NULL)
				{
					curr = pred;
					pred = pred->prevslab;
					(pred->safe).lock();
					break;
				}
				(curr->safe).lock();
			}
			if(curr->freeobj > 0)
			{
				// search within the slab
				returnptr = slab_traverse(curr, obj_size);
				(curr->safe).unlock();
				(pred->safe).unlock();
				return returnptr;
			}
			else
			{	
				// create a new slab
				if ((void_slab_ptr  = (void*)mmap(NULL, SZ, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0)) == MAP_FAILED) 
				{
					perror("mmap");
					(curr->safe).unlock();
					(pred->safe).unlock();
					return void_slab_ptr;
				}
				slab_init(void_slab_ptr, obj_size);
				curr->nxtslab = (struct slab*)void_slab_ptr;
				(curr->nxtslab)->prevslab = curr;
				// slab_ptr = slab_ptr->nxtslab;
				void *temp_ptr = void_slab_ptr;
				temp_ptr = (char*)temp_ptr + sizeof(struct slab) + sizeof(struct slab*);
				returnptr = temp_ptr;
				(curr->safe).unlock();
				(pred->safe).unlock();
				return returnptr;
			}
		}	
	}

	//unlock that position
	// head[bucket_hash].unlock();
	printf("This should not have happened! :P!!\n");
	return returnptr;
}

void slab_destroy(struct slab *slab_ptr, int obj_size)
{
	int bucket_hash = log2(obj_size);

	// head[bucket_hash].lock();
	if(bucket_hash_table[bucket_hash].firstslab == slab_ptr)
	{
		bucket_hash_table[bucket_hash].firstslab = slab_ptr->nxtslab;
		// head[bucket_hash].unlock();
	}
	else
	{
		// head[bucket_hash].unlock();
		// (slab_ptr->prevslab->safe).lock();
		// (slab_ptr->safe).lock();
		struct slab *temp_slab_ptr;
		temp_slab_ptr = slab_ptr->prevslab;
		temp_slab_ptr->nxtslab = slab_ptr->nxtslab;
		// (slab_ptr->safe).unlock();
		// (slab_ptr->prevslab->safe).unlock();
	}
	
	 munmap((void*)slab_ptr, SZ);	
}

void myfree(void *ptr)
{
	// printf("my free\n");
	struct slab *slab_ptr;
	struct object *obj_ptr;
	ptr = (char*)ptr - sizeof(struct slab*);
	obj_ptr = (struct object*) ptr;
	

	slab_ptr = obj_ptr->slab;

	int obj_size = (slab_ptr->bucket)->objsz;

	int bucket_hash = log2(obj_size);
	if(bucket_hash < 2)
		bucket_hash = 2;
	
	bucket_hash -= 2;


	(slab_ptr->safe).lock();
	int traverse_offset = ((char*)obj_ptr - (char*)slab_ptr - sizeof(struct slab)) / (obj_size + sizeof(struct slab*)) ; 
	(slab_ptr->bitmap)[traverse_offset] = 0;   // freeing the objects' memory by setting the required bit
	// printf("For DEBUG 	%d\n",slab_ptr->totobj);



	if(slab_ptr->totobj == slab_ptr->freeobj)
	{
		if(slab_ptr->prevslab!=NULL)//check if this is the first slab
		{	
			(slab_ptr->prevslab->safe).lock();
			// printf("Aa gaya maei yahaan!\n");
			slab_destroy(slab_ptr, obj_size); // destroy the slab if it is completely free.
			(slab_ptr->prevslab->safe).unlock();
		}
		else
			slab_destroy(slab_ptr, obj_size); // destroy the slab if it is completely free.


	}
	(slab_ptr->safe).unlock();


	//unlock array location
	head[bucket_hash].unlock();

}
