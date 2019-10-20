#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

void printString(char *s)
{
	printf("%s\n", s);
}

int sh(int argc, char **argv, char **envp)
{
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd;
	char **args = calloc(MAXARGS, sizeof(char *));
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	struct pathelement *pathlist;

	uid = getuid();
	password_entry = getpwuid(uid);   /* get passwd info */
	homedir = password_entry->pw_dir; /* Home directory to start
						  out with*/

	if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
	{
		perror("getcwd");
		exit(2);
	}

	owd = calloc(strlen(pwd) + 1, sizeof(char));
	memcpy(owd, pwd, strlen(pwd));
	prompt = "";

	/* Put PATH into a linked list */
	pathlist = get_path();

	char buffer[BUFFER_SIZE];
	int len;
	// char *path = getevn("PATH");
	char *environmentString = getenv("PATH");

	while (go)
	{
		/* print your prompt */
		printf("%s [%s]> ", prompt, pwd);
		len = readInput(buffer);
		/* get command line and process */

		if (len > 1)
		{
			char *commandline = calloc(len, sizeof(char));
			strcpy(commandline, buffer);
			tuple_t *tuple = stringToArray(commandline);

			// /* check for each built in command and implement */
			if (strcmp(tuple->arguments[0], "exit") == 0)
			{
				exit(0);
			}
			else if (strcmp(tuple->arguments[0], "pid") == 0)
			{
				printf("%d\n", (int)getpid());
			}
			else if (strcmp(tuple->arguments[0], "prompt") == 0)
			{
				printf("\tInput prompt prefix: ");
				int length = readInput(buffer);
				printString(buffer);
			}
			else if (strcmp(tuple->arguments[0], "where") == 0)
			{
				where(tuple->arguments[1], environmentString);
			}
			else
			{
				/*  else  program to exec */
				char *commandStr = which(tuple->arguments[0], environmentString);
				if (commandStr != NULL)
				{
					/* find it */
					/* do fork(), execve() and waitpid() */
					/* else */
					/* fprintf(stderr, "%s: Command not found.\n", args[0]); */

					pid_t pid;
					int status;

					if ((pid = fork()) == 0)
					{
						execve(commandStr, tuple->arguments, envp);
						perror("exec failed");
						exit(2);
					}
					else
					{
						waitpid(pid, &status, 0);
					}
				}
				else
				{
					printf("command not found\n");
				}
			}
		}
	}
	return 0;
} /* sh() */

char *which(char *command, char *path)
{
	/* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
	char *delim = ":";
	char *result = NULL;

	char copyPath[strlen(path)];
	strcpy(copyPath, path);
	if (strcmp(command, "") != 0)
	{
		result = strtok(copyPath, delim);
		while (result != NULL)
		{
			char pathOfCommand[strlen(result) + strlen(command)];
			sprintf(pathOfCommand, "%s/%s", result, command);
			if (access(pathOfCommand, X_OK) == 0)
			{
				char *returnString = calloc(strlen(pathOfCommand) + 1, sizeof(char));
				strncpy(returnString, pathOfCommand, strlen(pathOfCommand));
				return returnString;
			}
			result = strtok(NULL, delim);
		}
	}
	return NULL;

} /* which() */

char *where(char *command, char *path)
{
	/* similarly loop through finding all locations of command */
	char *delim = ":";
	char *result = NULL;

	char copyPath[strlen(path)];
	strcpy(copyPath, path);
	if (strcmp(command, "") != 0)
	{
		result = strtok(copyPath, delim);
		while (result != NULL)
		{
			char pathOfCommand[strlen(result) + strlen(command) + 1];
			sprintf(pathOfCommand, "%s/%s", result, command);

			if (access(pathOfCommand, F_OK) == 0)
			{
				printf("%s\n", pathOfCommand);
				break;
			}

			result = strtok(NULL, delim);
		}
	}
	return NULL;
} /* where() */

void list(char *dir)
{
	/* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */

char readInput(char *buffer)
{
	char length = 0;
	if (fgets(buffer, BUFFER_SIZE, stdin) != 0)
	{
		length = strlen(buffer);
		buffer[length - 1] = '\0';
	}
	return length;
}

tuple_t *stringToArray(char *input)
{
	tuple_t *tup = malloc(sizeof(tuple_t));

	// make a copy of array
	char buff[BUFFER_SIZE] = "";
	strcpy(buff, input);
	char *t = strtok(buff, " "); // this returns the first word (if empty string it returns NULL)
	int count = 0;

	// call strtok over and over until you determine how long it is
	while (strtok(NULL, " "))
	{
		count++;
	}

	char **argv = malloc((count + 2) * sizeof(*argv));
	argv[count + 1] = NULL;

	count = 0;
	strcpy(buff, input);
	t = strtok(buff, " ");

	while (t)
	{
		int len = strlen(t);
		argv[count] = (char *)malloc((len + 1) * sizeof(char));
		strcpy(argv[count], t);
		count++;
		t = strtok(NULL, " ");
	}

	tup->arguments = argv;
	tup->count = count;

	return tup;
}
