#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <float.h>
#include <signal.h>

// terminal input is used to create a struct using these attributes:
struct cmdLine_Struct
{
    char *inFile;
    char *outFile;
    char *exec_program;
    char *exec_args[256];
    int argsAmount;
    int startProgram;
    int badFile;
};

// Variable toggles foreground inside two different functions:
int fg_toggle = 1;	// default 1 means foreground is on, 0 means off

void handle_SIGTSTP(int signo);

// used to create a struct using terminal input:
struct cmdLine_Struct *new_struct(char *user_input)
{
    // search for symbols to help with redirection code:
    char input_symbol[] = "<";
    char output_symbol[] = ">";
    
    // pointer for strtok_r function:
    char *saveptr;
    int i;

    // Create dynamic memory for struct:
    struct cmdLine_Struct *current_cmdline = malloc(sizeof(struct cmdLine_Struct));
    current_cmdline->argsAmount = 0;

    // strtok_r gets token using the space delimiter 
    i = 0;
    char *token = strtok_r(user_input, " ", &saveptr);
    char *backup = token;
    size_t token_length = strlen(token);

    // Note down if first token is a directory or file name in badFile attribute:
    DIR *pDir;
    int sourceFD = open(token, O_RDONLY);
    if (sourceFD == -1) current_cmdline->badFile = 1;
    close(sourceFD);
    pDir = opendir(token);
    if (pDir == NULL) current_cmdline->badFile = 1;
    closedir (pDir);

    // toggle program to indicate if user input has a program command in it:
    int program = 0;
    

    // Use strtok_r until token is NULL. 
    while (token != NULL)
    {
        // Input Symbol:
        if (strcmp(token, input_symbol) == 0)
        {
            // To reach input, we need to remove the space, so we use strtok_r:
            token = strtok_r(NULL, " ", &saveptr);
            token_length = strlen(token);

            // We remove any new line by replacing it with a null terminator:
            if (token[token_length - 1] == '\n') token[--token_length] = '\0';

            // We create the dynamic memory, strncpy to build our struct, 
            // then remove the trailing space:
            current_cmdline->inFile = calloc(256, sizeof(char));
            strncpy(current_cmdline->inFile, token, strlen(token));
            token = strtok_r(NULL, " ", &saveptr);
        }
        // Output Symbol:
        else if (strcmp(token, output_symbol) == 0)
        {
            // To reach input, we need to remove the space, so we use strtok_r:
            token = strtok_r(NULL, " ", &saveptr);
            token_length = strlen(token);

            // We remove any new line by replacing it with a null terminator:
            if (token[token_length - 1] == '\n') token[--token_length] = '\0';
            
            // We create the dynamic memory, strncpy to build our struct, 
            // then remove the trailing space:
            current_cmdline->outFile = calloc(256, sizeof(char));
            strncpy(current_cmdline->outFile, token, strlen(token));
            token = strtok_r(NULL, " ", &saveptr);
        }
        else if (program == 0)
        {
            // remove newline so exec program can work:
            if (token[token_length - 1] == '\n') token[--token_length] = '\0';

            // We create the dynamic memory, strncpy & remove the trailing space:
            current_cmdline->exec_program = calloc(256, sizeof(char));
            strncpy(current_cmdline->exec_program, token, strlen(token));
            token = strtok_r(NULL, " ", &saveptr);

            // toggle program indicator:
            program += 1;   

            // toggle program indicator to enter exec family clause later:
            current_cmdline->startProgram++;    
        }
        else
        {
            // prepate with buffers, etc:
            char *checkDollarsSigns;    // Needle in the haystack, strstr function variable
            int j;
            int k;
            int myIndex;
            int newArg;
            char current_PID[50];
            int bufferPID = getpid();
            char newString[strlen(token) + bufferPID];
            j = 0;
            k = 0;
            myIndex = 0;
            newArg = 0;

            // PID integer --> string:
            sprintf(current_PID, "%d", bufferPID);

            // Search for $$-- The needle in the hay stack
            checkDollarsSigns = strstr(token, "$$");

            // Replace newline with null terminator for error prevention:
            token_length = strlen(token);
            if (token[token_length - 1] == '\n') token[--token_length] = '\0';

            // Check if we need to do expansion(is $$ there?)
            if (checkDollarsSigns == NULL)
            {
                current_cmdline->exec_args[current_cmdline->argsAmount] = calloc(256, sizeof(char));
                strcpy(current_cmdline->exec_args[current_cmdline->argsAmount], token);
                newArg++;       // To keep count of total number of arguments
            }

            // Code block to create expansion: We replace instance of && with PID number:
            else
            {
                for (k = 0; k <= strlen(token); k++)
                {
                    if (token[myIndex] == '$')
                    {
                        if (token[myIndex + 1] == '$')
                        {
                            // If we find a complete match, we can use strcat to add PID to new string:
                            strcat(newString, current_PID);
                            myIndex += 1;
                            j += strlen(current_PID);
                        } else {
                            // edge case: if just 1 $ symbol, no expansion:
                            newString[j] = token[myIndex];
                            j += 1;
                        }
                    }
                    else
                    {
                        // Create new string with expansion:
                        newString[j] = token[myIndex];
                        j += 1;
                    }
                    myIndex += 1;
                    k = myIndex;
                
                }

                // copy our result into our struct attribute:
                current_cmdline->exec_args[current_cmdline->argsAmount] = calloc(256, sizeof(char));
                strcpy(current_cmdline->exec_args[current_cmdline->argsAmount], newString);
                memset(newString,0,strlen(newString));        // clears our newString for next iteration
                newArg++;                                     // To help keep count of total arguments
             
            }
            // We increment for each argument:
            current_cmdline->argsAmount++;
            memset(token,0,strlen(token));        
            token = strtok_r(NULL, " ", &saveptr);      // Remove space to search the next token:
        }
    }
   
