#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>

#define BUFFSIZE 128

typedef struct tuple
{
	char **arguments;
	int count;
} tuple_t;


void printString(char* s) 
{
    printf("%s\n", s);
}

void printNumber(int n)
{
    printf("%d\n", n);
}

char** stringToArray(char* input) 
{
	// make a copy of array
	char buff[BUFFSIZE];
	strcpy(buff, input);
	char *t = strtok(buff, " "); // this returns the first word (if empty string it returns NULL)
	int count;

	// call strtok over and over until you determine how long it is
	while(strtok(NULL, " ")) 
	{
		count++;
	}

	char **argv = malloc((count + 1) * sizeof(char *));
	argv[count] = NULL;

	count = 0;
	strcpy(buff, input);
	t = strtok(buff, " ");

	while(t) 
	{
		int len = strlen(t);
		argv[count] = (char*)malloc((len + 1) * sizeof(char));
		strcpy(argv[count], t);
		count++;
		t = strtok(NULL, " ");
	}

	return argv;
}

char *which(char *command, char *path)
{
	char *delim = ":";
	char *result = NULL;

	char copyPath[strlen(path)];
	strcpy(copyPath, path);
	if (strcmp(command, "") != 0)
	{
		result = strtok( copyPath, delim );
		while (result != NULL)
		{
			char pathOfCommand[strlen(result)+strlen(command)];
			sprintf(pathOfCommand, "%s/%s", result, command);
			if (access(pathOfCommand, F_OK) == 0)
			{
				char *returnString = calloc(strlen(pathOfCommand)+1, sizeof(char));
				strncpy(returnString, pathOfCommand, strlen(pathOfCommand));
				return returnString;
			}
			result = strtok( NULL, delim );
		}
	}
	return NULL;
}

int main(int argc, char const *argv[])
{
	char *s = getenv("PATH");
	char *cmd = "ls";

	// printString(which(cmd, s));
	if (which("a", s) != NULL)
	{
		printString(":P");
	} else
	{
		printString("NULL");
	}
	// printString(which("mkdir", s));
	// printString(which("rm", s));
	// printString(which("kill", s));
	// printString(which("nice", s));
	// printString(which("which", s));

    return 0;
}