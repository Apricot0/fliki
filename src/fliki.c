#include <stdlib.h>
#include <stdio.h>

#include "fliki.h"
#include "global.h"
#include "debug.h"

char current_char;
int point_to_data_start;
int has_deletion_field;
int has_addtion_field;
int has_divid_line;
int error_occur;
int w;
int x;
int y;
int z;
int *current_field;
int first_comma;
int type_indicator;
int second_comma;
char *word_counter;
char *buffer_write_in;
int buffer_full;
int compeletly_read;


/*debug only
 *static void print_hunk_info(HUNK *hp) {
 *   char *type_string;
 *   switch (hp->type) {
 *   case HUNK_APPEND_TYPE:
 *       type_string = "APPEND";
 *       break;
 *   case HUNK_DELETE_TYPE:
 *       type_string = "DELETE";
 *       break;
 *   case HUNK_CHANGE_TYPE:
 *       type_string = "CHANGE";
 *       break;
 *   case HUNK_NO_TYPE:
 *   default:
 *       type_string = "NO TYPE";
 *       break;
 *   }
 * printf("Hunk information:\n");
 *   printf("Type: %s\n", type_string);
 *   printf("Serial: %d\n", hp->serial);
 *   printf("Old start: %d\n", hp->old_start);
 *   printf("Old end: %d\n", hp->old_end);
 *   printf("New start: %d\n", hp->new_start);
 *   printf("New end: %d\n", hp->new_end);
*}
*/
static int is_digit(char c) {
    if (c >= '0' && c <= '9') {
        return 1;
    } else {
        return 0;
    }
}

static int set_adc(HUNK *hp,char c) {
    if(c=='a'){
        hp->type = HUNK_APPEND_TYPE;
        word_counter = hunk_additions_buffer;
        buffer_write_in = hunk_additions_buffer+2;
        return (1);
    }else if(c=='d'){
        hp->type = HUNK_DELETE_TYPE;
        word_counter = hunk_deletions_buffer;
        buffer_write_in = hunk_deletions_buffer+2;
        return (1);
    }else if(c=='c'){
        hp->type = HUNK_CHANGE_TYPE;
        word_counter = hunk_deletions_buffer;
        buffer_write_in = hunk_deletions_buffer+2;
        return (1);
    }else{
        return (0);
    }

}
static char type_to_adc(HUNK *hp){
    if(hp->type == HUNK_CHANGE_TYPE){
        return 'c';
    }else if(hp->type == HUNK_APPEND_TYPE){
        return 'a';
    }else if(hp->type == HUNK_DELETE_TYPE){
        return 'd';
    }else{
        return 0;
    }
}

static int put_in(HUNK *hp, char in){
    if (is_digit(in)){
        int num = *current_field;
        if(*current_field==-1){
            *current_field=in-'0';
        }else{
            *current_field=(num*10)+(in-'0');
        }
    }else if(type_indicator == 0&&set_adc(hp, in)){
        current_field=&y;
        type_indicator = 1;
    }else if(in==','){
        if(first_comma == 0&&type_indicator==0&&second_comma==0){
            first_comma=1;
            if(current_field==&w){
                current_field=&x;
            }else if(current_field==&y){
                current_field=&z;
            }else if(current_field==&x){
                current_field=&y;
            }
        }else if(type_indicator==1&&second_comma==0){
            second_comma=1;
            if(current_field==&w){
                current_field=&x;
            }else if(current_field==&y){
                current_field=&z;
            }else if(current_field==&x){
                current_field=&y;
            }
        }else{
            error_occur=1;
            return ERR;
        }
    }else if(in=='\n'){
        if(w==-1 || y==-1){
            error_occur=1;
            return ERR;
        }
        if(x==0){
            if (first_comma==1){
                error_occur=1;
                return ERR;
            }
            x=w;
        }
        if(z==0){
            if(second_comma==1){
                error_occur=1;
                return ERR;
            }
            z=y;
        }

        hp->serial++;
        hp->old_start=w;
        hp->old_end=x;
        hp->new_start=y;
        hp->new_end=z;
        /*debug print_hunk_info(hp);*/
    }else{
        error_occur=1;
        return ERR;
    }
    return(0);
}
/**
 * @brief Get the header of the next hunk in a diff file.
 * @details This function advances to the beginning of the next hunk
 * in a diff file, reads and parses the header line of the hunk,
 * and initializes a HUNK structure with the result.
 *
 * @param hp  Pointer to a HUNK structure provided by the caller.
 * Information about the next hunk will be stored in this structure.
 * @param in  Input stream from which hunks are being read.
 * @return 0  if the next hunk was successfully located and parsed,
 * EOF if end-of-file was encountered or there was an error reading
 * from the input stream, or ERR if the data in the input stream
 * could not be properly interpreted as a hunk.
 */

