#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4

#define ARRAY_SIZE 2000000
#define STRING_SIZE 20000
#define ALPHABET_SIZE 26

pthread_mutex_t mutexsum;			// mutex for char_counts

char char_array[ARRAY_SIZE][STRING_SIZE];
int results[ARRAY_SIZE];			// count of individual characters

int num_lines_total;// assign the actual number of lines

FILE* open_file()
{
	FILE *fp = NULL;
	fp = fopen("/homes/eyv/cis520/wiki_dump.txt","r");

	if(fp == NULL)
	{
		perror("Error opening file");
		exit(-1);
	}

	return fp;
}

void init_arrays()
{

  	int i, j;
  	FILE *fp = open_file();
	// 2d array??
	i = 0;
	num_lines_total= 0;

	char* line = NULL;
	char current_char = 0;
	size_t len = 0;
	//read each line
	while(i < ARRAY_SIZE && getline(&line, &len,fp) != -1)
	{
		j = 0;// reset
		// read each line until end \0
		while(line[j] != '\0' && j < STRING_SIZE - 1)
		{
			// here add to
			if(line[j] == '\n') break;
			char_array[i][j] = line[j];
			j++;
		}
		// terminate string too help when running count
		char_array[i][j] = '\0';
		i++;

		num_lines_total++;
	}

	fclose(fp);
}

void *count_array(void *myID)
{
	// grab actual lines we have and ignore the overhead.
	//int line_lines = num_lines_total / NUM_THREADS;

	int myIDInt = *((int*)myID);

	int startSection = myIDInt * (num_lines_total / NUM_THREADS);
	int endSection = startSection + (num_lines_total / NUM_THREADS);

	// for looop looping and computing per line.
	//last thread finishes it up
	if(myIDInt == NUM_THREADS - 1)
	{
		endSection = num_lines_total;
	}


	for( int i = startSection; i < endSection; i++)
	{
		int max = 0;
		for(int j = 0; char_array[i][j] != '\0'; j++)
		{
			if((int)char_array[i][j] > max)
			{
				max = (int) char_array[i][j];
			}
		}
		results[i] = max;

	}

	pthread_exit(NULL);

}

void print_results()
{
    for (int i = 0; i < num_lines_total; i++) {
        printf("%d: %d\n", i, results[i]);
    }
}

main() {
	int i, rc;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;

	int ids[NUM_THREADS];


	/* Initialize and set thread detached attribute */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	//load file
	init_arrays();

	for (i = 0; i < NUM_THREADS; i++ ) {
			ids[i] = i;
	      rc = pthread_create(&threads[i], &attr, count_array, &ids[i]);
	      if (rc) {
	        printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	      }
	}

	/* Free attribute and wait for the other threads */
	pthread_attr_destroy(&attr);
	for(i=0; i<NUM_THREADS; i++) {

	     rc = pthread_join(threads[i], &status);
	     if (rc) {
		   printf("ERROR; return code from pthread_join() is %d\n", rc);
		   exit(-1);
	     }
	}

	print_results();

	printf("Main: program completed. Exiting.\n");
	return 0;
}