    return current_cmdline;
}

// Toggle foreground mode with catching ctrl Z. If foreground mode is toggled on, & in user input is ignored:
void handle_SIGTSTP(int signo)
{	
    if (fg_toggle == 1)
    {

        char *message = "Entering foreground-only mode (& is now ignored)\n"; 
        write(1, message, 50);        // Use write because it is a reenterant function
        fflush(stdout);                                       
        fg_toggle = 0;
    }
    else
    {
        char *message = "Exiting foreground-only mode\n"; 
        write(1, message, 30);        // Use write because it is a reenterant function
        fflush(stdout);
        fg_toggle = 1; 
    }
}

void statusFunction(status)
{
    
    // To check if the exit was normal: If it is, we can extract process's status using WEXITSTATUS:
    if (WIFEXITED(status))
    {
        printf("exit value %d\n", WEXITSTATUS(status));
        fflush(stdout);
    }

    // To check if the exit was abnormal: If it is, we can extract process's status using WTERMSIG:
    if WIFSIGNALED (status)
    {
        printf("terminated by signal %d\n", WTERMSIG(status));
        fflush(stdout);
    }
}

void smallsh()
{
    int wstatus;
    char user_input[2048];
    int smallshLoop = 1;

    // buffer placeholders for us to use our logic:
    char buffer1[100];
    char buffer2[100];
    int i;

    // We set up our structs for our signals:
    // Struct for SIGINT:
    struct sigaction SIGINT_action = {0}; 
	SIGINT_action.sa_handler = SIG_IGN;         // We make the sa_handler to ignore it's default action
	sigfillset(&SIGINT_action.sa_mask);
	// No flags set
	SIGINT_action.sa_flags = 0;
	

   // Struct for SIGTSTP:
    struct sigaction SIGTSTP_action = {0};

    // When we press ctrl Z, we instead make it's default function be our user made function, handle_SIGTSTP:
    SIGTSTP_action.sa_handler = handle_SIGTSTP;     
    sigfillset(&SIGTSTP_action.sa_mask);        
    // To deal with a bug: We make the SA_RESTART flag so that we don't get execvp() be activated using old user inputs. 
    SIGTSTP_action.sa_flags = SA_RESTART;                
    
    // Here we turn on the structs we created using the sigaction function:
    sigaction(SIGINT, &SIGINT_action, NULL);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL);   

    // We start our main smallsh function. It will stay on until smallshLoop turns 0:
    while (smallshLoop == 1)
    {
        // Default user input:
        printf(": ");
        fflush(stdout);

        // To get the user's input:
        fgets(user_input, sizeof(user_input), stdin);

        // Check if user input needs to have foreground or background settings:
        // for our bg and fg:
        int background = 0;             // Detected by & at the end of user input
        int foreground = 0;
        int mytestAmpersand = strcmp(user_input + strlen(user_input) - 2, "$\n");
        char a[256];
        
        // If & detected at end of user input, turn background variable on so 
        // background process options can be enabled later:
        if (strcmp(user_input + strlen(user_input) - 3, " &\n") == 0)
        {
            memset(a, '\0', sizeof(a));
            strncpy(a, user_input, strlen(user_input) -3);
            user_input[0] = '\0';
            
            strcpy(user_input, a);
            background = 1;             // 1 means "on". & found, so turn background on
            foreground = 0;
        // If & is not detected, turn foreground variable on:
        } else {
            // If no ampersand, we toggle fg variable:
            background = 0;
            foreground = 1;             // 1 means "on". no & found, so turn foreground on
        }


        // Make struct:
        struct cmdLine_Struct *currCmd = new_struct(user_input);

        // If comment, continue for next input:
        if (strcmp(currCmd->exec_program, "#") == 0)
        {
            continue;       
        }

        // If empty string, continue for next input:
        else if (strcmp(currCmd->exec_program, "\0") == 0)
        {
            continue;       
        }

        // If "status", return status:
        else if (strcmp(currCmd->exec_program, "status") == 0)
        {
            statusFunction(wstatus);
        }

        // If "exit", toggle loop to 0 which breaks loop to exit program:
        else if (strcmp(currCmd->exec_program, "exit") == 0)
        {
            smallshLoop = 0;
        }

        // Search for 'cd' If found, search the arguments to do correct task:
        else if (strcmp(currCmd->exec_program, "cd") == 0)
        {
            // Change to home if no destination requested:
            if (currCmd->exec_args[0] == NULL)
            {
                chdir(getenv("HOME"));
            }
            // Else change to requested destination:
            else
            {
                chdir(currCmd->exec_args[0]);
            }
        }

        // else send user inputs to child process: 
        else 
        {
            // Store length:
            int length = sizeof(currCmd->exec_args) / sizeof(currCmd->exec_args[0]);

            // Combine all elements into final array to match the execvp's vector requirement:
            char **arrayToExec;
            arrayToExec = malloc(currCmd->argsAmount + 1 * sizeof(char *));
            for (int arg_i = 0; arg_i < currCmd->argsAmount + 1; arg_i++)
            {
                arrayToExec[arg_i] = malloc(256 * sizeof(char));
                if (arg_i == 0)
                {
                    arrayToExec[arg_i] = currCmd->exec_program;
                }
                if (arg_i > 0)
                {
                    strcpy(arrayToExec[arg_i], currCmd->exec_args[arg_i - 1]);
                }
            }

            // Null terminate our arguments:
            arrayToExec[currCmd->argsAmount + 1] = NULL;

            // fork() & prepare variables:
            pid_t spawnpid;
            pid_t spawnpid2;
            int childStatus;
            int childPid;
            int return_value;
            int result;
            int sourceFD;
            char sendProgram[256];

            spawnpid = fork();
            switch (spawnpid)
            {
                case -1:
                    // If fork fails, return comment to inform user:
                    perror("fork() failed");
                    fflush(stdout);
                    break;
                case 0:
                    // Only foreground processes can use SIGINT signal, background processes maintains SIG_IGN setting:
                    if (foreground == 1)
                    {
                        SIGINT_action.sa_handler = SIG_DFL;         // Toggle sa_handler from SIG_IGN to SIG_DFL
                        sigaction(SIGINT, &SIGINT_action, NULL);    // sigaction to update
                        
                    }

                    // Use dup() to redirect file destcriptors to stored users destination inputs:

                    // Start with stdin: Open file and dup2:
                    if (currCmd->inFile)
                    {
                        sourceFD = open(currCmd->inFile, O_RDONLY);
                        if (sourceFD == -1)
                        { 
                            printf("cannot open %s for input \n", currCmd->inFile);
                            fflush(stdout);
                            exit(1);
                        }

                        result = dup2(sourceFD, 0); // makes stdin point to sourceFD pointer
                        if (result == -1)
                        { 
                            perror("source dup2()");
                            exit(2);
                        }

                        // Close file: we use fcnl instead of close so the child processes can't touch it
                        fcntl(sourceFD, F_SETFD, FD_CLOEXEC);
                    }

                    // Next for stdout: Open file and dup2:
                    if (currCmd->outFile)
                    {
                        if (currCmd->outFile[strlen(currCmd->outFile - 1)] == '\n') currCmd->outFile[strlen(currCmd->outFile)-1] = '\0';
                        int targetFD = open(currCmd->outFile, O_WRONLY | O_CREAT | O_TRUNC, 0777); 
                        if (targetFD == -1)
                        { 
                            printf("open() failed on \"%s\"\n", currCmd->outFile);
                            fflush(stdout);
                            perror("Error");
                            exit(1);
                        }

                        result = dup2(targetFD, 1); // Point stdout to targetFD pointer
                        if (result == -1)
                        {
                            perror("dup2");
                            exit(2);
                        }

                        // Close file: we use fcnl instead of close so the child processes can't touch fds:
                        fcntl(targetFD, F_SETFD, FD_CLOEXEC);
                    }
                    
                    // This block gets activated if we noticed an exec family call original user input(we toggled startProgram attribute
                    // to 1 if found):
                    if (currCmd->startProgram == 1)
                    {
                        return_value = execvp(currCmd->exec_program, arrayToExec);  // Send our constructed array in the proper format, print errors if created
                        if (return_value == -1)
                        {
                            if (currCmd->badFile == 1) printf("%s: no such file or directory\n", currCmd->exec_program);
                            exit(1);
                        }
                            
                    }
                    break;

                default:

                // Parent Process: If these options, parent must continue process(showing terminal) by using WNOHANG:
                if (background == 1 && fg_toggle == 1)
                {
                    // print background pid of background child process(spawnpid in parent section is child pid):
                    printf("background pid is %i\n", spawnpid); 
                    fflush(stdout);

                    // Parent continue showing terminal to user. Doesn't wait for background child to finish: 
                    childPid = waitpid(spawnpid, &wstatus, WNOHANG); 
                }

                // Foreground settings: If activated, parent waits for that foreground child process to finish
                if (foreground == 1 || fg_toggle == 0) 
                {
                    childPid = waitpid(spawnpid, &wstatus, 0);       
                
                }

                // For zombie processes cleanup. Parent doesn't wait for them to finish: 
                spawnpid = waitpid(-1, &wstatus, WNOHANG); 
                while (spawnpid > 0)
                {
                    printf("background pid %i is done: ", spawnpid); 
                    fflush(stdout);
                    statusFunction(wstatus);                            // Returns status of child termianted in the background
                    break;
                }

            }

        // reset/null terminate:
        currCmd->inFile = '\0';
        currCmd->outFile = '\0';
        currCmd->exec_program = '\0';
        for (i=0; i<currCmd->argsAmount; i++) currCmd->exec_args[i] = '\0';
        for (int arg_i = 0; arg_i < currCmd->argsAmount + 1; arg_i++) arrayToExec[arg_i] = '\0';
        currCmd->argsAmount = 0;
        currCmd->startProgram = 0;
        currCmd->badFile = 0;
        smallshLoop = 1;
        }
    }
}

int main(int argc, const char *argv[])
{

    // Start smallsh program:
    smallsh();

    return EXIT_SUCCESS;
}