int hunk_next(HUNK *hp, FILE *in) {
    for (int i = 0; i < HUNK_MAX; i++) {
        *(hunk_deletions_buffer+i) = 0;
        *(hunk_additions_buffer+i) = 0;
    }
    has_addtion_field = 0;
    has_deletion_field = 0;
    has_divid_line = 0;
    error_occur=0;
    w=-1;
    x=0;
    y=-1;
    z=0;
    current_field=&w;
    first_comma=0;
    type_indicator =0;
    second_comma=0;
    buffer_full=0;
    compeletly_read=0;
    if((current_char == 0) && (hp->serial==0)){
        current_char = fgetc(in);
        if(current_char==EOF){
            return EOF;
        }
        while (current_char != '\n'){
            if (put_in(hp,current_char)==ERR){
                return ERR;
            }
            current_char = fgetc(in);
        }
        if (put_in(hp,current_char)==ERR){
            return ERR;
        }

    }else if(current_char == EOF){
        return EOF;
    }else{
        while(1){
            while(current_char != '\n'){
               current_char = fgetc(in);
            }
            current_char = fgetc(in);
            if (is_digit(current_char)){
                while (current_char != '\n'){
                    if (put_in(hp,current_char)==ERR){
                        return ERR;
                    }
                    current_char = fgetc(in);
                }
                if (put_in(hp,current_char)==ERR){
                    return ERR;
                }
                break;
            }else if (current_char == EOF){
                return EOF;
            }
        }
    }
    error_occur=0;
    point_to_data_start=1;
    return (0);
}

/**
 * @brief  Get the next character from the data portion of the hunk.
 * @details  This function gets the next character from the data
 * portion of a hunk.  The data portion of a hunk consists of one
 * or both of a deletions section and an additions section,
 * depending on the hunk type (delete, append, or change).
 * Within each section is a series of lines that begin either with
 * the character sequence "< " (for deletions), or "> " (for additions).
 * For a change hunk, which has both a deletions section and an
 * additions section, the two sections are separated by a single
 * line containing the three-character sequence "---".
 * This function returns only characters that are actually part of
 * the lines to be deleted or added; characters from the special
 * sequences "< ", "> ", and "---\n" are not returned.
 * @param hdr  Data structure containing the header of the current
 * hunk.
 *
 * @param in  The stream from which hunks are being read.
 * @return  A character that is the next character in the current
 * line of the deletions section or additions section, unless the
 * end of the section has been reached, in which case the special
 * value EOS is returned.  If the hunk is ill-formed; for example,
 * if it contains a line that is not terminated by a newline character,
 * or if end-of-file is reached in the middle of the hunk, or a hunk
 * of change type is missing an additions section, then the special
 * value ERR (error) is returned.  The value ERR will also be returned
 * if this function is called after the current hunk has been completely
 * read, unless an intervening call to hunk_next() has been made to
 * advance to the next hunk in the input.  Once ERR has been returned,
 * then further calls to this function will continue to return ERR,
 * until a successful call to call to hunk_next() has successfully
 * advanced to the next hunk.
 */

