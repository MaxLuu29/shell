
#include "get_path.h"

typedef struct tuple
{
	char **arguments;
	int count;
} tuple_t;

int pid;
int sh(int argc, char **argv, char **envp);
char *which(char *command, char *path);
char *where(char *command, char *path);
void list (char *dir);
void printenv(char **envp);
char readInput(char *buffer);
tuple_t *stringToArray(char *input);

#define PROMPTMAX 32
#define MAXARGS 10
#define BUFFER_SIZE 128
