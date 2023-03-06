/*
 * Unix Shell
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "parsecmd.h"

/* The maximum size of your circular history queue. */
#define MAXHIST 10

/*
 * A struct to keep information one command in the history of
 * command executed
 */
struct histlist_t {
    unsigned int cmd_num;
    char cmdline[MAXLINE]; // command line for this process
};

/* Global variables declared here.
 * For this assignment, only use globals to manage history list state.
 * all other variables should be allocated on the stack or heap.
 *
 * Recall "static" means these variables are only in scope in this .c file. */
static struct histlist_t history[MAXHIST];

// TODO: add some more global variables for history tracking.
//       These should be the only globals in your program.
static int queue_next = 0;  // the next place to insert in the queue
static int queue_size = 0;  // the number of items in the queue
static unsigned int current_cmd = 0;  // the number of items in the queue

// TODO: add your function prototypes here.

void sigchld_handler(int sig) {
        pid_t pid;
        int status;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {}

    }

int main(int argc, char **argv) {
    // initialize variables
    char cmdline[MAXLINE];
    char **args;
    int bg;

    // signal handling
    if (signal(SIGCHLD, sigchld_handler) == SIG_ERR) {
        printf("SIGCHLD error");
        exit(1);
    }


    while (1) {
        // (1) print the shell prompt
        printf("cs31shell> ");
        fflush(stdout);

        // (2) read in the next command entered by the user
        if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)) {
            perror("fgets error");
        }
        if (feof(stdin)) { /* End of file (ctrl-D) */
            fflush(stdout);
            exit(0);
        }


        // (3) TODO: make a call to your parsing library to parse it into its
        //     args format
        if (strcmp(cmdline, "\n") != 0) {
            int qfork = 1;
            args = parse_cmd_dynamic(cmdline, &bg);

            // (4) TODO: determine how to execute it, and then execute it

            // if user uses !num command
            if (args[0][0] == '!') {
                // changes string to number for indexing
                char* temp = strdup(args[0]);
                char* using = temp+1;
                int histNum = atoi(using);

                //memory cleanup
                free(temp);

                // checks if number is in history range
                if ((0<=histNum) && (histNum<=current_cmd)) {
                    // find historical commandline 
                    for (int i=0; i<queue_size; i++) {
                        if (history[i].cmd_num == histNum) {
                            // finds length of args
                            int len = 0;
                            while (args[len] != NULL) {
                                len++;
                            }                        
                            
                            // remakes commandline and reparses 
                            char newcmdline[MAXLINE];
                            strcpy(newcmdline, history[i].cmdline);
                            strcpy(cmdline, newcmdline);
                            
                            // memory cleanup
                            int p = 0;
                            // frees each item in array
                            while (args[p] != NULL) {
                                free(args[p]);
                                p++;
                            }
                            // frees array
                            free(args);
                            
                            args = parse_cmd_dynamic(cmdline, &bg);

                            // breaks loop 
                            break;
                        }
                    }
                }
                else {
                    // prints error message 
                    printf("bash: %s: event not found\n", args[0]);
                    fflush(stdout);
                }
            }


            // if user calls exit
            char* exits = "exit";
            if (strcmp(args[0], exits) == 0) {
                qfork = 0;
                // memory cleanup
                int p = 0;
                // frees each item in array
                while (args[p] != NULL) {
                    free(args[p]);
                    p++;
                }
                // frees array
                free(args);

                //exits 
                exit(0);
            }
            
            // history addition
            if (!(args[0][0] == '!')) {
                // updates structs in the list 
                history[queue_next].cmd_num = current_cmd;
                strcpy(history[queue_next].cmdline, cmdline); 
                // circle list thingy
                if (queue_size < MAXHIST){
                    queue_size += 1;
                }
                queue_next = (queue_next+1) % MAXHIST;
                current_cmd++;
            }
            
            // if user calls history 
            char* hist = "history";
            if (strcmp(args[0], hist) == 0) {
                qfork = 0;
                // printing if history is not full 
                if (queue_size<MAXHIST){
                    for (int i=0; i<queue_size; i++) {
                        printf("%d  %s", i, history[i].cmdline);
                        fflush(stdout);
                    }
                }
                // printing if history is full
                else { 
                    for (int i=0; i<queue_size; i++) {
                        printf("%d  %s", history[queue_next].cmd_num,
                        history[queue_next].cmdline);
                        fflush(stdout);
                        queue_next = (queue_next+1) % MAXHIST;
                    }
                }
                // for neatness
                printf("\n");
                fflush(stdout);
            }

            // forking stuff :)
            if (qfork == 1) {
                pid_t child_pid;
                int child_status;

                // foreground running
                if (bg == 0) {
                    child_pid = fork();
                    if (child_pid<0) {
                        printf("Forking child process failed ");
                        fflush(stdout);
                        exit(1);
                    }
                    else if (child_pid == 0) {
                        // run by child
                        execvp(args[0], args);
                        // if execvp fails
                        printf("Unknown command\n");
                        fflush(stdout);

                        // memory cleanup
                        int p = 0;
                        // frees each item in array
                        while (args[p] != NULL) {
                            free(args[p]);
                            p++;
                        }
                        // frees array
                        free(args);

                        exit(1);
                    }
                    else {
                        waitpid(child_pid, &child_status, 0);
                    }
                }
                // running in background
                else {
                    child_pid = fork();
                    // checks if runs correctly 
                    if (child_pid<0) {
                        printf("Forking child process failed ");
                        fflush(stdout);
                        exit(1);
                    }
                    else if (child_pid == 0) {
                        // run by child
                        execvp(args[0], args);
                        // if execvp fails
                        printf("Unknown command\n");
                        fflush(stdout);
                        exit(1);
                    }
                }
                


            }
            
            // memory cleanup
            int p = 0;
            // frees each item in array
            while (args[p] != NULL) {
                free(args[p]);
                p++;
            }
            // frees array
            free(args);
            
        }

    }

    return 0;
}
