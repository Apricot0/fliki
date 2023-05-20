#include <stdio.h>
#include <stdlib.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    /*printf("TEST PRINT: Return value: %d\nGlobal value: %d\nFile name: %s\n",x, (int)global_options,diff_filename);*/
    if(validargs(argc, argv)){
        fprintf(stderr,"ERROR: invalid arguments.\n");
        USAGE(*argv, EXIT_FAILURE);
    }
    if(global_options == HELP_OPTION)
        USAGE(*argv, EXIT_SUCCESS);
    FILE *diff = fopen(diff_filename, "r+");
    if(diff==NULL){
        fprintf(stderr,"ERROR: can not find the given diff file.\n");
        USAGE(*argv, EXIT_FAILURE);
    }
    if((global_options&NO_PATCH_OPTION)==NO_PATCH_OPTION){
        freopen("/dev/null", "w", stdout);
    }
    if((global_options&QUIET_OPTION)==QUIET_OPTION){
        freopen("/dev/null", "w", stderr);
    }
    if(patch(stdin,stdout,diff)==0){
        return EXIT_SUCCESS;
    }


/* debug
  *  FILE *file2 = fopen("rsrc/test1", "r+");
  * FILE *out = fopen("rsrc/example", "w");
  *  patch(file2,out,file);
  *  HUNK hunk = {HUNK_NO_TYPE,0,0,0,0,0};
  *  int hn = hunk_next(&hunk, file);
  *  while (hn != EOF && hn != ERR){
  *      char name = hunk_getc(&hunk, file);
  *      while(name != ERR){
  *          printf("%c(%d)",name,(int)name);
  *          name = hunk_getc(&hunk, file);
  *      }
  *      printf("\ndeletion:\n");
  *      for(int i = 0; i < 512; i++){
  *          if (i%10==0){
  *              printf("\n");
  *          }
  *          printf("%d, ",(int)hunk_deletions_buffer[i]);
  *      }
  *      printf("\naddition:\n");
  *       for(int i = 0; i < 512; i++){
  *          if (i%10==0){
  *              printf("\n");
  *          }
  *          printf("%d, ",(int)hunk_additions_buffer[i]);
  *      }
  *      hunk_show(&hunk,out);
  *      hn = hunk_next(&hunk, file);
  *  }
  *  if (hn == ERR){
  *       printf("ERROR");
  *  }
*
*/
    return EXIT_FAILURE;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
