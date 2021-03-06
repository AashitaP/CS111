//NAME: Divij Ohri, Aashita Patwari
//EMAIL: divijohri28@gmail.com, harshupatwari@gmail.com


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "ext2_fs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <unistd.h>


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
int accumulator = 0;


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
    if(groupsDesc == NULL || groupsNumBlocks == NULL || groupsNumInodes == NULL)
    {
        fprintf(stderr, "Failed to allocate memory");
        exit(1);
    }
    ssize_t nBytes = pread(fd, groupsDesc, sizeof(struct ext2_group_desc) * numberGroups, 1024 + sizeof(struct ext2_super_block));
    if (nBytes == -1)
    {
        fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
        exit(1);
    }
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

            printf("GROUP,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", (unsigned long) i, numberBlocks, numberInodes,freeBlocks,freeInodes,bitmap, inodemap, firstInode);
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
        if(bitmapBuffer == NULL)
        {
            fprintf(stderr, "Failed to allocate memory");
            exit(1);
        }
        unsigned long bitmap = groupsDesc[i].bg_block_bitmap;
        ssize_t nBytes = pread(fd, bitmapBuffer, blockSize, bitmap * blockSize);
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
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
        if(inodeBuffer ==   NULL)
        {
            fprintf(stderr, "Failed to allocate memory");
            exit(1);
        }
        unsigned long inodemap = groupsDesc[i].bg_inode_bitmap;
        ssize_t nBytes = pread(fd, inodeBuffer, blockSize, inodemap * blockSize);
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
        int j; 
        int mask = 0x1;
        int i_ninodes = (int) numberInodes;
        
        for(j = 0; j < i_ninodes; j++)
        {
            __u8 byte = inodeBuffer[j/8];
            if((j % 8) == 0 && (byte & mask) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 1 && (byte & (mask << 1)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 2 && (byte & (mask << 2)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 3 && (byte & (mask << 3)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 4 && (byte & (mask << 4)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 5 && (byte & (mask << 5)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 6 && (byte & (mask << 6)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
            else if((j % 8) == 7 && (byte & (mask << 7)) == 0)
                printf("IFREE,%lu\n", (unsigned long) j+1);
        } 
        blockOffset += numberBlocks;
    }
}


void handleBlockReferences (int off, int p_inum, int indbl1, int indbl2, int indbl3, struct ext2_inode *cur)
{
    __u8 *block_holder;
    block_holder = malloc(blockSize);
    if(block_holder == NULL)
    {
        fprintf(stderr, "Failed to allocate memory");
        exit(1);
    }
    int firstindblock = blockSize * indbl1;
    int secindblock = blockSize * indbl2;
    int thirdindblock = blockSize * indbl3;
    
    if (indbl1)
    {
        ssize_t nBytes = pread(fd, block_holder, blockSize, firstindblock);
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
        unsigned long counter = 0;
        
        if (off)
            counter = off + accumulator - 1;
        else
            counter = accumulator + 12;
        
        int i = 0;
        int i_bs = (int) blockSize;
        
        while (i < i_bs)
        {
            if (block_holder[i] != 0)
            {
                printf("INDIRECT,%lu,%lu,%lu,%lu,%lu\n",(unsigned long) p_inum,(unsigned long)1,counter,(unsigned long)indbl1, (unsigned long)block_holder[i]);
                counter = counter+1;
                
                if (off == 0)
                    accumulator++;
            }
            i++;
        }
    }
    
    if (indbl2)
    {
        ssize_t nBytes = pread (fd, block_holder, blockSize, secindblock);
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
        unsigned long counter = 0;
        
        if (off)
            counter = off;
        else
            counter = 268;
        
        int i = 0;
        int i_bs = (int) blockSize;
        
        while (i < i_bs)
        {
            if (block_holder[i] != 0)
            {
                printf("INDIRECT,%lu,%lu,%lu,%lu,%lu\n",(unsigned long)p_inum, (unsigned long)2,counter,(unsigned long)indbl2, (unsigned long)block_holder[i]);
                handleBlockReferences(counter, p_inum, block_holder[i], 0, 0, cur);
                counter += 256;
            }
            i++;
        }
    }
    
    if (indbl3)
    {
        ssize_t nBytes = pread(fd, block_holder, blockSize, thirdindblock);
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
        unsigned long counter = 0;
        
        if (off)
            counter = off;
        else
            counter = 65804;
        
        int i = 0;
        int i_bs = (int) blockSize;
        
        while (i < i_bs)
        {
            if (block_holder[i] != 0)
            {
                printf("INDIRECT,%lu,%lu,%lu,%lu,%lu\n",(unsigned long)p_inum,(unsigned long)3,counter, (unsigned long)indbl3, (unsigned long)block_holder[i]);
                handleBlockReferences(counter, p_inum, 0, block_holder[i], 0, cur);
                counter += 65536;
            }
            i++;
        }
    }
    
    free(block_holder);
}

void directoryEntries (int p_inum, struct ext2_inode *pass_inode)
{
    int temp = pass_inode->i_blocks/(2 << superBlock.s_log_block_size);
    int i=0;
    
    while (i < temp+1)
    {
        int inode_factor = pass_inode->i_block[i]-1;
        if (i > 14)
        {
            fprintf (stderr, "An error occurred in the directoryEntries function.\n");
            exit(1);
        }
        
        char holder[blockSize];
        ssize_t nBytes = pread(fd, &holder, blockSize, 1024 + (blockSize * inode_factor));
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
        struct ext2_dir_entry *pointer = (struct ext2_dir_entry *)holder;
        int cSize = 0;
        int i_bs = (int) blockSize;
        
        while (cSize < i_bs)
        {
            if (pointer->file_type)
            {
                char nameOfFile[pointer->name_len+1];
                
                int x = 0;
                while (x < pointer->name_len)
                {
                    nameOfFile[x] = pointer->name[x];
                    x++;
                }
                
                nameOfFile[pointer->name_len] = 0;
                if (pointer != NULL)
                    printf("DIRENT,%lu,%lu,%lu,%lu,%lu,'%s'\n", (unsigned long) p_inum, (unsigned long) cSize, (unsigned long) pointer->inode, (unsigned long) pointer->rec_len,(unsigned long) pointer->name_len, nameOfFile);
                
                cSize += pointer->rec_len;
                pointer = pointer->rec_len + (void*)pointer;
            }
            
            else
                break;
        }
        
        i++;
    }
}

void scanInodes()
{
    int i;
    unsigned long tableSize = inodesPG * inodeSize;
    for(i = 0; i < numberGroups; i++)
    {
        inodeTable = malloc(tableSize);
        if(inodeTable == NULL)
        {
            fprintf(stderr, "Failed to allocate memory");
            exit(1);
        }
        ssize_t nBytes = pread(fd, inodeTable, tableSize, blockSize * groupsDesc[i].bg_inode_table);
        if (nBytes == -1)
        {
            fprintf(stderr, "Error number: %d, Error message: %s \n", errno, strerror(errno));
            exit(1);
        }
        int j;
        int i_pg = (int) inodesPG;
        
        for(j = 0; j < i_pg; j++)
        {
            if(inodeTable[j].i_mode != 0 && inodeTable[j].i_links_count != 0)
            {
                //symbolic 1010 regular file 1000
                char fileType = '?';
                if (inodeTable[j].i_mode & 0x8000)
                {
                    if(inodeTable[j].i_mode & 0x2000)
                    {
                        fileType = 's';
                    }
                    else
                    {
                        fileType = 'f';
                        struct ext2_inode *pass_inode = &inodeTable[j];
                        handleBlockReferences (0, j+1, inodeTable[j].i_block[12], inodeTable[j].i_block[13], inodeTable[j].i_block[14], pass_inode);
                    }
                }
                else if (inodeTable[j].i_mode & 0x4000)
                {
                    fileType = 'd';
                    struct ext2_inode *pass_inode = &inodeTable[j];
                    handleBlockReferences (0, j+1, inodeTable[j].i_block[12], inodeTable[j].i_block[13], inodeTable[j].i_block[14], pass_inode);
                    directoryEntries (j+1, pass_inode);
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

                time_t c_seconds = inodeTable[j].i_ctime;
                ts = *gmtime(&c_seconds);
                strftime(cTime, 80,"%m/%d/%y %H:%M:%S", &ts);



                printf("INODE,%lu,%c,%o,%lu,%lu,%lu,%s,%s,%s,%lu,%lu", (unsigned long) j + 1,fileType,(inodeTable[j].i_mode) & 0xFFF,(unsigned long) inodeTable[j].i_uid, (unsigned long) inodeTable[j].i_gid, (unsigned long) inodeTable[j].i_links_count, cTime, mTime, aTime, (unsigned long) inodeTable[j].i_size,(unsigned long) inodeTable[j].i_blocks);

                if(fileType == 's')
                {
                    printf(",%lu\n", (unsigned long) inodeTable[j].i_block[0]);
                }
                else
                    printf(",%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",(unsigned long) inodeTable[j].i_block[0], (unsigned long) inodeTable[j].i_block[1], (unsigned long) inodeTable[j].i_block[2],(unsigned long) inodeTable[j].i_block[3],(unsigned long) inodeTable[j].i_block[4],(unsigned long) inodeTable[j].i_block[5],(unsigned long) inodeTable[j].i_block[6], (unsigned long) inodeTable[j].i_block[7],(unsigned long) inodeTable[j].i_block[8], (unsigned long) inodeTable[j].i_block[9],(unsigned long) inodeTable[j].i_block[10],(unsigned long) inodeTable[j].i_block[11],(unsigned long) inodeTable[j].i_block[12],(unsigned long) inodeTable[j].i_block[13],(unsigned long) inodeTable[j].i_block[14]);
            }

        }

        free(inodeTable);
    }
}

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Usage: Require one argument - an image file");
        exit(1);
    }

    fileName = argv[1];

    fd = open(fileName, O_RDONLY);

    if(fd == -1)
    {
        fprintf(stderr, "Error opening image");
        exit(1);
    }

    printSuper();
    printGroupDesc();
    printFreeBlock();
    printFreeInodes();
    scanInodes();

    exit(0);


}
