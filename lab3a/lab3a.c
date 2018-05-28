#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ext2_fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>


#define OFFSET = 1024; //starts from byte 1024
struct ext2_super_block superBlock;
struct ext2_group_desc* groupsDesc = NULL;
struct ext2_inode* inodeTable = NULL;
char* fileName = NULL;
int fd;
unsigned long numBlocks = 0;
unsigned long blocksPG = 0;
unsigned long numInodes = 0;
unsigned long inodesPG = 0;
int numberGroups = 0;
unsigned long* groupsNumBlocks = NULL;
unsigned long* groupsNumInodes = NULL;
unsigned long blockSize = 0;
unsigned long inodeSize = 0;


void printSuper() //total number of blocks(4), inodes(0), block size (24), inode size (88), blocks per group (32), inodes per group (40), first non reserved inode (84)
{

    ssize_t nBytes = pread(fd, &superBlock, sizeof(struct ext2_super_block), 1024);
    if (nBytes == -1)
    {
        fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
        exit(1);
    }

    numBlocks = superBlock.s_blocks_count;
    numInodes = superBlock.s_inodes_count;
    blockSize = 1024 << superBlock.s_log_block_size;
    inodeSize = superBlock.s_inode_size;
    blocksPG = superBlock.s_blocks_per_group;
    inodesPG = superBlock.s_inodes_per_group;
    unsigned long firstNon = superBlock.s_first_ino;



//block size = 1024 << s_log_block_size;
    printf("SUPERBLOCK,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",numBlocks, numInodes, blockSize, inodeSize, blocksPG, inodesPG, firstNon);


}

void printGroupDesc() //number blocks, inodes, freeblocks, free inodes, block number of free block bitmap, ...inode, block number of first block of inodes
{
    numberGroups = 1 + (numBlocks - 1)/blocksPG; //is it because rounding?
    int i;
    groupsDesc = malloc(numberGroups * sizeof(struct ext2_group_desc));
    groupsNumBlocks = malloc(numberGroups * sizeof(unsigned long));
    groupsNumInodes = malloc(numberGroups * sizeof(unsigned long));
    if(groupsDesc == NULL)
    {
        fprintf(stderr, "Failed to allocate memory");
    }
    ssize_t nBytes = pread(fd, groupsDesc, sizeof(struct ext2_group_desc) * numberGroups, 1024 + sizeof(struct ext2_super_block));
    unsigned long remainingBlocks = numBlocks;
    unsigned long remainingInodes = numInodes;
    for (i = 0; i < numberGroups; i++)
    {
        unsigned long numberInodes;
        unsigned long numberBlocks;
        if (remainingInodes < inodesPG)
        {
            numberInodes = remainingInodes;
        }
        else 
        {
            numberInodes = inodesPG;
            remainingInodes -= inodesPG;
        }

        groupsNumInodes[i] = numberInodes;

        if (remainingBlocks < blocksPG)
        {
            numberBlocks = remainingBlocks;
        }
        else 
        {
            numberBlocks = blocksPG;
            remainingBlocks -= blocksPG;
        }
        groupsNumBlocks[i] = numberBlocks;
        unsigned long freeBlocks = groupsDesc[i].bg_free_blocks_count;
        unsigned long freeInodes = groupsDesc[i].bg_free_inodes_count;
        unsigned long bitmap = groupsDesc[i].bg_block_bitmap;
        unsigned long inodemap = groupsDesc[i].bg_inode_bitmap;
        unsigned long firstInode = groupsDesc[i].bg_inode_table;	

            printf("GROUP,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", i, numberBlocks, numberInodes,freeBlocks,freeInodes,bitmap, inodemap, firstInode);
    }
}

