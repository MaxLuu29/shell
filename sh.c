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
#include <wordexp.h>
#include <errno.h>
#include "sh.h"

int sh(int argc, char **argv, char **envp)
{
	char *prompt = calloc(PROMPTMAX, sizeof(char)); // allocates memory for display prompt
	char *command, *arg, *commandpath, *p, *pwd, *owd;
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;

	uid = getuid();
	password_entry = getpwuid(uid);   // get passwd info
	homedir = password_entry->pw_dir; // Home directory to start out with

	if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL) // get current working directory
	{
		perror("getcwd");
		exit(2);
	}

	owd = calloc(strlen(pwd) + 1, sizeof(char)); // old working directory for cd -
	memcpy(owd, pwd, strlen(pwd));

	char buffer[BUFFER_SIZE];
	int len;
	char *environmentString = getenv("PATH");

	while (go)
	{
		// print the prompt 
		printf("%s [%s]> ", prompt, pwd);

		// get user input
		len = readInput(buffer);

		// get command line and process 
		if (len > 1)
		{
			char *commandline = calloc(len, sizeof(char));
			strcpy(commandline, buffer);
			tuple_t *tuple = stringToArray(commandline); 

			// check for each built in command and implement

			/* EXIT -------------------------------------------------------------- */
			if (strcmp(tuple->arguments[0], "exit") == 0)
			{
				printExecCommand(1, "exit");
				free(prompt);
				free(pwd);
				free(owd);
				free(commandline);
				freeArgs(tuple);
				exit(0);
			}

			/* PID --------------------------------------------------------------- */
			/* prints the pid of the shell */
			else if (strcmp(tuple->arguments[0], "pid") == 0)
			{
				printExecCommand(1, "pid");
				printf("%d\n", (int)getpid());
				freeArgs(tuple);
				free(commandline);
			}

			/* PROPMT ------------------------------------------------------------ */
			else if (strcmp(tuple->arguments[0], "prompt") == 0)
			{
				printExecCommand(1, "prompt");
				if (tuple->count == 1)
				{
					printf("\tInput prompt prefix: "); //ask user for prompt name
					int length = readInput(buffer);
					if (length >= PROMPTMAX) // check if prompt name is more than 32 characters
					{
						fprintf(stderr, "invalid prompt size\n");
					}
					else
					{
						strcpy(prompt, buffer); //copys user's input into prompt
					}
					freeArgs(tuple);
					free(commandline);
				}
				else if (tuple->count == 2)
				{
					if (strlen(tuple->arguments[1]) >= PROMPTMAX)
					{
						fprintf(stderr, "invalid prompt size\n");
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
					// if prompt receives to many arguments
					printf("too many arguments for prompt\n");
					freeArgs(tuple);
					free(commandline);
				}
			}

			/* WHICH ----------------------------------------------------------------
			* check if the arguments are builtin executables
			* and prints the first absolute path to the excecutalbles 
			* or null otherwise
			*/
			else if (strcmp(tuple->arguments[0], "which") == 0)
			{
				printExecCommand(1, "which");
				if (tuple->arguments[1] != NULL) // loops through the arguments
				{
					int i = 1;
					while (tuple->arguments[i] != NULL)
					{
						int exist = 0;
						char *whichReturn = which(tuple->arguments[i], environmentString); // calls the which helper function
																						   // to check for absolute path
						if (whichReturn != NULL)
						{
							printString(whichReturn);
							free(whichReturn);
							exist = 1;
						}
						if (exist == 0) //print command not found if which returns NULL
						{
							printf("%s not found\n", tuple->arguments[i]);
						}
						i++;
					}
				}
				freeArgs(tuple);
				free(commandline);
			}

			/**
			 * WHERE ----------------------------------------------------------------
			 * almost the same as which
			 * check if the arguments are builtin executables 
			 * and prints ALL occurences with its absolute path
			 * or print path does not exist otherwise
			 */ 
			else if (strcmp(tuple->arguments[0], "where") == 0)
			{
				printExecCommand(1, "where");
				if (tuple->arguments[1] != NULL)
				{
					int i = 1;
					while (tuple->arguments[i] != NULL)
					{
						where(tuple->arguments[i], environmentString); // calls the helper where function
						i++;
					}
				}
				freeArgs(tuple);
				free(commandline);
			}

			/**
			 * PRINTENV --------------------------------------------------------------
			 * prints the entire environment variables with no arguments
			 * or print the path matching user input environment variable
			 */
			else if (strcmp(tuple->arguments[0], "printenv") == 0)
			{
				printExecCommand(1, "printenv");
				if (tuple->count == 1)
				{
					for (char **env = envp; *env != 0; env++) // get the environment path for main
					{
						char *curEnv = *env;
						printString(curEnv);
					}
				}
				else if (tuple->count == 2)
				{
					char *environmentFromGetEnv = getenv(tuple->arguments[1]); // get path of specific env. variable
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

			/**
			 * SETENV -----------------------------------------------------------------
			 * prints all environment variables if run with no arguments
			 * or set the user input environment variable with its 
			 * respective user input value
			 */
			else if (strcmp(tuple->arguments[0], "setenv") == 0)
			{
				printExecCommand(1, "setenv");
				if (tuple->count == 1)
				{
					for (char **env = envp; *env; env++) // prints all env. variables if provide no arguments
						printString(*env);
				}
				else if (tuple->count == 2)
				{
					if (strcmp(tuple->arguments[1], "HOME") == 0) // check if user is updating HOME
																  // to change the homedir to ""
																  // if provided only one arguments
					{
						setenv("HOME", "", 1);
						strcpy(homedir, "");
					}
					else
					{
						setenv(tuple->arguments[1], "", 1);
					}
				}
				else if (tuple->count == 3)
				{
					if (strcmp(tuple->arguments[1], "HOME") == 0) // check if user is updating HOME
																  // to change the homedir to ""
																  // if provided only one arguments
					{
						DIR *d = opendir(tuple->arguments[2]); // check if new home dir is a valid directory
						if (d)
						{
							closedir(d);
							setenv("HOME", tuple->arguments[2], 1);
							strcpy(homedir, tuple->arguments[2]);
						}
						else if (ENOENT == errno)
						{
							perror("directory does not exist");
						}
						else
						{
							perror("directory failed for some reason");
						}
					}
					else
					{
						if ((setenv(tuple->arguments[1], tuple->arguments[2], 1)) != 0)
							perror("");
					}
				}
				freeArgs(tuple);
				free(commandline);
			}

			/**
			 * KILL ---------------------------------------------------------------
			 * kills a process if provided with its pid
			 * or kills a process by passing a signal base on user input
			 * value
			 */
			else if (strcmp(tuple->arguments[0], "kill") == 0)
			{
				printExecCommand(1, "kill");
				if (tuple->count == 1 || tuple->count > 3) // check if too little or
														   // too many arguments
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
					if ((killStatus = kill(pid1, SIGTERM)) != 0) //kills the process using its pid
					{
						perror("2");
					}
				}
				else if (tuple->count == 3)
				{
					int pid1 = atoi(tuple->arguments[2]);
					int signal = atoi(tuple->arguments[1]);
					int killStatus;

					if (signal < 0)
					{
						signal *= -1;
						free(prompt);
						free(pwd);
						free(owd);
					}

					free(commandline);
					freeArgs(tuple);
					if ((killStatus = kill(pid1, signal)) != 0) //kills the process with a custom signal
						perror("3");
				}
			}

			/**
			 * LIST -----------------------------------------------------------------------
			 * list all files and directory of a input directory
			 * if no args is given, list the files of the current directory
			 */
			else if (strcmp(tuple->arguments[0], "list") == 0)
			{
				printExecCommand(1, "list");
				if (tuple->count == 1)
				{
					printf("current directory\n");
					list("."); // calls helper function for opening the directory
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

			/**
			 * CD -------------------------------------------------------------------------
			 * change directory
			 */
			else if (strcmp(tuple->arguments[0], "cd") == 0)
			{
				printExecCommand(1, "cd");
				if (tuple->count > 2)
				{
					perror("cd: too many arguments");
				}
				else
				{
					if (tuple->count == 1)
					{
						chdir(homedir); // no args, cd into home directory
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
							chdir(homedir); // "~" cd into home directory
							free(pwd);
							if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
							{
								perror("getcwd");
								exit(2);
							}
						}
						else if (strcmp(tuple->arguments[1], "-") == 0)
						{
							chdir(owd); // "-" cd into the old working directory, in this case the directory of the 
										// shell
							free(pwd);
							if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
							{
								perror("getcwd");
								exit(2);
							}
						}
						else if (*(tuple->arguments[1]) == '/' && strlen(tuple->arguments[1]) > 1) // check if directory starts with a '/'
																								   // for example: /bin/
						{
							char strippedString[strlen(tuple->arguments[1])];
							strncpy(strippedString, tuple->arguments[1] + 1, strlen(tuple->arguments[1]));

							if (chdir(strippedString) != 0)
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
						else
						{
							if (chdir(tuple->arguments[1]) != 0) // cd into directory without '/' in front
																 // for example: bin/
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
				}
				freeArgs(tuple);
				free(commandline);
			}
			else
			{
				/*  else program to exec */
				char *commandStr = which(tuple->arguments[0], environmentString);
				int wildCard = 0;
				char **expCmd;
				if (commandStr != NULL)
				{
					printExecCommand(0, commandStr);

					/** 
					 * WILDCARD HANDLING --------------------------------------------------------
					 * using wordexp
					 * essentially expand wildcard inside the arguments if there are any
					 */

					wordexp_t p;
					char **w;
					int wordExpIndex;
					for (size_t i = 0; tuple->arguments[i] != NULL; i++)
					{
						int index = strcspn(tuple->arguments[i], "*?"); // check for wildcard
						int k, j;
						if (index < strlen(tuple->arguments[i])) // if wildcard exist
						{
							wildCard = 1;

							wordexp(tuple->arguments[i], &p, 0); // expand wildcard
							w = p.we_wordv; 
							int count = i + p.we_wordc + 1;

							expCmd = malloc(count * sizeof(*expCmd)); // creates a new **args to hold the new
																	  // expanded files matching the wildcard
							expCmd[count - 1] = NULL;

							for (j = 0; j < i; j++) // copies the command and its flag(s) into the new **args 
							{
								expCmd[j] = (char *)malloc((strlen(tuple->arguments[j]) + 1) * sizeof(char));
								strcpy(expCmd[j], tuple->arguments[j]);
							}

							for (k = 0; k < p.we_wordc; k++) // copies the expanded files into the new **args
							{
								expCmd[k + i] = (char *)malloc((strlen(w[k]) + 1) * sizeof(char));
								strcpy(expCmd[k + i], w[k]);
							}
							wordfree(&p);
						}
					}

					pid_t pid;
					int status;

					if ((pid = fork()) == 0) // create a new child process for running external commands
					{
						if (wildCard == 0) // exec for no wildcards
						{
							execve(commandStr, tuple->arguments, envp);
							perror("exec failed");
							free(commandline);
							free(tuple);
							exit(2);
						}
						if (wildCard == 1) // exec for wildcards
						{
							printf("reached here");
							execve(commandStr, expCmd, envp);
							perror("exec failed");
							free(commandline);
							free(tuple);
							exit(2);
						}
					}
					else
					{
						waitpid(pid, &status, 0); // wait for child process to finish
					}
				}
				else
				{
					printf("command not found\n");
				}
				free(commandStr);
				free(commandline);
				if (wildCard == 1)
				{
					for (size_t i = 0; expCmd[i] != NULL; i++)
					{
						free(expCmd[i]);
					}
					free(expCmd);
				}

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
	if (*command == '/' || *command == '.')
	{
		if (access(command, X_OK) == 0) // calles access system call to check if the command exist
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
		result = strtok(copyPath, delim); // splice the path
		while (result != NULL)
		{
			char pathOfCommand[strlen(result) + strlen(command)];
			sprintf(pathOfCommand, "%s/%s", result, command); // append the command to each path and check
															  // if it exists
			if (access(pathOfCommand, X_OK) == 0)
			{
				char *returnString = calloc(strlen(pathOfCommand) + 1, sizeof(char));
				strncpy(returnString, pathOfCommand, strlen(pathOfCommand)); //returns the existing path after allocating enough space
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
		result = strtok(copyPath, delim); // splice the path
		while (result != NULL)
		{
			char pathOfCommand[strlen(result) + strlen(command) + 1]; // append the command to each path and check
															          // if it exists
			sprintf(pathOfCommand, "%s/%s", result, command);

			if (access(pathOfCommand, F_OK) == 0)
			{
				printf("%s\n", pathOfCommand); // prints all paths to the command
				break;
			}

			result = strtok(NULL, delim);
		}
	}
	return NULL;
} /* where() */

// list the content of the inputted directory
void list(char *dirPath)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(dirPath); // opens the directory
	if (d == NULL) // check if it's a valid directory
	{
		perror("list");
	}
	else
	{
		while ((dir = readdir(d)) != NULL) // reads the directory and print each piece of content
		{
			printf("%s\n", dir->d_name);
		}
		closedir(d); // closes the directory
	}
} /* list() */

// reads and saves user's input from the command line
char readInput(char *buffer)
{
	char length = 0;
	if (fgets(buffer, BUFFER_SIZE, stdin) != 0) // saves user input into buffer
	{
		length = strlen(buffer);
		buffer[length - 1] = '\0'; // nulls the last character in buffer
	}
	if (feof(stdin)) // check for EOF and ignores it
	{
	}
	return length;
}

// split up user input into arrays of strings containing all comands with its flags and arguments
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

	char **argv = malloc((count + 2) * sizeof(*argv)); // creates the array to hold strings
	argv[count + 1] = NULL; // NULLS the last index of **args for exec to work properly

	count = 0;
	strcpy(buff, input); 
	t = strtok(buff, " ");

	while (t)
	{
		int len = strlen(t);
		argv[count] = (char *)malloc((len + 1) * sizeof(char)); // allocate space for each idex of the array
		strcpy(argv[count], t); // copy over the input token to the array
		count++;
		t = strtok(NULL, " ");
	}

	tup->arguments = argv;
	tup->count = count;

	return tup;
}

// frees the created tuple containing the **args and its count
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
	//printString("tuple freed");
}

// helper function for printing strings
void printString(char *s)
{
	printf("%s\n", s);
}

// helper function for printing what command is being run
void printExecCommand(int isBuiltin, char *command)
{
	if (isBuiltin == 1)
	{
		printf("Executing built-in [%s]\n", command);
	}
	else
	{
		printf("Executing [%s]\n", command);
	}
}