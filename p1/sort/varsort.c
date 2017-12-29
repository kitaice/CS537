#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include "sort.h"
#include <sys/types.h>
#include <sys/stat.h>
int column;
int cmp(const void * a, const void * b) {
    rec_dataptr_t obj1 = *(rec_dataptr_t*)a;
    rec_dataptr_t obj2 = *(rec_dataptr_t*)b;
    unsigned int num1 = (column <= (obj1.data_ints-1))
        ?obj1.data_ptr[column]:obj1.data_ptr[obj1.data_ints-1];
    unsigned int num2 = (column <= (obj2.data_ints-1))
        ?obj2.data_ptr[column]:obj2.data_ptr[obj2.data_ints-1];
    return (num1 > num2);
}
void usage(char *prog) {
    fprintf(stderr, "Usage: ./varsort -i inputfile "
        "-o outputfile [-c column]\n");
    exit(1);
}
int main(int argc , char* argv[]) {
    //  arguments
    char* inFile = NULL;
    char* outFile = NULL;
    int c = 0;
    column = 0;
    opterr = 0;
    while ((c = getopt(argc, argv, "i:o:c:")) != -1) {
        switch (c) {
            case 'i':
                inFile = strdup(optarg);
                break;
            case 'o':
                outFile = strdup(optarg);
                break;
            case 'c':
                column = atoi(optarg);
                    if (column <0) {
                        fprintf(stderr , "Error: Column should"
                            " be a non-negative integer\n");
                        exit(1);
                        }
                break;
            default:
                usage(argv[0]);
        }
    }
    if (!inFile) usage(argv[0]);
    if (!outFile) usage(argv[0]);
    //  open file
    int fd = open(inFile, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr , "Error: Cannot open file %s\n" , inFile);
        exit(1);
    }

    int recordsLeft;
    int rc;
    //  read num of records
    rc = read(fd, &recordsLeft, sizeof(recordsLeft));
    if (rc != sizeof(recordsLeft)) {
        perror("read");
        exit(1);
    }
//  printf("Number of records: %d\n", recordsLeft);
    int numRecord = recordsLeft;
    //  read records

    rec_nodata_t r;
    rec_dataptr_t *records = (rec_dataptr_t*)
        malloc(numRecord*(sizeof(rec_dataptr_t)));
    if (records == NULL) fprintf(stderr , "Error: failed to allocate memory. ");
    int i;
//  int j;
    unsigned int size = 0;
    for (i = 0 ; i < numRecord ; i++) {
        rc = read(fd , &(r.index) , sizeof(unsigned int));
        if (rc != sizeof(unsigned int)) {
            perror("read");
            exit(1);
        }

        rc = read(fd , &(r.data_ints) , sizeof(unsigned int));
        if (rc != sizeof(unsigned int)) {
            perror("read");
            exit(1);
        }

        records[i].index = r.index;
        records[i].data_ints = r.data_ints;
        size = r.data_ints;
        records[i].data_ptr = malloc(sizeof(unsigned int)*size);
        rc = read(fd , records[i].data_ptr , sizeof(unsigned int)*size);
        if (rc != size*sizeof(unsigned int)) {
            perror("read");
            exit(1);
        }
//      printf("%u ",records[i].index);
//      printf("%u ",records[i].data_ints);
//      for(j=0;j<records[i].data_ints;j++) printf("%u  ",
//        records[i].data_ptr[j]);
//        printf("\n");
    }
    (void) close(fd);
    //  sort
    qsort(records , numRecord , sizeof(rec_dataptr_t) , cmp);

//    for (i = 0 ; i < numRecord ; i++) {
//        printf("%u " , records[i].index);
//        printf("%u " , records[i].data_ints);
//        for (j = 0 ; j < records[i].data_ints ; j++)
//            printf("%u  " , records[i].data_ptr[j]);
//        printf("\n");
//    }i
    //  write
    fd = open(outFile , O_WRONLY|O_CREAT|O_TRUNC , S_IRWXU);
    if (fd < 0) {
    fprintf(stderr, "Error: Cannot open file %s\n", outFile);
    exit(1);
    }
    rc = write(fd , &recordsLeft , sizeof(int));
    if (rc != sizeof(recordsLeft)) {
         perror("write");
         (void)close(fd);
         exit(1);
    }

    for (i = 0 ; i < recordsLeft ; i++) {
//     unsigned int index=records[i].index;
        size = (sizeof(unsigned int));
        rc = write(fd , &records[i].index , size);
        if (rc != sizeof(unsigned int)) {
            perror("write");
            (void)close(fd);
            exit(1);
            }
         rc = write(fd , &records[i].data_ints , size);
        if (rc != sizeof(unsigned int)) {
            perror("write");
            (void)close(fd);
            exit(1);
            }

        size = (sizeof(unsigned int)*(records[i].data_ints));
        rc = write(fd , records[i].data_ptr , size);
        if (rc != size) {
            perror("write");
            (void)close(fd);
            exit(1);
           }
    }
    (void)close(fd);
    for (i = 0 ; i < numRecord ; i++) free(records[i].data_ptr);
    free(records);
    free(inFile);
    free(outFile);


    exit(0);
}