void printFreeBlock()
{
    int i;
    unsigned long blockOffset = 1;
    for(i = 0; i < numberGroups; i++)
    {
        unsigned long numberBlocks = groupsNumBlocks[i];
        __u8 *bitmapBuffer = malloc(blockSize); //bitmap is one block
        unsigned long bitmap = groupsDesc[i].bg_block_bitmap;
        ssize_t nBytes = pread(fd, bitmapBuffer, blockSize, bitmap * blockSize);
        unsigned long j; 
        int mask = 0x1;
        for(j = 0; j < numberBlocks; j++)
        {
            __u8 byte = bitmapBuffer[j/8];
            if((j % 8) == 0 && (byte & mask) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 1 && (byte & (mask << 1)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 2 && (byte & (mask << 2)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 3 && (byte & (mask << 3)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 4 && (byte & (mask << 4)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 5 && (byte & (mask << 5)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 6 && (byte & (mask << 6)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
            else if((j % 8) == 7 && (byte & (mask << 7)) == 0)
                printf("BFREE,%lu\n", j+blockOffset);
        } 
        blockOffset += numberBlocks;
    }

}

void printFreeInodes()
{
    int i;
    unsigned long blockOffset = 1;
    for(i = 0; i < numberGroups; i++)
    {
        unsigned long numberBlocks = groupsNumBlocks[i];
        unsigned long numberInodes = groupsNumInodes[i];
        __u8 *inodeBuffer = malloc(blockSize); //bitmap is one block
        unsigned long inodemap = groupsDesc[i].bg_inode_bitmap;
        ssize_t nBytes = pread(fd, inodeBuffer, blockSize, inodemap * blockSize);
        unsigned long j; 
        int mask = 0x1;
        for(j = 0; j < numberInodes; j++)
        {
            __u8 byte = inodeBuffer[j/8];
            if((j % 8) == 0 && (byte & mask) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 1 && (byte & (mask << 1)) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 2 && (byte & (mask << 2)) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 3 && (byte & (mask << 3)) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 4 && (byte & (mask << 4)) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 5 && (byte & (mask << 5)) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 6 && (byte & (mask << 6)) == 0)
                printf("IFREE,%lu\n", j+1);
            else if((j % 8) == 7 && (byte & (mask << 7)) == 0)
                printf("IFREE,%lu\n", j+1);
        } 
        blockOffset += numberBlocks;
    }
}

void scanInodes()
{
    int i;
    unsigned long tableSize = inodesPG * inodeSize;
    for(i = 0; i < numberGroups; i++)
    {
        inodeTable = malloc(tableSize);
        ssize_t nBytes = pread(fd, inodeTable, tableSize, blockSize * groupsDesc[i].bg_inode_table);
        int j;
        for(j = 0; j < inodesPG; j++)
        {
            if(inodeTable[j].i_mode != 0 && inodeTable[j].i_links_count != 0)
            {
                //symbolic 1010 regular file 1000
                char fileType = '?';
                if(inodeTable[j].i_mode & 0x8000)
                {
                    if(inodeTable[j].i_mode & 0x2000)
                    {
                        fileType = 's';
                    }
                    else
                    {
                        fileType = 'f';
                    }
                }
                else if(inodeTable[j].i_mode & 0x4000)
                {
                    fileType = 'd';
                }

                char cTime[80];
                char mTime[80];
                char aTime[80];

                time_t m_seconds = inodeTable[j].i_mtime;
                struct tm ts;
                ts = *gmtime(&m_seconds);
                strftime(mTime, 80,"%m/%d/%y %H:%M:%S", &ts);

                time_t a_seconds = inodeTable[j].i_atime;
                ts = *gmtime(&a_seconds);
                strftime(aTime, 80,"%m/%d/%y %H:%M:%S", &ts);


                printf("INODE,%lu,%c,%o,%lu,%lu,%lu,%s,%s,%s,%lu,%lu", j + 1,fileType,(inodeTable[j].i_mode) & 0xFFF,inodeTable[j].i_uid,inodeTable[j].i_gid,inodeTable[j].i_links_count, mTime, mTime, aTime, inodeTable[j].i_size, inodeTable[j].i_blocks);

                if(fileType == 's')
                {
                    printf(",%lu\n", inodeTable[j].i_block[0]);
                }
                else
                    printf(",%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",inodeTable[j].i_block[0],inodeTable[j].i_block[1],inodeTable[j].i_block[2],inodeTable[j].i_block[3], inodeTable[j].i_block[4], inodeTable[j].i_block[5], inodeTable[j].i_block[6], inodeTable[j].i_block[7], inodeTable[j].i_block[8], inodeTable[j].i_block[9], inodeTable[j].i_block[10], inodeTable[j].i_block[11], inodeTable[j].i_block[12], inodeTable[j].i_block[13], inodeTable[j].i_block[14]);
            }

        }

        free(inodeTable);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Require one argument - an image file");
        exit(1);
    }

    fileName = argv[1];

    fd = open(fileName, O_RDONLY);

    if(fd == -1)
    {
        fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
        exit(1);
    }

    printSuper();
    printGroupDesc();
    printFreeBlock();
    printFreeInodes();
    scanInodes();


}