#include <stdlib.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 * @modifies global variable "diff_filename" to point to the name of the file
 * containing the diffs to be used.
 */

int validargs(int argc, char **argv) {
    global_options = 0x0;
    diff_filename = NULL;
    if (argc == 1){
        return (-1);
    }
    for (int i = 1; i < argc; i++){
        if (*(*(argv + i) + 0) == '-'){
            if (*(*(argv + i) + 2) != '\0'){
                return (-1);
            }else{
                if (*(*(argv + i) + 1) == 'h'){
                    if (global_options != 0){
                        return (-1);
                    }
                    global_options = HELP_OPTION;
                    return (0);
                }else if(*(*(argv + i) + 1) == 'n'){
                    global_options = global_options | NO_PATCH_OPTION;
                }else if(*(*(argv + i) + 1) == 'q'){
                    global_options = global_options | QUIET_OPTION;
                }else{
                    return (-1);
                }
            }
        }else{
            if (i != argc -1 || diff_filename != NULL){
                return (-1);
            }else{
                diff_filename =  *(argv + i);
            }
        }
    }
    if (diff_filename == NULL){
        global_options = 0x0;
        return (-1);
    }else{
        return (0);
    }
}
