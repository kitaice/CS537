#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fs.h"
#include "types.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]){
    struct stat buf;
    int fd=open(argv[1],O_RDONLY);
    if(fd == -1) {
        fprintf(stderr,"image not found.\n");
        exit(1);
    }
    fstat(fd,&buf);
    void *imgptr = (void*)mmap(NULL, buf.st_size, PROT_READ,MAP_PRIVATE,fd,0);
    struct superblock *sb =(struct superblock*)(imgptr+BSIZE);
    struct dinode *dip = (struct dinode*)(imgptr+BSIZE*2);
    int dpb = (BSIZE / sizeof(struct dirent));
//    int const nblks = (int)sb->nblocks;
    int const size = (int)sb->size;

    uint ninodes = (sb->ninodes + IPB - 1) / IPB;
    uint nbitmap = (sb->size + BPB - 1) / BPB;
    uchar *bitmap = (uchar*)(imgptr+BSIZE*((ninodes)+4-1));

    uint data_bound=nbitmap+ninodes+3;
    uint addr;
    uint iaddr;
    uint* indirect;
    struct dirent* dirs;

    int usage[size];
    for(int i=0;i<size;i++) usage[i]=0;

    uint index=0;
    uint location=0;
    uint offset=0;

    for(int i=0;i<(int)sb->ninodes;i++) {
        //TODO:
        //Each inode is either unallocated or one of the valid types (T_FILE, T_DIR, T_DEV). ERROR: bad inode.
        if (!(dip[i].type == 0 || dip[i].type == 1 || dip[i].type == 2 || dip[i].type == 3)) {
            fprintf(stderr,"ERROR: bad inode.\n");
            exit(1);
        }

        //TODO:
        //Each directory contains . and .. entries. ERROR: directory not properly formatted.
        if(dip[i].type == 1){
            addr=dip[i].addrs[0];
            dirs = (struct dirent*)(imgptr+BSIZE*addr);
            if(!(strcmp(dirs[0].name,".")==0 && strcmp(dirs[1].name,"..")==0)) {
                fprintf(stderr,"ERROR: directory not properly formatted.\n");
                exit(1);
            }
        }

        //TODO:
        //If the direct block is used and is invalid, print ERROR: bad direct address in inode.
        for (int j = 0; j < NDIRECT; j++) {
            if (dip[i].addrs[j] == 0) {
            } else if (dip[i].addrs[j] > size || dip[i].addrs[j] < data_bound) {
                //larger than size or smaller than the area of non-data blks
                fprintf(stderr, "ERROR: bad direct address in inode.\n");
                exit(1);
            }
        }

        //TODO:
        // if the indirect block is in use and is invalid,print ERROR: bad indirect address in inode.
        addr = dip[i].addrs[NDIRECT];
        if (addr != 0) {
            if (addr > size || addr < data_bound) {
                //larger than size or smaller than the area of non-data blks
                fprintf(stderr, "ERROR: bad indirect address in inode.\n");
                exit(1);
            }
            indirect = (uint*)(imgptr+BSIZE*addr);
            for (int j = 0; j < NINDIRECT; j++) {
                iaddr = indirect[j];
                if(iaddr!=0){
                    if (iaddr> size ||iaddr < data_bound) {
                        //larger than size or smaller than the area of non-data blks
                        fprintf(stderr, "ERROR: bad indirect address in inode.\n");
                        exit(1);
                    }
                }
            }
        }

        if(dip[i].type!=0){
            //TODO:
            //For in-use inodes, direct address in use is only used once. ERROR: direct address used more than once.
            for (int j = 0; j < NDIRECT; j++) {
                addr = dip[i].addrs[j];
                if (addr != 0) {
                    if (usage[addr] == 0) {
                        usage[addr] = 1;
                    } else {
                        fprintf(stderr, "ERROR: direct address used more than once.\n");
                        exit(1);
                    }
                }
            }

            //TODO:
            //For in-use inodes, indirect address in use is only used once. ERROR: indirect address used more than once.
            if (dip[i].addrs[NDIRECT] != 0) {
                addr = dip[i].addrs[NDIRECT];
                if (usage[addr] == 0) {
                    usage[addr] = 1;
                } else {
                    fprintf(stderr, "ERROR: direct address used more than once.\n");
                    exit(1);
                }
                indirect = (uint*)(imgptr+BSIZE*addr);
                for (int j = 0; j < NINDIRECT; ++j) {
                    iaddr = indirect[j];
                    if (iaddr != 0) {
                        if (usage[iaddr] == 0) {
                            usage[iaddr] = 1;
                        } else {
                            fprintf(stderr, "ERROR: indirect address used more than once.\n");
                            exit(1);
                        }
                    }
                }
            }

            //TODO:
            //For in-use inodes, each address in use is also marked in use in the bitmap. ERROR: address used by inode but marked free in bitmap.
            for (uint j = 0; j < NDIRECT; ++j) {
                addr=dip[i].addrs[j];
                location = addr/8;
                offset = addr%8;
                if((1<<offset)& bitmap[location]){
                } else {
                    //printf("i=%d loc=%d off=%d\n",i,location,offset);
                    fprintf(stderr,"ERROR: address used by inode but marked free in bitmap.\n");
                    exit(1);
                }
            }
            if(dip[i].addrs[NDIRECT]!=0){
                addr=dip[i].addrs[NDIRECT];
                indirect = (uint*)(imgptr+BSIZE*addr);
                for (uint j = 0; j < NINDIRECT; ++j){
                    iaddr=indirect[j];
                    if(iaddr!=0){
                        location = iaddr/8;
                        offset = iaddr%8;
                        if((1<<offset)& bitmap[location]){
                        } else {
                            //printf("i=%d loc=%d off=%d\n",i,location,offset);
                            fprintf(stderr,"ERROR: address used by inode but marked free in bitmap.\n");
                            exit(1);
                        }
                    }
                }
            }
        }

    }


    //TODO:
    //Root directory exists, and it is inode number 1. ERROR: root directory does not exist.
    //check the first dirent of first inode, inum == 1, and  if the name is "."
    addr = dip[1].addrs[0];
    dirs = (struct dirent*)(imgptr+BSIZE*addr);
    if(dip[1].type != 1){
        fprintf(stderr,"ERROR: root directory does not exist.\n");
        exit(1);
    }
    if(!(strcmp(dirs[0].name,".")==0 && dirs[0].inum==1
         &&strcmp(dirs[1].name,"..")==0&& dirs[1].inum==1)) {
        fprintf(stderr,"ERROR: root directory does not exist.\n");
        exit(1);
    }



    ushort refcount[(int)sb->ninodes];
    memset(refcount, 0, sizeof(refcount));
    refcount[1] = 1;
    ushort parentdir[(int)sb->ninodes];
    memset(parentdir, 0, sizeof(parentdir));
    ushort childdir[(int)sb->ninodes];
    memset(childdir, 0, sizeof(childdir));
    childdir[1] = 1;
    for(int i=0;i<(int)sb->ninodes;i++){
        if (dip[i].type == 1){

            for (uint j = 0; j < NDIRECT; ++j) {
                addr=dip[i].addrs[j];
                dirs = (struct dirent*)(imgptr+BSIZE*addr);
                int k=0;
                if(j==0){
                    parentdir[i]=dirs[1].inum;
                    k=2;
                }
                for(;k<dpb;k++){
                    if (dirs[k].inum != 0) {
                        refcount[dirs[k].inum]++;
                        childdir[dirs[k].inum] = i;
                    }
                }
            }
            if(dip[i].addrs[NDIRECT]!=0){
                addr=dip[i].addrs[NDIRECT];
                indirect = (uint*)(imgptr+BSIZE*addr);
                for (uint j = 0; j < NINDIRECT; ++j){
                    iaddr=indirect[j];
                    if(iaddr!=0){
                        dirs=(struct dirent*)(imgptr+BSIZE*iaddr);
                        for (int k = 0; k < dpb; ++k) {
                            if (dirs[k].inum != 0) {
                                refcount[dirs[k].inum]++;
                                childdir[dirs[k].inum] = i;
                            }
                        }
                    }

                }
            }
        }
    }
    for(int i=0;i<(int)sb->ninodes;i++) {
        //TODO:
        //For all inodes marked in use, must be referred to in at least one directory. ERROR: inode marked use but not found in a directory.
        if(dip[i].type!=0){
            if (refcount[i] == 0) {
                fprintf(stderr, "ERROR: inode marked use but not found in a directory.\n");
                exit(1);
            }
        }
        //TODO:
        //For each inode number that is referred to in a valid directory, it is actually marked in use. ERROR: inode referred to in directory but marked free.
        if (dip[i].type == 0) {
            if (refcount[i] > 0) {
                fprintf(stderr, "ERROR: inode referred to in directory but marked free.\n");
                exit(1);
            }
        }

        //TODO:
        //No extra links allowed for directories (each directory only appears in one other directory). ERROR: directory appears more than once in file system.
        if(dip[i].type==1){
            if (refcount[i] >1) {
                fprintf(stderr, "ERROR: directory appears more than once in file system.\n");
                exit(1);
            }
        }

        //TODO:
        //Each .. entry in directory refers to the proper parent inode, and parent inode points back to it. ERROR: parent directory mismatch.
        if (dip[i].type == 1) {
            if (parentdir[i] != childdir[i]) {
                fprintf(stderr, "ERROR: parent directory mismatch.\n");
                exit(1);
            }
        }
        //TODO:
        //Reference counts (number of links) for regular files match the number of times file is referred to in directories (i.e., hard links work correctly)
        //ERROR: bad reference count for file.
        if (dip[i].type == 2){
            if(dip[i].nlink!=refcount[i]){
                fprintf(stderr, "ERROR: bad reference count for file.\n");
                exit(1);
            }
        }
    }

    //TODO:
    //For blocks marked in-use in bitmap, actually is in-use in an inode or indirect block somewhere. ERROR: bitmap marks block in use but it is not in use.
    for(int i=(ninodes+4);i<size;i++){
        location = i/8;
        offset = i%8;
        if((usage[i]==0) && (bitmap[location]&(1<<(offset)))){
           // printf("usage %d bitmap %d\n",usage[i],(bitmap[location]&(1<<offset)));
           // printf("i=%d loc=%d off=%d\n",i,location,offset);
            fprintf(stderr,"ERROR: bitmap marks block in use but it is not in use.\n");
            exit(1);
        }
    }
    exit(0);
}
