#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>


void* dumb(void* someth);
void* naive(void* data);
void* still_naive(void* someth);
void* good(void* someth);

struct data_t
{
	long long n;  
	long long* count;
	pthread_mutex_t* m;
};

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		printf("To few arguments\n");
		exit(0);
	}

	long long n = atoll(argv[1]);
	int nthr = atoi(argv[2]);
	int alg = atoi(argv[3]);

	long long count = 0;
	pthread_t tr[nthr];

	pthread_mutex_t mut;
	pthread_mutex_init(&mut, NULL); 
	struct data_t data = {(n / nthr) + 1, &count, &mut};

	switch(alg)
	{
		case 1:
		{
			for (int i = 0; i < nthr; i++)
			{
				pthread_create(tr + i, NULL, dumb, &data);
			}
			break;
		}
		case 2:
		{

			for (int i = 0; i < nthr; i++)
			{
				pthread_create(tr + i, NULL, naive, &data);
			}
			break;
		}
		case 3:
		{

			for (int i = 0; i < nthr; i++)
			{
				pthread_create(tr + i, NULL, still_naive, &data);
			}
			break;
		}

		case 4:
		{

			for (int i = 0; i < nthr; i++)
			{
				pthread_create(tr + i, NULL, good, &data);
			}
			break;
		}

		default:
		{
			printf("Wrong algorythm\n");
			exit(0);
		}
	}

	for (int i = 0; i < nthr; i++)
				pthread_join(tr[i], NULL);
			
	return 0;
}

void* dumb(void* someth)
{
	struct data_t* data = (struct data_t*) someth;
	for(long long i = 0; i < (data -> n); ++i)
		(*(data -> count))++;
	
	#ifdef PRINT
		printf("Done\n");
		printf("Data = %lld\n", *(data -> count));
	#endif

	return NULL;
}

void* naive(void* someth)
{
	struct data_t* data = (struct data_t*) someth;
	for(long long i = 0; i < data -> n; ++i)
	{
		pthread_mutex_lock((data -> m));
		(*(data -> count))++;
		pthread_mutex_unlock((data -> m));
	}
	#ifdef PRINT
		printf("Done\n");
		printf("Data = %lld\n", *(data -> count));
	#endif

	return NULL;
}

void* still_naive(void* someth)
{
	struct data_t* data = (struct data_t*) someth;

	pthread_mutex_lock((data -> m));
	for(long long i = 0; i < data -> n; ++i)
		(*(data -> count))++;
	pthread_mutex_unlock((data -> m));

	#ifdef PRINT
		printf("Done\n");
		printf("Data = %lld\n", *(data -> count));
	#endif

	return NULL;
}

void* good(void* someth)
{
	struct data_t* data = (struct data_t*) someth;
	long long cnt = 0;
	for(long long i = 0; i < data -> n; ++i)
		cnt++;

	pthread_mutex_lock((data -> m));
		*(data -> count) += cnt;
	pthread_mutex_unlock((data -> m));

	#ifdef PRINT
		printf("Done\n");
		printf("Data = %lld\n", *(data -> count));
	#endif
	return NULL;
}


