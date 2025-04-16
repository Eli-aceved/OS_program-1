#ifndef SLOSH_H
#define SLOSH_H

/* INCLUDES */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>    // write(), chdir()
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>    // sigaction()
#include <limits.h>
#include <errno.h>

/* DEFINES */
/* Define PATH_MAX if it's not available */
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

/* FUNCTION PROTOTYPES */
void display_prompt(void);
void sigint_handler(int sig);
int parse_input(char *input, char **args);
int handle_builtin(char **args);
void execute_command(char **args);


#endif