int hunk_getc(HUNK *hp, FILE *in) {
    int type = hp->type;
    if (error_occur==1){
        return ERR;
    }
    if(hp->serial==0){
        error_occur=1;
        return ERR;
    }
    if(point_to_data_start==1){
        current_char=fgetc(in);
        if(current_char==EOF){
            error_occur=1;
            return ERR;
        }
        switch(type){
        case HUNK_APPEND_TYPE:
            if(current_char=='>'){
                current_char = fgetc(in);
                if(current_char==' '){
                    has_addtion_field=1;
                    point_to_data_start=0;
                    break;
                }else{
                    error_occur=1;
                    return ERR;
                }
            }else{
                error_occur=1;
                return ERR;
            }
        case HUNK_CHANGE_TYPE:
        case HUNK_DELETE_TYPE:
            if(current_char=='<'){
                current_char = fgetc(in);
                if(current_char==' '){
                    has_deletion_field=1;
                    point_to_data_start=0;
                    break;
                }else{
                    error_occur=1;
                    return ERR;
                }
            }else{
                error_occur=1;
                return ERR;
            }

        }
    }else if(current_char == '\n'){
        if(buffer_full==0){
            word_counter =  buffer_write_in;
            buffer_write_in = word_counter+2;
        }
        current_char=fgetc(in);
        if(is_digit(current_char)){
            ungetc(current_char,in);
            current_char = '\n';
            error_occur=1;
            compeletly_read=1;
            return EOS;
        }else if(current_char == EOF){
            error_occur=1;
            compeletly_read=1;
            return EOS;
        }else{
            switch (type){
                case HUNK_CHANGE_TYPE:
                    if(current_char=='<'){
                        current_char = fgetc(in);
                        if(current_char==' '&&has_divid_line==0){
                            has_deletion_field=1;
                            break;
                        }else{
                            error_occur=1;
                            return ERR;
                        }
                    }else if(current_char=='-'){
                        char c1 = fgetc(in);
                        char c2 = fgetc(in);
                        char c3 = fgetc(in);
                        if(c1=='-'&&c2=='-'&&c3=='\n'&&has_divid_line==0){
                            current_char = '\n';
                            has_divid_line=1;
                            buffer_write_in = hunk_additions_buffer;
                            buffer_full=0;
                            return EOS;
                        }else{
                            error_occur=1;
                            return ERR;
                        }
                    }else if(current_char == '>'){
                        current_char = fgetc(in);
                        if(current_char == ' '&&has_deletion_field == 1&&has_divid_line==1){
                            has_addtion_field=1;
                            break;
                        }else{
                            error_occur=1;
                            return ERR;
                        }
                    }else{
                        error_occur=1;
                        return ERR;
                    }
                case HUNK_APPEND_TYPE:
                    if(current_char=='>'){
                        current_char = fgetc(in);
                        if(current_char==' '){
                            has_addtion_field=1;
                            point_to_data_start=0;
                            break;
                        }else{
                            error_occur=1;
                            return ERR;
                        }
                    }else{
                        error_occur=1;
                        return ERR;
                    }
                case HUNK_DELETE_TYPE:
                    if(current_char=='<'){
                        current_char = fgetc(in);
                        if(current_char==' '){
                            has_deletion_field=1;
                            point_to_data_start=0;
                            break;
                        }else{
                            error_occur=1;
                            return ERR;
                        }
                    }else{
                        error_occur=1;
                        return ERR;
                    }

            }
        }
    }
    current_char = fgetc(in);
    if(current_char==EOF){
        return ERR;
    }
    if(buffer_full == 0){
        *buffer_write_in = current_char;
        buffer_write_in = buffer_write_in+1;
        if(buffer_write_in==hunk_deletions_buffer+HUNK_MAX-2||buffer_write_in==hunk_additions_buffer+HUNK_MAX-2){
            buffer_full=1;
        }
        if(*word_counter == 0xff){
            *(word_counter+1)=*(word_counter+1)+1;
            *(word_counter)=0x00;
        }else{
            *word_counter = *word_counter+0x1;
        }
    }

    return current_char;
}

/**
 * @brief  Print a hunk to an output stream.
 * @details  This function prints a representation of a hunk to a
 * specified output stream.  The printed representation will always
 * have an initial line that specifies the type of the hunk and
 * the line numbers in the "old" and "new" versions of the file,
 * in the same format as it would appear in a traditional diff file.
 * The printed representation may also include portions of the
 * lines to be deleted and/or inserted by this hunk, to the extent
 * that they are available.  This information is defined to be
 * available if the hunk is the current hunk, which has been completely
 * read, and a call to hunk_next() has not yet been made to advance
 * to the next hunk.  In this case, the lines to be printed will
 * be those that have been stored in the hunk_deletions_buffer
 * and hunk_additions_buffer array.  If there is no current hunk,
 * or the current hunk has not yet been completely read, then no
 * deletions or additions information will be printed.
 * If the lines stored in the hunk_deletions_buffer or
 * hunk_additions_buffer array were truncated due to there having
 * been more data than would fit in the buffer, then this function
 * will print an elipsis "..." followed by a single newline character
 * after any such truncated lines, as an indication that truncation
 * has occurred.
 *
 * @param hp  Data structure giving the header information about the
 * hunk to be printed.
 * @param out  Output stream to which the hunk should be printed.
 */

