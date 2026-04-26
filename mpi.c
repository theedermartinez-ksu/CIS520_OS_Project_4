#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define STRING_SIZE 20000

int  ARRAY_SIZE;
int  NUM_RANKS;

char** char_array = NULL;
int* results;
int   num_lines_total;

FILE* open_file()
{
    FILE* fp = NULL;
    fp = fopen("/homes/eyv/cis520/wiki_dump.txt", "r");

    if (fp == NULL)
    {
        perror("Error opening file");
        exit(-1);
    }

    return fp;
}

void allocate_memory()
{
    int max_lines = (ARRAY_SIZE / NUM_RANKS) / STRING_SIZE;

    char_array = malloc(max_lines * sizeof(char*));
    results = malloc(max_lines * sizeof(int));

    if (char_array == NULL || results == NULL)
    {
        perror("Error loading memory");
        return;
    }

    for (int i = 0; i < max_lines; i++)
    {
        char_array[i] = NULL;
        char_array[i] = malloc(STRING_SIZE);
        if (char_array[i] == NULL)
            perror("Error loading memory");
    }
}

//load the data
int init_arrays(FILE* fp)
{
    int    i = 0;
    char* line = NULL;
    size_t len = 0;

    int max_lines = (ARRAY_SIZE / NUM_RANKS) / STRING_SIZE;

    while (i < max_lines && getline(&line, &len, fp) != -1)
    {
        int j = 0;
        while (line[j] != '\0' && j < STRING_SIZE - 1)
        {
            if (line[j] == '\n') break;
            char_array[i][j] = line[j];
            j++;
        }
        char_array[i][j] = '\0';
        i++;
    }

    free(line);
    num_lines_total = i;
    return i;
}

//each rank processes its assigned lines
void compute_max(int rank)
{
    for (int i = rank; i < num_lines_total; i += NUM_RANKS)
    {
        int max = 0;
        for (int j = 0; char_array[i][j] != '\0'; j++)
        {
            if ((unsigned char)char_array[i][j] > max)
                max = (unsigned char)char_array[i][j];
        }
        results[i] = max;
    }
}

void free_memory()
{
    int max_lines = (ARRAY_SIZE / NUM_RANKS) / STRING_SIZE;
    for (int i = 0; i < max_lines; i++)
        free(char_array[i]);
    free(char_array);
    free(results);
}

void process_file_batches(int rank)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);

    char filename[100];
    sprintf(filename, "output_mpi%d-%d.txt", NUM_RANKS, ARRAY_SIZE);

    FILE* out = NULL;
    FILE* fp = NULL;

    if (rank == 0)
    {
        out = fopen(filename, "w");
        if (out == NULL) { perror("Error opening output file"); exit(-1); }
        fp = open_file();
    }

    //buffer max_lines * STRING_SIZE chars
    int max_lines = (ARRAY_SIZE / NUM_RANKS) / STRING_SIZE;
    char* flat_buf = malloc((size_t)max_lines * STRING_SIZE);
    if (!flat_buf) { perror("malloc flat_buf"); exit(-1); }

    int global_index = 0;

    while (1)
    {
        int lines_loaded = 0;

        // rank 0 reads a batch
        if (rank == 0)
            lines_loaded = init_arrays(fp);

        // tell everyone how many lines this batch has
        MPI_Bcast(&lines_loaded, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (lines_loaded == 0) break;

        // pack char_array into buffer on rank 0 and broadcast to all
        if (rank == 0)
        {
            for (int i = 0; i < lines_loaded; i++)
                memcpy(flat_buf + (size_t)i * STRING_SIZE,
                    char_array[i], STRING_SIZE);
        }
        MPI_Bcast(flat_buf, max_lines * STRING_SIZE, MPI_CHAR,
            0, MPI_COMM_WORLD);

        // unpack into char_array on the non zero ranks
        if (rank != 0)
        {
            num_lines_total = lines_loaded;
            for (int i = 0; i < lines_loaded; i++)
                memcpy(char_array[i],
                    flat_buf + (size_t)i * STRING_SIZE, STRING_SIZE);
        }

        // zero out results then each rank fills its slice
        memset(results, 0, (size_t)lines_loaded * sizeof(int));
        compute_max(rank);

        // reduce collapses all slices
        int* gathered = NULL;
        if (rank == 0)
            gathered = malloc((size_t)lines_loaded * sizeof(int));

        MPI_Reduce(results, gathered, lines_loaded, MPI_INT, MPI_MAX,
            0, MPI_COMM_WORLD);

        // rank 0 writes this batch
        if (rank == 0)
        {
            for (int k = 0; k < lines_loaded; k++)
                fprintf(out, "%d: %d\n", global_index + k, gathered[k]);
            free(gathered);
        }

        global_index += lines_loaded;
    }

    gettimeofday(&end, NULL);
    double elapsed_time = (end.tv_sec - start.tv_sec) +
        (end.tv_usec - start.tv_usec) / 1e6;

    if (rank == 0)
    {
        fprintf(out, "\n===== PERFORMANCE DATA =====\n");
        fprintf(out, "MPI Ranks: %d\n", NUM_RANKS);
        fprintf(out, "Array Size (Memory): %d\n", ARRAY_SIZE);
        fprintf(out, "Total Lines Processed: %d\n", global_index);
        fprintf(out, "Execution Time: %.6f seconds\n", elapsed_time);

        fclose(out);
        fclose(fp);
    }

    free(flat_buf);
}

int main(int argc, char* argv[])
{
    int rc = MPI_Init(&argc, &argv);
    if (rc != MPI_SUCCESS)
    {
        printf("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    int rank, nprocs;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    //errror stuff
    if (argc < 3)
    {
        if (rank == 0)
            printf("Usage: %s <num_ranks> <array_size>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    if (sscanf(argv[1], "%d", &NUM_RANKS) != 1)
    {
        if (rank == 0) printf("invalid number of ranks\n");
        MPI_Finalize();
        return 1;
    }

    if (sscanf(argv[2], "%d", &ARRAY_SIZE) != 1)
    {
        if (rank == 0) printf("invalid array size\n");
        MPI_Finalize();
        return 1;
    }

    NUM_RANKS = nprocs;

    allocate_memory();

    process_file_batches(rank);

    if (rank == 0)
        printf("Main: program completed. Exiting.\n");

    free_memory();

    MPI_Finalize();
    return 0;
}