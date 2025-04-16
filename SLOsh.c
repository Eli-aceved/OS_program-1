/*****************************************************************************
 * File name:  SLOsh.c
 *             SLOsh - San Luis Obispo Shell
 * 
 * Author:    Elizabeth Acevedo & Alejadro Regalado
 * 
 * Date:      04/15/2025
 * 
 * Class:      CSC 453 - Operating Systems
 * 
 * Description: This program demonstrates a simple shell implementation in C. 
 * It supports basic command execution, pipes, output redirection, and built-in
 * commands like "cd" and "exit". The shell handles user input, parses commands,
 * and executes them in child processes. It also manages signal handling for
 * Ctrl+C (SIGINT) to prevent the shell from terminating when a child process
 * is running. The shell provides a command prompt with the current working
 * directory and allows users to execute commands interactively.
 * 
 * 
 * Utilization: 
 *  make
 *  ./SLOsh
 *  <enter command>
 * 
 *****************************************************************************/

 #include "SLOsh.h" // Header file containing function prototypes and constants
 
 /* Global variable for signal handling */
 volatile sig_atomic_t child_running = 0;
 
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
    /* Redirection Settings */
    int redirect = 0;
    int append = 0;
    int forward_indx = 0;

    while(args[forward_indx] != NULL) {
        if (strcmp(args[forward_indx], ">") == 0) {
            redirect = 1;
            args[forward_indx] = NULL;
            break;
        }
        else if (strcmp(args[forward_indx], ">>") == 0) {
            redirect = 1;
            append = O_APPEND;
            args[forward_indx] = NULL;
            break;
        }
        forward_indx++;
    }
    
    /* Pipes */
    int piping = 0; // Flag to indicate if piping is used
    int pipe_indx = 0; // Index of the pipe
    //int first_cmd_indx = 0; // Index of the first command
    int pipe_fd[2]; // File descriptors for the pipe, pipe_fd[0] for reading, pipe_fd[1] for writing
    
    // Loop through args to check for a pipe
    while (args[pipe_indx] != NULL) {
        if (strcmp(args[pipe_indx], "|") == 0) { // Check for pipe
            piping = 1; // Set piping flag to 1
            if (pipe(pipe_fd) == -1) { // Create a pipe
                perror("pipe() failed");
                exit(EXIT_FAILURE);
            }
            args[pipe_indx] = NULL; // Null-terminate the first command
            break; // Exit the loop
        }
        pipe_indx++;
    }
    /* Main Command FORK */
    int status = 0; // Status of the child process
    pid_t pid; // Process ID
    pid = fork(); // Create a new process

    if (pid < 0) { // Fork failed
        // Handle error
        perror("fork() failed");
        exit(EXIT_FAILURE); // Fork failed
    }
    else if (pid == 0) {
        /* CHILD PROCESS */

        /* Reset signal handling */
        struct sigaction sa;
        sa.sa_handler = sigint_handler; // Call sigint_handler() when SIGINT is received
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART; // Restart system calls if interrupted
        signal(SIGINT, SIG_DFL); // Reset SIGINT handler using SIG_DFL because this will allow the child process to terminate when Ctrl+C is pressed

        /* I/O Redirection */
        // If redirecting, change output to file
        if (redirect && !piping) {
            int file = open(args[forward_indx + 1], O_WRONLY | O_CREAT | append, 0777);
            if (file == -1) {
                perror("open() failed");
                exit(EXIT_FAILURE);
            }
            dup2(file, STDOUT_FILENO); // Duplicate the file descriptor, 1 is the file descriptor for stdout
            close(file);               // Close the original file descriptor
        }

        /* Pipes */
        if (piping) { // If piping is used
            /* Use Write Wnd of the Pipe */
            close(pipe_fd[0]); // Close the read end
            dup2(pipe_fd[1], STDOUT_FILENO); // Duplicate the write end of the pipe to stdout
            close(pipe_fd[1]); // Close the original write end
        }

        /* Execute basic commands */
        if (execvp(args[0], args) == -1) { // execvp() is better than execv() because it searches for the command in the PATH environment variable (pretty much anywhere in the computer)
            perror("execv() failed");
            exit(EXIT_FAILURE); // execvp failed
        }
    }
    else {
        /* PARENT PROCESS */

        /* Second FORK used for Piping */
        if (piping) { 
            if (fork() == 0) {
                /* CHILD PROCESS */
                /* Reset signal handling */
                struct sigaction sa;
                sa.sa_handler = sigint_handler; // Call sigint_handler() when SIGINT is received
                sigemptyset(&sa.sa_mask);
                sa.sa_flags = SA_RESTART; // Restart system calls if interrupted
                signal(SIGINT, SIG_DFL); // Reset SIGINT handler using SIG_DFL because this will allow the child process to terminate when Ctrl+C is pressed

                /* I/O Redirection */
                // If redirecting, change output to file
                if (redirect) {//&& !piping) {
                    int file = open(args[forward_indx + 1], O_WRONLY | O_CREAT | append, 0777);
                    if (file == -1) {
                        perror("open() failed");
                        exit(EXIT_FAILURE);
                    }
                    dup2(file, STDOUT_FILENO); // Duplicate the file descriptor, 1 is the file descriptor for stdout
                    close(file);               // Close the original file descriptor
                }

                /* Read End of the Pipe */
                close(pipe_fd[1]); // Close the write end
                dup2(pipe_fd[0], STDIN_FILENO); // Duplicate the read end of the pipe to stdin
                close(pipe_fd[0]); // Close the original read end

                /* Execute second command */
                if (execvp(args[pipe_indx + 1], &args[pipe_indx + 1]) == -1) { // execvp() is better than execv() because it searches for the command in the PATH environment variable (pretty much anywhere in the computer)
                    perror("execv() failed");
                    exit(EXIT_FAILURE); // execvp failed
                }
            }

            /* PARENT PROCESS */
            close(pipe_fd[0]); // Close the read end of the pipe
            close(pipe_fd[1]); // Close the write end of the pipe

            child_running = 2; // Set child_running to 2 to indicate a second child process is running

        }
        // Second child process is not running
        child_running = 1; // Set child_running to 1 to indicate a child process is running

        while (child_running-- != 0) { // Resets child_running to 0 after waiting for the child process
            pid_t child_pid = waitpid(pid, &status, 0); // Wait for the child process to finish
            // Wait for the child(ren) process(es) to finish
            if (child_pid == -1) { 
                perror("waitpid() failed");
                exit(EXIT_FAILURE); // Waitpid failed
            }
            else if (child_pid > 0) { // 
                if (WIFEXITED(status)) { // Check if the child process terminated normally
                    int exit_code = WEXITSTATUS(status); // Get the exit code of the child process
                    if (exit_code != 0) {
                        printf("Child process exited with status %d\n", exit_code); // Get exit code (0-255)
                    }
                }
                else if (WIFSIGNALED(status)) { // Check if the child process was terminated by a signal
                    printf("Child process terminated by signal %d\n", WTERMSIG(status)); // Get the signal number that terminated the child process
                }
            }
        }
    }

    /* Hints:
    * 1. Fork a child process OK
    * 2. In the child, reset signal handling and execute the command OK
    * 3. In the parent, wait for the child and handle its exit status OK
    * 4. For pipes, create two child processes connected by a pipe
    * 5. For redirection, use open() and dup2() to redirect stdout OK
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