void hunk_show(HUNK *hp, FILE *out) {
    char *hunk_pointer;
    fprintf(out,"%d",hp->old_start);
    if(hp->old_end!=hp->old_start){
        fprintf(out,",%d",hp->old_end);
    }
    fputc(type_to_adc(hp),out);
    fprintf(out,"%d",hp->new_start);
    if(hp->new_end!=hp->new_start){
        fprintf(out,",%d",hp->new_end);
    }
    fputc('\n',out);
    if(compeletly_read==1){
        if(has_deletion_field==1){
            hunk_pointer = hunk_deletions_buffer+2;
            fputc('<',out);
            fputc(' ',out);
            while(1){
                if(*hunk_pointer==0x0){
                    if(*(hunk_pointer+1)==0x0){
                        break;
                    }
                }
                if(*hunk_pointer=='\n'){
                    fputc('\n',out);
                    if(*(hunk_pointer+1)==0x0&&*(hunk_pointer+2)==0x0){
                        break;
                    }
                    fputc('<',out);
                    fputc(' ',out);
                    hunk_pointer=hunk_pointer+2;
                }else{
                    fputc(*hunk_pointer,out);
                }
                hunk_pointer++;
            }
            if(has_addtion_field==1){
                fputc('-',out);
                fputc('-',out);
                fputc('-',out);
                fputc('\n',out);
            }
        }
        if(has_addtion_field==1){
            hunk_pointer = hunk_additions_buffer+2;
            fputc('>',out);
            fputc(' ',out);
            while(1){
                if(*hunk_pointer==0x0){
                    if(*(hunk_pointer+1)==0x0){
                        break;
                    }
                }
                if(*hunk_pointer=='\n'){
                    fputc('\n',out);
                    if(*(hunk_pointer+1)==0x0&&*(hunk_pointer+2)==0x0){
                        break;
                    }
                    fputc('>',out);
                    fputc(' ',out);
                    hunk_pointer=hunk_pointer+2;
                }else{
                    fputc(*hunk_pointer,out);
                }
                hunk_pointer++;
            }
        }

    }
}

/**
 * @brief  Patch a file as specified by a diff.
 * @details  This function reads a diff file from an input stream
 * and uses the information in it to transform a source file, read on
 * another input stream into a target file, which is written to an
 * output stream.  The transformation is performed "on-the-fly"
 * as the input is read, without storing either it or the diff file
 * in memory, and errors are reported as soon as they are detected.
 * This mode of operation implies that in general when an error is
 * detected, some amount of output might already have been produced.
 * In case of a fatal error, processing may terminate prematurely,
 * having produced only a truncated version of the result.
 * In case the diff file is empty, then the output should be an
 * unchanged copy of the input.
 *
 * This function checks for the following kinds of errors: ill-formed
 * diff file, failure of lines being deleted from the input to match
 * the corresponding deletion lines in the diff file, failure of the
 * line numbers specified in each "hunk" of the diff to match the line
 * numbers in the old and new versions of the file, and input/output
 * errors while reading the input or writing the output.  When any
 * error is detected, a report of the error is printed to stderr.
 * The error message will consist of a single line of text that describes
 * what went wrong, possibly followed by a representation of the current
 * hunk from the diff file, if the error pertains to that hunk or its
 * application to the input file.  If the "quiet mode" program option
 * has been specified, then the printing of error messages will be
 * suppressed.  This function returns immediately after issuing an
 * error report.
 *
 * The meaning of the old and new line numbers in a diff file is slightly
 * confusing.  The starting line number in the "old" file is the number
 * of the first affected line in case of a deletion or change hunk,
 * but it is the number of the line *preceding* the addition in case of
 * an addition hunk.  The starting line number in the "new" file is
 * the number of the first affected line in case of an addition or change
 * hunk, but it is the number of the line *preceding* the deletion in
 * case of a deletion hunk.
 *
 * @param in  Input stream from which the file to be patched is read.
 * @param out Output stream to which the patched file is to be written.
 * @param diff  Input stream from which the diff file is to be read.
 * @return 0 in case processing completes without any errors, and -1
 * if there were errors.  If no error is reported, then it is guaranteed
 * that the output is complete and correct.  If an error is reported,
 * then the output may be incomplete or incorrect.
 */

