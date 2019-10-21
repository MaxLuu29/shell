#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>
#include <wordexp.h>

#define BUFFSIZE 128

// void printString(char *s)
// {
// 	printf("%s\n", s);
// }

// void printNumber(int n)
// {
// 	printf("%d\n", n);
// }

char **stringToArray(char *input)
{
	// make a copy of array
	char buff[BUFFSIZE];
	strcpy(buff, input);
	char *t = strtok(buff, " "); // this returns the first word (if empty string it returns NULL)
	int count = 0;

	// call strtok over and over until you determine how long it is
	while (strtok(NULL, " "))
	{
		count++;
	}

	char **argv = malloc((count + 1) * sizeof(char *));
	argv[count] = NULL;

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

	return argv;
}

// char *which(char *command, char *path)
// {
// 	char *delim = ":";
// 	char *result = NULL;

// 	char copyPath[strlen(path)];
// 	strcpy(copyPath, path);
// 	if (strcmp(command, "") != 0)
// 	{
// 		result = strtok(copyPath, delim);
// 		while (result != NULL)
// 		{
// 			char pathOfCommand[strlen(result) + strlen(command)];
// 			sprintf(pathOfCommand, "%s/%s", result, command);
// 			if (access(pathOfCommand, F_OK) == 0)
// 			{
// 				char *returnString = calloc(strlen(pathOfCommand) + 1, sizeof(char));
// 				strncpy(returnString, pathOfCommand, strlen(pathOfCommand));
// 				return returnString;
// 			}
// 			result = strtok(NULL, delim);
// 		}
// 	}
// 	return NULL;
// }

char readInput(char *buffer)
{
	char length = 0;
	if (fgets(buffer, 128, stdin) != 0)
	{
		length = strlen(buffer);
		buffer[length - 1] = '\0';
	}
	return length;
}

int main(int argc, char const *argv[], char **envp)
{
	// struct passwd *pw = getpwuid(getuid());
	// const char *homedir = pw->pw_dir;
	// printf("%s\n", homedir);

	// setenv("HOME", "/Users/max/Downloads/myshell", 1);
	// printf("%s\n", getenv("HOME"));

	// struct passwd *pw2 = getpwuid(getuid());
	// const char *homedir2 = pw2->pw_dir;

	// printf("%s\n", homedir2);

	char *cmd[5];
	cmd[0] = "ls";
	cmd[1] = "-a";
	cmd[2] = "-l";
	cmd[3] = "*.c";
	cmd[4] = NULL;
	// char *cmd[4];
	// cmd[0] = "ls";
	// cmd[1] = "-al";
	// cmd[2] = "*.c";
	// cmd[3] = NULL;
	wordexp_t p;
	char **w;
	int wordExpIndex;

	
	for (size_t i = 1; cmd[i] != NULL; i++)
	{
		int index = strcspn(cmd[i], "*?");
		int k, j;
		if (index < strlen(cmd[i]))
		{
			printf("%s\n---------\n", cmd[i]);

			wordexp(cmd[i], &p, 0);
			w = p.we_wordv;
			int count = i + p.we_wordc + 1;
			printf("%d\n", count);
			char **expCmd = malloc(count * sizeof(*expCmd));
			expCmd[count-1] = NULL;

			// for (size_t c = 0; c < p.we_wordc; c++)
			// {
			// 	printf("%s\n", w[c]);
			// }
			// printf("---------------------------------\n");

			for (j = 0; j < i; j++)
			{
				printf("%s\n", cmd[j]);
				expCmd[j] = (char *)malloc((strlen(cmd[j]) + 1) * sizeof(char));
				strcpy(expCmd[j], cmd[j]);
			}	
			printf("---------------------------------\n");

			for (k = 0; k < p.we_wordc; k++)
			{
				printf("%s\n", w[k]);
				expCmd[k+i] = (char *)malloc((strlen(w[k]) + 1) * sizeof(char));
				strcpy(expCmd[k+i], w[k]);
			}
			printf("---------------------------------\n");


			for (size_t x = 0; expCmd[x] != NULL; x++)
			{
				printf("%s\n", expCmd[x]);
			}
			printf("---------------------------------\n");

			execve("/bin/ls", expCmd, envp);
			wordfree(&p);
		}
	}

	// wordexp_t p;
	// char **w;
	// int i;
	// char c[] = "*.c";
	// wordexp(c, &p, 0);
	// w = p.we_wordv;
	// for (i = 0; i < p.we_wordc; i++)
	// 	printf("%s\n", w[i]);
	// wordfree(&p);

	return 0;
}