#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <pwd.h>
#include <sys/types.h>

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

int main(int argc, char const *argv[])
{
	struct passwd *pw = getpwuid(getuid());
	// printf("%s\n", pw->pw_dir);
	char *pwd = getcwd(NULL, PATH_MAX + 1);
	printf("%s\n", pwd);

	// chdir(pw->pw_dir);

	// char *pwd2 = getcwd(NULL, PATH_MAX + 1);
	// printf("%s\n\n\n\n", pwd2);

	free(pwd);
	// free(pwd2);

	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			printf("%s\n", dir->d_name);
		}
		closedir(d);
		
	}
	return 0;
}