## Project Overview

A shell is a user interface for accessing an operating system's services. In the realm of Unix-like operating systems, the shell provides a command-line interface for users, allowing them to run commands, navigate directories, manage processes, and more. For this project, I've developed a simple shell, mimicking some core functionalities found in established shells such as bash.

## Capabilities

1. **Prompt Execution**: Provides a prompt for users to input and execute commands.
2. **Blank Line & Comments Handling**: Efficiently handles blank lines and supports comment lines that start with a '#'.
3. **Variable Expansion**: Supports expansion for variables using '&&'.
4. **Built-in Commands**: Incorporates code directly into the shell to execute commands like 'exit', 'cd', and 'status'.
5. **Command Execution**: Uses the exec family of functions to execute all other commands.
6. **Input & Output Redirection**: Fully supports redirecting command inputs and outputs.
7. **Foreground & Background Processes**: Allows commands to run both in the foreground and background.
8. **Custom Signal Handlers**: Implemented custom signal handlers to manage signals such as SIGINT and SIGTSTP.

## Running the Code

To compile and run the shell, use the following commands:

```bash
gcc -std=gnu99 -pthread -o smallsh smallsh.c
./smallsh
