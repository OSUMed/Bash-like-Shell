About Project:

This program is creating a simple simple which implements a subset of features of well-known shells, such as bash. It has the following capabilities:
1. Provide prompt for running commands
2. Handle blank lines and comments(start with #)
3. Provides expansion for a variable(&&)
4. Executes three commands(exit, cd, status) using the code build into the shell
5. Executes remaining commands using exec family of functions
6. Supports input and output redirection
7. Supports commands running in foreground and background processes
8. Implements custom signal handlers for SIGINT and SIGTSTP

Instructions on how to run code:

gcc -std=gnu99 -pthread -o smallsh smallsh.c
./smallsh
