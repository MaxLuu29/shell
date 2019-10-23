#include "get_path.h"

/**
 * custom data structure to hold 
 * the args string array and 
 * the count of arguments 
 */
typedef struct tuple
{
	char **arguments;
	int count;
} tuple_t;

int pid;
// runs the shell with arguments from main
int sh(int argc, char **argv, char **envp);

/**
 * which returns the pathnames of the files (or links) which would be exe‐
 * cuted in the current environment, had its arguments been given as commands
 */
char *which(char *command, char *path);

/**
 * where returns all pathnames of the files (or links) where would be exe‐
 * cuted in the current environment, had its arguments been given as commands
 */
char *where(char *command, char *path);

/**
 * list  information  about  the FILEs (the current directory by default).
 * Sort entries alphabetically if none of -cftuvSUX nor --sort is specified
 */
void list (char *dir);

/**
 * print the values of the specified environment VARIABLE(s).  If no 
 * VARIABLE is specified, print name and value pairs for them all.
 */
void printenv(char **envp);

/**
 * reads and saves user input
 */
char readInput(char *buffer);

/**
 * parses user input string into an array of strings of arguments 
 */
tuple_t *stringToArray(char *input);

/**
 * helper function to free **args
 */
void freeArgs(tuple_t *t);

/**
 * helper function for printing strings
 */
void printString(char *s);

/**
 * helper function for display what commands are being executed
 */
void printExecCommand(int isBuiltin, char *command);

#define PROMPTMAX 32
#define MAXARGS 10
#define BUFFER_SIZE 128
