#include "file_util.h"

// start_up(FILE *, FILE *, FILE *) ; initialized program returns infile, outfile, listing file

FILE *inputFile;
FILE *outputFile;
FILE *listFile;
FILE *tempFile;
FILE *thebackupFile;
char tempFilename[CHAR_LIMIT];
char inputName[CHAR_LIMIT];

FILE * getFile(FILE *name) {
	return name;
}

//openfiles function takes three parameters (number of arguments user enters in the command line, Input file name, output file name)
//if one argument this indicates no filenames were entered function to get Inputfilenames is called.
//if two arguments this indicates one filename entered.  This will defult be the input file and the function to check for existance is called.
//if three arguments this indicates two filenames were entered function to check for existance is called
int openFiles(int args, char *filename, char *outname) 
{
	int stat;
	int statout;
	int statret = 0;
	if (args == 1) 
	{
		stat = getInFile();
	}
	else if (args == 2) 
	{
		stat = checkIFile(filename, "");
	} 
	else 
	{
		while(strcmp(filename, outname) == 0)
		{
			printf("Input file and output file cannot be the same.\nPlease enter new output file name:\n");
			gets(outname);
		}
		stat = checkIFile(filename, outname);
	}
	if (stat == 1) {
		statout = checkOutput(outname, filename);
		if (statout == 1) {
			statret = 1;
		}
	}
	return statret;
}


// getInFile will simply ask the user for an input filename if none is entered in the command line
// getInFile then calls the function to check if input file exists
int getInFile()
{
	char inp[CHAR_LIMIT];
	int stat;

	printf("Please enter the name of the input file:\n");
	gets(inp);
	stat = checkIFile(inp, "");
	return stat;
}


// changeExt function takes two parameters (file name, integer type)
char * changeExt(char *inp, int type) {
	char newName[CHAR_LIMIT];
	int i=0;
	
	while((inp[i] != '.') && (inp[i] != '\0')) {
			i++;
		}
	if(inp[i] != '.') {
		if (type == 1) {
			strcpy(newName, inp);
			strcat(newName, IN_EXTENSION);
			rename(inp, newName);
			strcat(inp, IN_EXTENSION);
		} else if (type == 2) {
			strcat(inp, OUT_EXTENSION);
		} else if (type == 3) {
			strcat(inp, LIST_EXTENSION);
		}
		}
	return inp;
}


int checkIFile(char *inFile, char* oFile)
{
	int exists = 0;
	char contents;
	char outt[CHAR_LIMIT];
	int status;
	
	
	if (inFile[0] == ' ' || inFile[0] == '\0' || inFile[0] == '\n' || inFile[0] == '\t' || inFile == NULL) 
	{
	printf("No file, No program!!");
	status = 0;
	}
	else 
	{
	
				
	strcpy(inFile, changeExt(inFile, 1));
	inputFile = fopen(inFile, "r+");
	if(inputFile != NULL)
		{
			printf("\n\nINPUT FILE:  %s\n", inFile);
			printf("Input File successfully opened.\n\n");
			exists = 1;
			status = 1;	
			
		}
		
		else
		{
			fclose(inputFile);
			do
			{
				printf("Input File Does Not Exist.\nPlease enter a new file:");
				gets(inFile);
				if (inFile[0] == ' ' || inFile[0] == '\0' || inFile[0] == '\n' || inFile[0] == '\t' || inFile == NULL) 
				{
				printf("No file, No program!!");
				exists = 1;
				status = 0;
				}
				else {
				strcpy(inFile, changeExt(inFile, 1));
				if(inputFile = fopen(inFile, "r+"))
				{
					printf("\n\nINPUT FILE:  %s\n", inFile);
					printf("Input File successfully opened.\n\n");
					exists = 1;
					status = 1;
				}
				}
			}
			while(exists==0);
			
		}
	}
	strcpy(inputName, inFile);
	return status;
}


void *fileCopy() 
{
	char content;
	content = fgetc(inputFile);
	while (content != EOF) 
	{
		fputc(content, outputFile);
		content = fgetc(inputFile);
	}
	
	printf("\n\n------------------------------------\n");
	printf("| Contents of input file have been |\n|    transferred to output file.   |\n");
	printf("------------------------------------");
}

