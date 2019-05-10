#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#define TLB_SIZE 16
#define PAGE_SIZE 256
#define FRAME_SIZE 256
#define PHYSICAL_MEMORY_SIZE PAGE_SIZE*FRAME_SIZE


int logicalAddress = 0;
int offsetNumber = 0;
int pageNumber = 0;
int physicalAddress = 0;
int Frame = 0;
int Value = 0;
int Hit = 0;
int tlbIndex = 0;
int tlbSize = 0;

unsigned pageNumberMask = 65280; //1111111100000000
unsigned offsetMask = 255; //11111111

int tlbHitCount = 0;
float tlbHitRate = 0;
int addressCount = 0;
int pageFaultCount = 0;
float pageFaultRate = 0;

struct tlbTable {
	unsigned int pageNum;
	unsigned int frameNum;
};

int main (int argc, char *argv[])
{
	//Check to see if user inputs addresses.txt
	if (argc != 2)
	{
		fprintf(stderr, "Usage ./VirtualMem_Manager <Filename.txt> \n");
		exit(1);
	}
	//Open addresses.txt, BACKING_STORE.bin, and
	//Create Output.txt to store program results
	FILE *addresses = fopen(argv[1], "r" );
	FILE *BACKINGSTORE = fopen("/Users/Ryan/Downloads/BACKING_STORE.bin", "rb");


	int physicalMemory[PHYSICAL_MEMORY_SIZE];
	char Buffer[256];
	int Index;

	//Declare and initialize pageTable[] array to -1
	int pageTable[PAGE_SIZE];
	memset(pageTable, -1, 256*sizeof(int));

	//Declare and initialize tlb[] structure to -1
	struct tlbTable tlb[TLB_SIZE];
	memset (pageTable, -1, 16*sizeof(char));

	//Read each address from addresses.txt
	while(fscanf(addresses, "%d", &logicalAddress) == 1)
	{
		addressCount++;

		//set the page number and offset for each logical address
		pageNumber = logicalAddress & pageNumberMask;
		pageNumber = pageNumber >> 8;
		offsetNumber = logicalAddress & offsetMask;
		Hit = -1;

		//Check to see if the page number is already in the tlb
		//If it is in tlb, then it is tlb hit
		for(Index = 0; Index < tlbSize; Index++)
		{
			if(tlb[Index].pageNum == pageNumber)
			{
				Hit = tlb[Index].frameNum;
				physicalAddress = Hit*256 + offsetNumber;
			}
		}

		if(!(Hit == -1))
		{
			tlbHitCount++;
		}
		//This "else if" loop is the tlb miss
		//Gets the physical page number from page table
		else if(pageTable[pageNumber] == -1)
		{
			fseek(BACKINGSTORE, pageNumber*256, SEEK_SET);
			fread(Buffer, sizeof(char), 256, BACKINGSTORE);
			pageTable[pageNumber] = Frame;

			for(Index = 0; Index < 256; Index++)
			{
				physicalMemory[Frame*256 + Index] = Buffer[Index];
			}
			pageFaultCount++;
			Frame++;

			//FIFO algorithm for the tlb
			if(tlbSize == 16)
				tlbSize--;

			for(tlbIndex = tlbSize; tlbIndex > 0; tlbIndex--)
			{
				tlb[tlbIndex].pageNum = tlb[tlbIndex-1].pageNum;
				tlb[tlbIndex].frameNum = tlb[tlbIndex-1].frameNum;
			}

			if (tlbSize <= 15)
				tlbSize++;

			tlb[0].pageNum = pageNumber;
			tlb[0].frameNum = pageTable[pageNumber];
			physicalAddress = pageTable[pageNumber]*256 + offsetNumber;
		}
		else
		{
			physicalAddress = pageTable[pageNumber]*256 + offsetNumber;
		}

		//Gets the value from the bin file provided
		Value = physicalMemory[physicalAddress];
		//print the addresses and value to Output.txt
		printf("Virtual Address: %d Physical Address: %d Value: %d \n", logicalAddress, physicalAddress, Value);
	}

	//The statistics of the program
		pageFaultRate = pageFaultCount*1.0f / addressCount;
		tlbHitRate = tlbHitCount*1.0f / addressCount;

//Close files provided for the project
fclose(addresses);
fclose(BACKINGSTORE);

//Print the statistics of the program to Output.txt
printf("Number of Addresses: %d\n", addressCount);
printf("Number of Page Faults: %d\n", pageFaultCount);
printf("Page Fault Rate: %f\n", pageFaultRate);
printf("TLB Hits: %d\n", tlbHitCount);
printf("TLB Hit Rate %f\n", tlbHitRate);



return 0;

}
