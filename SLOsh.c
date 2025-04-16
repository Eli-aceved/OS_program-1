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
 #include <unistd.h>    // write(), chdir()
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
    // Tokenize the input string using space as a delimiter
    // Note: strtok() modifies the input string, so make a copy if needed
    //char *input_copy = strdup(input); // Duplicate the input string
    int i = 0; // Index for args array
    char *token;
    int args_parsed = 0; // Number of arguments parsed

    token = strtok(input, " \n"); 
    // Check if there are no tokens
    if (token == NULL) {
        printf("No arguments to be parsed. Not enough arguments.\n");
        return 0; // No arguments parsed
    }
    
    args[i] = token; // Store the first token in the args array
    args_parsed++; // Increment the number of arguments parsed
    // Store tokens in args array
    while (token != NULL) {
        // Store next token into args array
        i++;
        token = strtok(NULL, " \n"); // Get the next token
        args[i] = token; // Store the token in the args array
        args_parsed++; // Increment the number of arguments parsed
    }

    return args_parsed; // Return the number of arguments parsed
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
    int status = 0; // Status of the child process
    pid_t pid; // Process ID
    pid = fork(); // Create a new process

    if (pid < 0) {
        perror("fork() failed");
        exit(EXIT_FAILURE); // Fork failed
    }
    else if (pid == 0) { // Child process
        // Reset signal handling
        struct sigaction sa;
        sa.sa_handler = sigint_handler; // Call sigint_handler() when SIGINT is received
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; // Restart system calls if interrupted
        signal(SIGINT, SIG_DFL); // Reset SIGINT handler using SIG_DFL because this will allow the child process to terminate when Ctrl+C is pressed

        // Execute the command
        if (execvp(args[0], args) == -1) { // execvp() is better than execv() because it searches for the command in the PATH environment variable (pretty much anywhere in the computer)
            perror("execv() failed");
        }
    }
    else {               // Parent process
        child_running = 1; // Set child_running to 1 to indicate a child process is running
        // Wait for the child process to finish
        if (waitpid(pid, &status, 0) == -1) { 
            perror("waitpid() failed");
            exit(EXIT_FAILURE); // Waitpid failed
        }

        if (WIFEXITED(status)) { // Check if the child process terminated normally
            printf("Child process exited with status %d\n", WEXITSTATUS(status)); // Get exit code (0-255)
        }
        if (WIFSIGNALED(status)) { // Check if the child process was terminated by a signal
            printf("Child process terminated by signal %d\n", WTERMSIG(status)); // Get the signal number that terminated the child process
        }
        child_running = 0; // Reset child_running to 0 to indicate no child process is running
    }



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
    if (args[0] == NULL) {
        return -2; // No command entered
    }
    // Check for built-in commands
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "Error: cd command is missing argument(s)\n");
            return 1; // Continue the shell, just missing argument. Have user retry
        } 
        else {
            // Change directory using chdir() & check if the argument is a valid directory
            if (chdir(args[1]) != 0) {
                perror("cd command");
            }
            return 1; // Continue the shell
        }
    }
    else if (strcmp(args[0], "exit") == 0) {
        return 0; // Exit the shell
    }
    // Return value for built-in commands (0 success, -1 failure)
    return -1; // Not a built-in command
}
 
 int main(void) {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS] = {NULL}; // Initialize args array to NULL
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

