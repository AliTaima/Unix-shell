#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#define MAXCOM 256  // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

char buf[MAXCOM];       // string to store the command
char *command[MAXLIST]; // an array for command and arguments
char i;                 // global for loop counter
pid_t child_pid;        // global variable for the child process ID
int Zombie =0;          // global Counter of Zombie Processes

//Function Initialization
void init_shell();  // Prepare log file and output greeting message
void get_cmd();     // get command from the user
void openHelp();    // open the help for the user
void convert_cmd(); // split string into command
void handler();     // to handle SIGCHLD signal
int cd(char *path); // for the cd command
void log_write(char);// writes to the log file when any child process is started or terminated
void bury_Zombies();//To Bury Zombie Processes to prevent their stacking

/*Deriving Code*/
int main()
{
    init_shell();
    signal(SIGCHLD, handler);
    while (1)
    {
        // get command from user
        get_cmd();
        // check empty commands
        if (!strcmp("", buf))
            continue;       //Prompt the user for the next command
        // check for "exit" command
        if (!strcmp("exit", buf))
            break; //or exit(1);

        if (!strcmp("help", buf))
        {
            // if the user needs help
            openHelp();
            continue;       //Prompt the user for the next command
        }//end if

        convert_cmd();

        if (!strcmp("cd", command[0]))
        {
            if (cd(command[1]) < 0)
            {
                perror(command[1]);
            }

            // Skip the fork
            continue;       //Prompt the user for the next command
        }//end if

        child_pid = fork();

        if (-1 == child_pid)
        {//no child process
            printf("failed to create a child\n");
        }//end if
        else if (0 == child_pid)
        {// In Child process
            // execute a command
            if (execvp(command[0], command) < 0)
            {
                // handle errors
                perror(command[0]);
                exit(1);
            }//end if
        }//end else if
        else
        {//In parent Process
            log_write('o');// write to the log that a child process is started
            // wait for the command to finish if "&" is not present
            if (NULL == command[i])
            {
                waitpid(child_pid, NULL, 0);
                Zombie--;       //Buried Zombie

            }//end if
        }//end else

        bury_Zombies();     //bury zombies before reprompting if they existed
    }//end while

    return 0;
}//end main

/*Function Definition*/
void init_shell()
{
    fclose(fopen("log.txt", "w")); // to Open / clear the log file content before writing in it

    // Greeting shell during startup
    printf("\n\n\n\t  Welcom to our SHELL\n"
           " --------------------------------------------"
           "\n|write help to see commands that you can use|\n"
           " --------------------------------------------\n\n\n\n");

    sleep(1);       //No Need
}

void get_cmd()
{
    // get command from the user

    char *username = getenv("USER"); // get the user name
    printf("@%s>> ", username);
    fgets(buf, MAXCOM, stdin);
    // neglect the newline at the end of the command
    if ((strlen(buf) > 0) && (buf[strlen(buf) - 1] == '\n'))
        buf[strlen(buf) - 1] = '\0';
}

void openHelp()
{
    // open the help for the user
    printf("\n\nList of Commands supported:"
           "\n\t>ls"
           "\n\t>cd"
           "\n\t>& after the command --> to open the process in the background."
           "\n\t>exit --> to exit the shell"
           "\n\t>all other general commands available in UNIX shell \n\n\n");
}

void convert_cmd()
{
    // split string into command
    char *ptr;
    i = 0;
    ptr = strtok(buf, " ");
    while (ptr != NULL)
    {
        command[i] = ptr;
        i++;
        ptr = strtok(NULL, " ");
    }

    // check for "&"
    if (!strcmp("&", command[i - 1]))
    {
        command[i - 1] = NULL;
        command[i] = "&";
        sleep(1); // sleep untill run the process
    }
    else
    {
        command[i] = NULL;
    }
}

void handler(void)
{
    Zombie++;       //Not buried Zombie
    // writes to the log file when any child process is terminated
    log_write('c');

}

void log_write(char type){
    // writes to the log file when any child process is started or terminated
    FILE *pf;
    // open the file to wirte in it
    pf = fopen("log.txt", "a");
    if (pf == NULL)
        perror("Error opening file.");
    else if(type == 'c')// adds the trmination message to the file
       fprintf(pf, "A Child process was terminated.\n");
    else if(type == 'o')               // adds the starting message to the file
       fprintf(pf, "Child process %d was started.\n",child_pid);
    fclose(pf); // close the file
}

int cd(char *path)
{
    // for the cd command
    return chdir(path);
}

void bury_Zombies(void){
    //To Bury Zombie Processes
    while(Zombie >=1){wait(NULL);Zombie--;}
}
