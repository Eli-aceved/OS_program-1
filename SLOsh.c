/**
 * SLOsh - San Luis Obispo Shell
 * CSC 453 - Operating Systems
 * 
 * TODO: Complete the implementation according to the comments
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <stdint.h>
 #include <string.h>
 #include <unistd.h>    // write()
 #include <sys/wait.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <signal.h>    // sigaction()
 #include <limits.h>
 #include <errno.h>
 
 /* Define PATH_MAX if it's not available */
 #ifndef PATH_MAX
 #define PATH_MAX 4096
 #endif
 
 #define MAX_INPUT_SIZE 1024
 #define MAX_ARGS 64
 
 /* Global variable for signal handling */
 volatile sig_atomic_t child_running = 0;
 
 /* PROTOTYPES */
 void display_prompt(void);
 
 /**
  * Signal handler for SIGINT (Ctrl+C)
  * 
  * TODO: Implement the signal handler to:
  * 1. Print a newline
  * 2. If no child is running, display a prompt
  * 3. Make sure the shell doesn't exit when SIGINT is received
  */
 void sigint_handler(int sig) {
    /* TODO: Your implementation here */
    write(STDOUT_FILENO, "\n", 2);
    if (!child_running) { // If a child is not running
        // Display a prompt
        char cwd[PATH_MAX]; // Buffer to store the current working directory which is a string containing the file path 
        // Get the current working directory length
        uint16_t cwd_len = strlen(cwd);

        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            write(STDOUT_FILENO, cwd, cwd_len);
            write(STDOUT_FILENO, "> ", 2);
        } 
        else {
            perror("getcwd");
            write(STDOUT_FILENO, "SLOsh> ", 7);
        }
        fflush(stdout);
    }
 }
 
 /**
  * Display the command prompt with current directory
  */
 void display_prompt(void) {
     char cwd[PATH_MAX];
     
     if (getcwd(cwd, sizeof(cwd)) != NULL) {
         printf("%s> ", cwd);
     } else {
         perror("getcwd");
         printf("SLOsh> ");
     }
     fflush(stdout);
 }
 
 /**
  * Parse the input line into command arguments
  * 
  * TODO: Parse the input string into tokens and store in args array
  * 
  * @param input The input string to parse
  * @param args Array to store parsed arguments
  * @return Number of arguments parsed
  */
 int parse_input(char *input, char **args) {
    /* TODO: Your implementation here */
    // Tokenize the input string using delimiters
    /*char *token;
    const char *delimiters = "|>";
    strtok(input, delimiters); */

    // Store tokens in args array


    return 0;
 }
 
 /**
  * Execute the given command with its arguments
  * 
  * TODO: Implement command execution with support for:
  * 1. Basic command execution
  * 2. Pipes (|)
  * 3. Output redirection (> and >>)
  * 
  * @param args Array of command arguments (NULL-terminated)
  */
 void execute_command(char **args) {
     /* TODO: Your implementation here */
     
     /* Hints:
      * 1. Fork a child process
      * 2. In the child, reset signal handling and execute the command
      * 3. In the parent, wait for the child and handle its exit status
      * 4. For pipes, create two child processes connected by a pipe
      * 5. For redirection, use open() and dup2() to redirect stdout
      */
 }
 
 /**
  * Check for and handle built-in commands
  * 
  * TODO: Implement support for built-in commands:
  * - exit: Exit the shell
  * - cd: Change directory
  * 
  * @param args Array of command arguments (NULL-terminated)
  * @return 0 to exit shell, 1 to continue, -1 if not a built-in command
  */
 int handle_builtin(char **args) {
     /* TODO: Your implementation here */
     return -1;  /* Not a builtin command */
 }
 
 int main(void) {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    int status = 1;
    int builtin_result;


    /* TODO: Set up signal handling for SIGINT (Ctrl+C) */
    struct sigaction sa;
    sa.sa_handler = sigint_handler; // Call sigint_handler() when SIGINT is received
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Restart system calls if interrupted
    sigaction(SIGINT, &sa, NULL);


    while (status) {
        display_prompt();
        
    /* Read input and handle signal interruption */
    if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
        /* TODO: Handle EOF and signal interruption */
        break;
        }
        
        /* Parse input */
        parse_input(input, args);
        
        /* Handle empty command */
        if (args[0] == NULL) {
            continue;   // Skips the remaining statements in the loop (and goes to the next iteration)
        }
        
        /* Check for built-in commands */
        builtin_result = handle_builtin(args);
        if (builtin_result >= 0) {
            status = builtin_result;
            continue;
        }
        
        /* Execute external command */
        execute_command(args);
    }
    
    printf("SLOsh exiting...\n");
    return EXIT_SUCCESS;
}