void *backupFile(char *back)
{
	char c;
	thebackupFile = fopen(back, "w");
	
	c=fgetc(outputFile);
	while(c != EOF)
	{
		fputc(c, thebackupFile);
		c=fgetc(outputFile);
		
	}
	
	printf("\n**Backup file created.**");
	
}

void *makeListFile(char *inp1, char *inp2) {
	char *delim;
	strcpy(inp1, inp2);
	delim = strtok(inp1, ".");
	strcpy(inp1, changeExt(inp1, 3));
	listFile = fopen(inp1, "w+");
	
}

int checkOutput(char *output, char *inputFilename)
{
	FILE* backupOutput;
	char outty[CHAR_LIMIT];
	char backout[CHAR_LIMIT];
	char listFilename[CHAR_LIMIT];
	char *delim;
	char answer[CHAR_LIMIT];
	int status;
	char temp[CHAR_LIMIT];
	int i;
	
	/*if (output == "" || output[0] == '\0') 
	{
		printf("Please enter the output file:  \n");
		gets(outty);
		while(strcmp(inputFilename, outty) == 0)
		{
			printf("Input file and output file cannot be the same.\nPlease enter new output file name:\n");
			gets(outty);
		}
		if (outty[0] == ' ' ||  outty[0] == '\0' || outty[0] == '\n' || outty[0] == '\t' || outty == NULL) 
		{
			printf("No file, No program!!");
			status = 0;
			return status;
		}
		else
		{
			strcpy(outty, changeExt(outty, 2));
		}
		
	}
	else
	{
		strcpy(outty, output);
		strcpy(outty, changeExt(outty, 2));
	}*/
	
	
	if(access(outty, W_OK) != -1)
	{
		printf("\nOutput file already exists.  \n--*Please enter the letter 'O' to overwrite OR ENTER to quit.\n");
		gets(answer);
		/*while(strcmp(inputFilename, answer) == 0)
		{
			printf("Input file and output file cannot be the same.\nPlease enter new output file name:\n");
			gets(answer);
		}
		if (answer[0] == ' ' ||  answer[0] == '\0' || answer[0] == '\n' || answer[0] == '\t' || answer == NULL) 
		{
			printf("No file, No program!!");
			status = 0;
			return status;
		}*/
		if(answer[0] == 'o' && strlen(answer) == 1|| answer[0] == 'O' && strlen(answer) == 1)
		{
			
			
    	i = 0;
		while((inputName[i] != '.') && (inputName[i] != '\0')) {
			i++;
		}
		inputName[i] = '\0';
		strcat(inputName, C_EXTENSION);
	
    	strcpy(outty, inputName);
		strcpy(backout, outty);
		strcat(backout, BACKUP_EXTENSION);
		outputFile=fopen(outty, "r");
		backupFile(backout);
		fclose(outputFile);
		fclose(thebackupFile);
		status = 1;
		}
		/*else if (answer[0] == ' ' ||  answer[0] == '\0' || answer[0] == '\n' || answer[0] == '\t' || answer == NULL) 
		{
				printf("No file, No program!!");
				status = 0;
				
					
		}
		else {
			checkOutput(answer, inputFilename);
	}*/
    }
    
    if (status != 1) {
    	i = 0;
		while((inputName[i] != '.') && (inputName[i] != '\0')) {
			i++;
		}
		inputName[i] = '\0';
		strcat(inputName, C_EXTENSION);
	
    	strcpy(outty, inputName);
	}

	outputFile = fopen(outty, "w");
	printf("\n\nOUTPUT FILE:  %s \n", outty);
	printf("Output File successfully opened.");
	status = 1;
	makeListFile(listFilename, outty);
	// make temp file
	strcpy(tempFilename, outty);
	strcat(tempFilename, TEMP_EXTENSION);
	tempFile = fopen(tempFilename, "w+");
	status = 1;
	
	return status;
}

void wrapUp()
{
	
	fclose(inputFile);
	fclose(outputFile);
	fclose(listFile);
	fclose(tempFile);
	remove(tempFilename);
	printf("\n\n***Files successfully Closed.***");
}

