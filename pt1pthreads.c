#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#define STRING_SIZE 20000
#define MAX_LINES 1000000

// Global
int  ARRAY_SIZE;
int NUM_THREADS;

char **char_array = NULL;

int *results;

int num_lines_total;

//---------open file --------
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


//-----------------Memory allcooation (no more static arrays) -------------
void allocate_memory()
{
	int max_lines_possible = ARRAY_SIZE / STRING_SIZE;

	char_array = malloc(max_lines_possible * sizeof(char*));
	results = malloc(max_lines_possible * sizeof(int));

	if(char_array == NULL  || results == NULL)
	{
		perror("Error loading memory");
		return;
	}

	for (int i = 0; i < max_lines_possible; i++)
	{
		char_array[i] = NULL;
		char_array[i] = malloc(STRING_SIZE);
		if(char_array[i]  == NULL)
		{
			perror ("Error loading memory");
		}
	}

}

//-----------load dtat-----
int init_arrays(FILE *fp)
{

	int i = 0;
	char* line = NULL;
	size_t len = 0;

	int max_lines_possible = ARRAY_SIZE / STRING_SIZE;

	while(i < max_lines_possible && getline(&line, &len, fp) != -1)
	{
		int j = 0;
		while(line[j] != '\0' && j < STRING_SIZE - 1)
		{
			if(line[j] == '\n') break;
			char_array[i][j] = line[j];
			j++;
		}
		char_array[i][j] = '\0';
		i++;
	}

	free(line);
	num_lines_total = i;

	return i; // Number of lines loaoded

}


void *count_array(void *myID)
{
	// ggrab lines we have ignore overhead
	//int line_lines = num_lines_total / NUM_THREADS;

	int myIDInt = *((int*)myID);

	int chunk = num_lines_total / NUM_THREADS;
	int startSection = myIDInt * chunk;
	int endSection = startSection + chunk;

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
			if((unsigned char)char_array[i][j] > max)
			{
				max = (unsigned char) char_array[i][j];
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

void write_output_to_file(const char *filename, double elapsed_time) {
	FILE *fp = fopen(filename, "w");

	if (fp == NULL) {
		perror("Error opening output file");
		return;
	}

	// ------write results
	for (int i = 0; i < num_lines_total; i++) {
		fprintf(fp, "%d: %d\n", i, results[i]);
	}

	// --preformance info
	fprintf(fp, "\n===== PERFORMANCE DATA =====\n");
	fprintf(fp, "Threads: %d\n", NUM_THREADS);
	fprintf(fp, "Array Size (Memory): %d\n", ARRAY_SIZE);
	fprintf(fp, "Lines Processed: %d\n", num_lines_total);
	fprintf(fp, "Execution Time: %.6f seconds\n", elapsed_time);

	fclose(fp);
}

void free_memory()
{
	int max_lines_possible = ARRAY_SIZE / STRING_SIZE;
	// again coounter
	for(int i = 0; i < max_lines_possible; i++)
	{
		free(char_array[i]);
	}
	free(char_array);
	free(results);
}

void process_file_batches(pthread_t threads[], pthread_attr_t *attr, int ids[])
{
	//struct to keep track oof time
	struct timeval start, end;
	gettimeofday(&start,NULL);

	// Create the approaite file
	char filename[100];
	sprintf(filename, "output%d-%d.txt", NUM_THREADS, ARRAY_SIZE);

	FILE *out = fopen(filename, "w");
	if (out == NULL) {
		perror("Error opening output file");
		exit(-1);
	}
	FILE *fp = open_file();

	int global_index = 0;
	int i, rc;
	void *status;

	while (1)
	{
		int lines_loaded = init_arrays(fp);
		// ifn noothing is loaded
		if (lines_loaded == 0) break;

		num_lines_total = lines_loaded;

		// create threads
		for (i = 0; i < NUM_THREADS; i++ ) {
			ids[i] = i;
			rc = pthread_create(&threads[i], attr, count_array, &ids[i]);
			if (rc) {
				printf("ERROR; return code from pthread_create() is %d\n", rc);
				exit(-1);
			}
		}

		// join threads
		for(i = 0; i < NUM_THREADS; i++) {
			rc = pthread_join(threads[i], &status);
			if (rc) {
				printf("ERROR; return code from pthread_join() is %d\n", rc);
				exit(-1);
			}
		}
		//write
		for (int k = 0; k < lines_loaded; k++) {
			fprintf(out, "%d: %d\n", global_index, results[k]);
			global_index++;
		}

	}
	gettimeofday(&end,NULL);

	double elapsed_time = (end.tv_sec - start.tv_sec) +
						  (end.tv_usec - start.tv_usec) / 1e6;

	fprintf(out, "\n===== PERFORMANCE DATA =====\n");
	fprintf(out, "Threads: %d\n", NUM_THREADS);
	fprintf(out, "Array Size (Memory): %d\n", ARRAY_SIZE);
	fprintf(out, "Total Lines Processed: %d\n", global_index);
	fprintf(out, "Execution Time: %.6f seconds\n", elapsed_time);


	fclose(out);
	fclose(fp);
}

int main(int argc, char* argv[] ) {

	//---------------------------ERROR CHECKING ---------------
	if(argc <3)
	{
		printf("Usage: %s <num_threads> <array_size>\n", argv[0]);
		return 1;
	}

	if(sscanf(argv[1], "%d", &NUM_THREADS) != 1)
	{
		printf("invalid number threads\n");
		return 1;
	}

	if(sscanf(argv[2], "%d", &ARRAY_SIZE) != 1)
	{
		printf("invalid array size\n");
		return 1;
	}

	//---------------------------SET UP TIME-------------------

	// allocate memory
	allocate_memory();

	int i, rc;
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void *status;

	int ids[NUM_THREADS];

	//-------------init threads
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	process_file_batches(threads, &attr, ids);


	// write to file
	char filename[100];
	sprintf(filename, "output%d-%d.txt", NUM_THREADS, ARRAY_SIZE);


	printf("Main: program completed. Exiting.\n");

	free_memory();

	return 0;
}
