#include "file_util.h"
#include "scanner.h"
#include "parser.h"

// global variables
FILE *inputFile;
FILE *outputFile;
FILE *listFile;
FILE *tempFile;

// CSC 460 - Language Translation

void init(int argc, char *argv[]) {
	char filename[CHAR_LIMIT];
	char outname[CHAR_LIMIT];
	int filesOpen;
	
	if(argc == 1)
	{
		filesOpen = openFiles(1, "", "");
	}
	else if(argc == 2)
	{
		strcpy(filename, argv[1]);
		filesOpen = openFiles(2, filename, "");
	}
	else if(argc == 3)
	{
		strcpy(filename, argv[1]);
		strcpy(outname, argv[2]);
		filesOpen = openFiles(3, filename, outname);
	}
	
	if (filesOpen == 1) {
		printf("\n\n\nfiles successfully opened!\n");
		inputFile = getFile(inputFile);
		outputFile = getFile(outputFile);
		listFile = getFile(listFile);
		tempFile = getFile(tempFile);
	}
}

int main(int argc, char *argv[]) 
{
	char tokenBuffer[CHAR_LIMIT * 2];
	
	// initiate program
	init(argc, argv);
	
	// call system_goal to parse the input file
	system_goal(tokenBuffer, inputFile, outputFile, listFile, tempFile);
		

	return 0;
}


