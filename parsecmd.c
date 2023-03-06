/*
 * Unix Shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parsecmd.h"

char **parse_cmd_dynamic(const char *cmdline, int *bg) {
    // initialize variables
    char **result = NULL;
    int i, j, len, tok_start, tok_end; 

    // initialize a list to carry start and end indicies
    len = strlen(cmdline);
    int *indicies = malloc(sizeof(int)*len);
    int ind_count = 0;
    *bg = 0;

    // loop to get token indicies 
    int in_token = 0;
    for (i=0; i<=len; i++) {
        // runs if we are in a token 
        if (in_token == 1) {
            // checks if background symbol comes up and ends
            if (cmdline[i] == '&') {
                // sets background indicator
                *bg = 1;

                // stores token indicies
                tok_end = i-1;
                indicies[ind_count] = tok_start;
                indicies[ind_count+1] = tok_end;
                ind_count += 2;

                // breaks loop
                in_token = 2;
                i = len + 1;
            }
            else if (isspace(cmdline[i]) || cmdline[i] == '\0') {
                // change variables
                tok_end = i-1;
                in_token = 0;
                
                // stores token indicies
                indicies[ind_count] = tok_start;
                indicies[ind_count+1] = tok_end;
                ind_count += 2;
            }
        }
        // runs if we are searching for token 
        if (in_token == 0) {   
            // checks if background symbol comes up and ends
            if (cmdline[i] == '&') {
                // sets background indicator
                *bg = 1;
                
                // breaks loop
                in_token = 2;
                i = len + 1;
            } 
            else if (!(isspace(cmdline[i]))) { 
                // change variables
                tok_start = i;
                in_token = 1;
            }
        }
    }

    // memory allocation
    result = malloc(sizeof(char*)*(ind_count/2+1));

    // puts the tokens in the result list 
    for (i=0; i<(ind_count/2); i++) {
        // makes space for token
        len = indicies[i*2+1]-indicies[i*2]+1;
        result[i] = malloc(sizeof(char)*(len+1));
        // puts token in
        for (j=0; j<len; j++) {
            result[i][j] = cmdline[indicies[i*2]+j];
        }
        // add null terminator
        result[i][j] = '\0';
    }
    // makes last item in list null
    result[i] = NULL;

    // memory cleanup
    free(indicies);

    return result;
}
