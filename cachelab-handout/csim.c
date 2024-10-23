#include "cachelab.h"

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER 256
#define MEMORY_BITS 64

struct cache_line {
    int age;
    int valid;
    long long tag;
};

long long power_2(int n)
{
    long long res = 2;
    for (int i = 1; i < n; i++)
        res *= 2;

    return res;
}

long long get_set_index(long long address, int set_bits, int block_bits)
{
    return (address >> block_bits) & (power_2(set_bits)-1);
}

long long get_tag(long long address, int set_bits, int block_bits)
{
    return (address >> (set_bits+block_bits));
}

int main(int argc, char **argv)
{
    int sflag = 0;
    int Eflag = 0;
    int bflag = 0;
    int vflag = 0;
    char *tflag = NULL;
    int c;

    while ((c = getopt(argc, argv, "vs:E:b:t:")) != -1)
        switch (c) {
            case 's':
                sflag = atoi(optarg);
                break;
            case 'E':
                Eflag = atoi(optarg);
                break;
            case 'b':
                bflag = atoi(optarg);
                break;
            case 't':
                tflag = optarg;
                break;
            case 'v':
                vflag = 1;
                break;
        }

    if (!sflag || !Eflag || !bflag || tflag == NULL) {
        printf("./csim: Missing required command line argument\n");
        printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
        printf("Options:\n");
        printf("  -h         Print this help message.\n");
        printf("  -v         Optional verbose flag.\n");
        printf("  -s <num>   Number of set index bits.\n");
        printf("  -E <num>   Number of lines per set.\n");
        printf("  -b <num>   Number of block offset bits.\n");
        printf("  -t <file>  Trace file.\n\n");
        printf("Examples:\n");
        printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
        printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
        return 1;
    }

    if (bflag+sflag > MEMORY_BITS) {
        printf("b+s cannot be greater than 64\n");
        return 1;
    }

    struct cache_line** cache = malloc(sizeof(struct cache_line*) * power_2(sflag));
    if (!cache) {
        printf("Failed to initialize cache\n");
        return 1;
    }
    for (int i = 0; i < power_2(sflag); i++) {
        cache[i] = malloc(Eflag * sizeof(struct cache_line));
        if (!cache[i]) {
            printf("Failed to initialize cache line\n");
            return 1;
        }

        for (int j = 0; j < Eflag; j++) {
            cache[i][j].age = 0;
            cache[i][j].tag = 0;
            cache[i][j].valid = 0;
        }
    }

    int hits = 0;
    int misses = 0;
    int evictions = 0;
    FILE *fptr;
    fptr = fopen(tflag, "r");
    char content[BUFFER];
    while (fgets(content, BUFFER, fptr)) {
        if (content[0] == 'I')
            continue;

        char address[MEMORY_BITS];
        char line2print[BUFFER];
        strncpy(address, "", sizeof(address));
        strncpy(line2print, "", sizeof(line2print));
        int i = 1;
        int stop = 0;
        while (1) {
            line2print[i-1] = content[i];
            if (!stop && (i > 2))
                address[i-3] = content[i];
            i++;
            if (content[i] == '\n')
                break;
            if (content[i] == ',')
                stop = 1;
        }

        if (vflag) {
            printf("%s ", line2print);
        }

        long long laddress = strtoll(address, NULL, 16);
        long long set_index = get_set_index(laddress, sflag, bflag);
        long long tag = get_tag(laddress, sflag, bflag);

        int line = 0;
        int oldest = INT_MIN;
        int evict = 0;
        for (int e = 0; e < Eflag; e++) {
            if (cache[set_index][e].valid) {
                if (cache[set_index][e].tag == tag) {
                    evict = 0;
                    if (content[1] == 'M') {
                        if (vflag)
                            printf("hit ");
                        hits++;
                    }
                    if (vflag)
                        printf("hit\n");
                    hits++;
                } else {
                    evict = 1;
                    if (oldest < cache[set_index][e].age) {
                        line = e;
                        oldest = cache[set_index][e].age;
                    }
                    continue;
                }
            } else {
                evict = 0;
                if (vflag)
                    printf("miss");
                misses++;
                cache[set_index][e].valid = 1;
                cache[set_index][e].tag = tag;
                if (content[1] == 'M') {
                    if (vflag)
                        printf(" hit");
                    hits++;
                }
                if (vflag)
                    printf("\n");
            }
            line = e;
            break;
        }

        for (int e = 0; e < Eflag; e++) {
            cache[set_index][e].age++;
        }
        cache[set_index][line].age = 0;

        if (evict) {
            if (vflag)
                printf("miss eviction");
            misses++;
            evictions++;
            cache[set_index][line].tag = tag;
            if (content[1] == 'M') {
                if (vflag)
                    printf(" hit");
                hits++;
            }
            if (vflag)
                printf("\n");
        }
    }
    fclose(fptr);
    for (int i = 0; i < sflag; i++)
        free(cache[i]);
    free(cache);

    printSummary(hits, misses, evictions);
    return 0;
}