int patch(FILE *in, FILE *out, FILE *diff) {
    char c_from_filein;
    char c_from_diff;
    int old_line_number=1;
    int new_line_number=1;
    int operating_line=0;
    HUNK hp = {HUNK_NO_TYPE,0,0,0,0,0};
    int hn = hunk_next(&hp, diff);
    if(hn==EOF||hn==ERR){
        fprintf(stderr, "ERROR: can not get the first hunk\n");
        return -1;
    }
    operating_line = hp.old_start;
    while(1){
        if(hp.old_end<hp.old_start||hp.new_end<hp.new_start){
            fprintf(stderr, "ERROR: hunk has illformat line number: (start > end)\n");
            hunk_show(&hp,stderr);
            return -1;
        }
        switch(hp.type){
            case HUNK_CHANGE_TYPE:
                if(operating_line<old_line_number){
                    fprintf(stderr,"ERROR: hunks don't have correct order.\n");
                    hunk_show(&hp,stderr);
                    return -1;
                }
                if(old_line_number==operating_line){
                    if(hp.new_start!=new_line_number){
                        fprintf(stderr,"ERROR: new start not match\n");
                        hunk_show(&hp,stderr);
                        return -1;
                    }
                    c_from_diff=hunk_getc(&hp,diff);
                    c_from_filein=fgetc(in);
                    while(c_from_diff!=EOS){
                        /*debug
                         *printf("%c,%c\n",c_from_diff,c_from_filein);
                         */
                        if(c_from_filein!=c_from_diff){
                            fprintf(stderr,"ERROR: deletion line info not match\n");
                            hunk_show(&hp,stderr);
                            return -1;
                        }
                        if(c_from_diff=='\n'){
                            old_line_number++;
                        }
                        c_from_diff=hunk_getc(&hp,diff);
                        c_from_filein=fgetc(in);
                        if(c_from_diff==ERR){
                            fprintf(stderr,"ERROR: can not get next char (illformat hunk data)\n");
                            hunk_show(&hp,stderr);
                            return -1;
                        }
                    }
                    if(hp.old_end!=old_line_number-1){
                        fprintf(stderr,"ERROR: old end not match\n");
                        hunk_show(&hp,stderr);
                        return -1;
                    }
                    c_from_diff=hunk_getc(&hp,diff);
                    while(c_from_diff!=EOS){
                        if(c_from_diff=='\n'){
                            new_line_number++;
                        }
                        fputc(c_from_diff,out);
                        c_from_diff=hunk_getc(&hp,diff);
                        if(c_from_diff==ERR){
                            fprintf(stderr,"ERROR: can not get next char (illformat hunk data)\n");
                            hunk_show(&hp,stderr);
                            return -1;
                        }
                    }
                    if(hp.new_end!=new_line_number-1){
                        fprintf(stderr,"ERROR: new end not match\n");
                        hunk_show(&hp,stderr);
                        return -1;
                    }
                    hn=hunk_next(&hp,diff);
                    if(hn!=EOF&&hn!=ERR){
                        operating_line = hp.old_start;
                    }
                    ungetc(c_from_filein,in);
                }else{
                    if(c_from_filein!=EOF){
                        c_from_filein=fgetc(in);
                        if(c_from_filein!=EOF){
                            fputc(c_from_filein,out);
                        }
                        if (c_from_filein=='\n'){
                            old_line_number++;
                            new_line_number++;
                        }
                    }
                }
                break;

            case HUNK_APPEND_TYPE:
                if(operating_line<old_line_number-1){
                    fprintf(stderr, "ERROR: hunks dont have correct order.\n");
                    hunk_show(&hp,stderr);
                    return -1;
                }
                if(hp.old_start!=hp.old_end){
                    fprintf(stderr,"ERROR: invalid append header\n");
                    hunk_show(&hp,stderr);
                    return -1;
                }
                if(old_line_number==operating_line+1){
                    if(hp.new_start!=new_line_number){
                        fprintf(stderr,"ERROR: new start not match\n");
                        hunk_show(&hp,stderr);
                        return -1;
                    }
                    c_from_diff=hunk_getc(&hp,diff);
                    while(c_from_diff!=EOS){
                        if(c_from_diff=='\n'){
                            new_line_number++;
                        }
                        fputc(c_from_diff,out);
                        c_from_diff=hunk_getc(&hp,diff);
                        if(c_from_diff==ERR){
                            fprintf(stderr,"ERROR: can not get next char (illformat hunk data)\n");
                            hunk_show(&hp,stderr);
                            return -1;
                        }
                    }
                    if(hp.new_end!=new_line_number-1){
                        fprintf(stderr,"ERROR: new end not match\n");
                        hunk_show(&hp,stderr);
                        return -1;
                    }
                    hn=hunk_next(&hp,diff);
                    if(hn!=EOF&&hn!=ERR){
                        operating_line = hp.old_start;
                    }
                }else{
                    if(c_from_filein!=EOF){
                        c_from_filein=fgetc(in);
                        if(c_from_filein!=EOF){
                            fputc(c_from_filein,out);
                        }
                        if (c_from_filein=='\n'){
                            old_line_number++;
                            new_line_number++;
                        }
                    }
                }
                break;



            case HUNK_DELETE_TYPE:
                if(operating_line<old_line_number){
                    fprintf(stderr, "ERROR: hunks dont have correct order.\n");
                    hunk_show(&hp,stderr);
                    return -1;
                }
                if(hp.new_start!=hp.new_end){
                    fprintf(stderr,"ERROR: invalid delete header\n");
                    hunk_show(&hp,stderr);
                    return -1;
                }
                if(operating_line==old_line_number){
                    if(hp.new_start!=new_line_number-1){
                        fprintf(stderr,"ERROR: new start not match\n");
                        hunk_show(&hp,stderr);
                        return -1;
                    }
                    c_from_diff=hunk_getc(&hp,diff);
                    c_from_filein=fgetc(in);
                    while(c_from_diff!=EOS){
                         /*debug
                          *printf("%c,%c\n",c_from_diff,c_from_filein);
                          */
                        if(c_from_filein!=c_from_diff){
                            fprintf(stderr,"ERROR: deletion line info not match\n");
                            hunk_show(&hp,stderr);
                            return -1;
                        }
                        if(c_from_diff=='\n'){
                            old_line_number++;
                        }
                        c_from_diff=hunk_getc(&hp,diff);
                        c_from_filein=fgetc(in);
                        if(c_from_diff==ERR){
                            fprintf(stderr,"ERROR: can not get next char (illformat hunk data)\n");
                            hunk_show(&hp,stderr);
                            return -1;
                        }
                    }
                    if(hp.old_end!=old_line_number-1){
                        fprintf(stderr,"ERROR: old end not match\n");
                        return -1;
                    }
                    ungetc(c_from_filein,in);
                    hn=hunk_next(&hp,diff);
                    if(hn!=EOF&&hn!=ERR){
                        operating_line = hp.old_start;
                    }
                }else{
                    if(c_from_filein!=EOF){
                        c_from_filein=fgetc(in);
                        if(c_from_filein!=EOF){
                            fputc(c_from_filein,out);
                        }
                        if (c_from_filein=='\n'){
                            old_line_number++;
                            new_line_number++;
                        }
                    }
                }
                break;



            default:
                fprintf(stderr,"ERROR: hunk type not defined\n");
                hunk_show(&hp,stderr);
                return -1;
        }
        if(hn==ERR){
            fprintf(stderr, "ERROR: unable to read the next hunk\n");
            return -1;
        }
        if(hn==EOF){
            while(c_from_filein!=EOF){
                c_from_filein=fgetc(in);
                if(c_from_filein!=EOF){
                    fputc(c_from_filein,out);
                }
            }
            break;
        }
    }
    return 0;

}
