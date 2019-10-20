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
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	// struct pathelement *pathlist;

	uid = getuid();
	password_entry = getpwuid(uid);   /* get passwd info */
	homedir = password_entry->pw_dir; /* Home directory to start out with*/

	if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
	{
		perror("getcwd");
		exit(2);
	}

	owd = calloc(strlen(pwd) + 1, sizeof(char));
	memcpy(owd, pwd, strlen(pwd));

	/* Put PATH into a linked list */
	// pathlist = get_path();

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
				free(prompt);
				free(pwd);
				free(owd);
				free(commandline);
				freeArgs(tuple);
				exit(0);
			}
			else if (strcmp(tuple->arguments[0], "pid") == 0)
			{
				printf("%d\n", (int)getpid());
				freeArgs(tuple);
				free(commandline);
			}
			else if (strcmp(tuple->arguments[0], "prompt") == 0)
			{
				if (tuple->count == 1)
				{
					printf("\tInput prompt prefix: ");
					int length = readInput(buffer);
					if (length >= PROMPTMAX)
					{
						perror("invalid size");
					}
					else
					{
						strcpy(prompt, buffer);
					}
					freeArgs(tuple);
					free(commandline);
				}
				else if (tuple->count == 2)
				{
					if (strlen(tuple->arguments[1]) >= PROMPTMAX)
					{
						perror("invalid size");
					}
					else
					{
						strcpy(prompt, tuple->arguments[1]);
					}
					freeArgs(tuple);
					free(commandline);
				}
				else
				{
					printf("too many arguments for prompt\n");
					freeArgs(tuple);
					free(commandline);
				}
			}
			else if (strcmp(tuple->arguments[0], "where") == 0)
			{
				if (tuple->arguments[1] != NULL)
				{
					int i = 1;
					while (tuple->arguments[i] != NULL)
					{
						where(tuple->arguments[i], environmentString);
						i++;
					}
				}
				freeArgs(tuple);
				free(commandline);
			}
			else if (strcmp(tuple->arguments[0], "printenv") == 0)
			{
				if (tuple->count == 1)
				{
					for (char **env = envp; *env != 0; env++)
					{
						char *curEnv = *env;
						printString(curEnv);
					}
				}
				else if (tuple->count == 2)
				{
					char *environmentFromGetEnv = getenv(tuple->arguments[1]);
					if (environmentFromGetEnv != NULL)
					{
						printString(environmentFromGetEnv);
					}
					else
					{
						perror("path does not exist");
					}
				}
				else
				{
					perror("too many arguments");
				}
				freeArgs(tuple);
				free(commandline);
			}
			else if (strcmp(tuple->arguments[0], "setenv") == 0)
			{
			}
			else if (strcmp(tuple->arguments[0], "kill") == 0)
			{
				if (tuple->count == 1 || tuple->count > 3)
				{
					perror("invalid number of arguments");
					free(commandline);
					freeArgs(tuple);
				}
				else if (tuple->count == 2)
				{
					int pid1 = atoi(tuple->arguments[1]);
					free(prompt);
					free(pwd);
					free(owd);
					free(commandline);
					freeArgs(tuple);
					int killStatus;
					if ((killStatus = kill(pid1, SIGTERM)) != 0)
					{
						perror("2");
					}
				}
				else if (tuple->count == 3)
				{
					int pid1 = atoi(tuple->arguments[1]);
					int signal = atoi(tuple->arguments[2]);
					// free(prompt);
					// free(pwd);
					// free(owd);
					// free(commandline);
					// freeArgs(tuple);
					int killStatus;
					if ((killStatus = kill(pid1, signal)) != 0)
					{
						perror("3");
					}
				}
			}

			else if (strcmp(tuple->arguments[0], "list") == 0)
			{
				if (tuple->count == 1)
				{
					printf("current directory\n");
					list(".");
				}
				else
				{
					int i = 1;
					while (tuple->arguments[i] != NULL)
					{
						printf("%s:\n", tuple->arguments[i]);
						list(tuple->arguments[i]);
						printf("\n");
						i++;
					}
				}
				freeArgs(tuple);
				free(commandline);
			}

			else if (strcmp(tuple->arguments[0], "cd") == 0)
			{
				if (tuple->count > 2)
				{
					perror("cd: too many arguments");
				}
				else
				{
					if (tuple->count == 1)
					{
						chdir(homedir);
						free(pwd);
						if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
						{
							perror("getcwd");
							exit(2);
						}
					}
					else
					{
						if (strcmp(tuple->arguments[1], "~") == 0)
						{
							chdir(homedir);
							free(pwd);
							if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
							{
								perror("getcwd");
								exit(2);
							}
						}
						else if (strcmp(tuple->arguments[1], "-") == 0)
						{
							chdir(owd);
							free(pwd);
							if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
							{
								perror("getcwd");
								exit(2);
							}
						}
						else if (chdir(tuple->arguments[1]) != 0)
						{
							perror("chdir failed");
						}
						else
						{
							free(pwd);
							if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
							{
								perror("getcwd");
								exit(2);
							}
						}
					}
				}
				freeArgs(tuple);
				free(commandline);
			}
			else
			{
				/*  else  program to exec */
				char *commandStr = which(tuple->arguments[0], environmentString);
				if (commandStr != NULL)
				{
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
				free(commandStr);
				free(commandline);
				freeArgs(tuple);
			}
		}
	}
	return 0;
} /* sh() */

char *which(char *command, char *path)
{
	/* loop through pathlist until finding command and return it.  Return
   NULL when not found. */

	//check for absolute path
	if (*command == '/')
	{
		if (access(command, X_OK) == 0)
		{
			char *returnString = calloc(strlen(command) + 1, sizeof(char));
			strncpy(returnString, command, strlen(command));
			return returnString;
		}
		else
		{
			return NULL;
		}
	}

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

void list(char *dirPath)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(dirPath);
	if (d == NULL)
	{
		perror("list");
	}
	else
	{
		while ((dir = readdir(d)) != NULL)
		{
			printf("%s\n", dir->d_name);
		}
		closedir(d);
	}
} /* list() */

char readInput(char *buffer)
{
	char length = 0;
	if (fgets(buffer, BUFFER_SIZE, stdin) != 0)
	{
		length = strlen(buffer);
		buffer[length - 1] = '\0';
	}
	if (feof(stdin)) 
	{

	}
	return length;
}

tuple_t *stringToArray(char *input)
{
	tuple_t *tup = malloc(sizeof(tuple_t));
	printString("tuple created");

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

void freeArgs(tuple_t *t)
{
	//free args index
	int i = 0;
	while (t->arguments[i] != NULL)
	{
		free(t->arguments[i]);
		i++;
	}
	// //free args list
	free(t->arguments);
	//free tuple
	free(t);
	printString("tuple freed");
}
