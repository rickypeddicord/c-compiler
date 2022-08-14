#ifndef FILE_UTIL_H_
#define FILE_UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <io.h>
#include <sys/stat.h>

#ifndef CHAR_LIMIT
#define CHAR_LIMIT 50
#endif

#ifndef IN_EXTENSION
#define IN_EXTENSION (".in")
#endif

#ifndef OUT_EXTENSION
#define OUT_EXTENSION (".out")
#endif

#ifndef C_EXTENSION
#define C_EXTENSION (".c")
#endif

#ifndef BACKUP_EXTENSION
#define BACKUP_EXTENSION (".bak")
#endif

#ifndef LIST_EXTENSION
#define LIST_EXTENSION (".lis")
#endif

#ifndef TEMP_EXTENSION
#define TEMP_EXTENSION (".temp")
#endif


// function declarations 
int openFiles(int args, char *filename, char *outname);
int getInFile();
int checkIFile(char *inFile, char *outFile);
char * changeExt(char *inp, int type);
void *fileCopy();
void *backupFile(char *back);
void *makeListFile(char *inp1, char *inp2);
int checkOutput(char *output, char *inputFile);
void wrapUp();
FILE *getFile(FILE *name);


#endif


