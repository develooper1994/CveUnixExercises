/*
AUTHOR: Mustafa Selçuk Çağlar

mycat [-t(text)|-x(hex)|-o(octal)] [-c or --bytes < number >, default: 10] [-n or --lines < number >, default: 10] [-d or --order < ascending >=0 | descending order <0 >, default: ascending] [-e or --enumurate < enumurate >=0 | not enumurate <0 >, default: not enumurate] [files]

Seçenekler:

[argümansız]
-t (text) (default) (thick)
-o (octal) (thick)
-x (hex) (thick)
-h or --help (thick)
-v or --verbose (thick)

[argümanlı]
-c or --bytes, default n = 10 (thick)
-n or --lines, default n = 10
-d or --order < ascending >=0 | descending order <0 >, default order = ascending
-e or --enumurate < enumurate >=0 | not enumurate <0 >, default: not enumurate

Burada -t "text olarak yazdır",
-o "ocatal olarak yazdır,
-x "hex olarak yazdır anlamına gelmektedir. Bu seçeneklerden yalnızca bir tanesi belirtilebilir ve bu seçeneklerden hiçbiri belirtilmemişse -t seçeneği belirtilmiş gibi işlem yapılmalıdır.

--bytes isteğe bağlı (optional) argüman alabilen uzun bir seçenektir. Dosyanın başındaki ilk n satırı yazdırır. Bu uzun seçeneğin default değeri 10'dur. --verbose seçeneği birden fazla dosyanın yazdırıldığı durumda dosya isimlerinin de basılmasını sağlamaktadır. Programın örnek bir gerçekleştirim şöyle olabilir:

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

// symbolic constants
#define DEFAULT_LINE          10
#define DEFAULT_BYTE_LENGHT   10
#define HEX_OCTAL_LINE_LENGHT 16

// order
#define ASCENDING_ORDER       0 // ascending order >=0
#define DESCENDING_ORDER      -1 // descending order <0
#define DEFAULT_ORDER         ASCENDING_ORDER

// enumurate
#define ENUMERATE             1 // enumurate >=0

#define PRINT_OCTAL           0
#define PRINT_HEX             1

#define DELIM   '\n'

// function prototypes
int print_text(FILE *fp, const int n, int ch, int order);
int print_hex_octal(FILE* fp, const int n, int ch, int hexflag, int order);
long filesize(FILE* f);

//int print_text_last(FILE *fp, const int nline);
//int print_hex_octal_last(FILE* f, const int nline, int hexflag);

int main(int argc, char *argv[])
{
    // create options, flags and arguments and other variables
    int result=0, err_flag=0;
    int t_flag=0, o_flag=0, x_flag=0, bytes_flag=0, lines_flag=0, order_flag=0, enumurate_flag=0, verbose_flag=0, help_flag=0;
    char *bytes_arg, *lines_arg, *order_arg, *enumurate_arg;

    struct option long_options[] = {
        {"bytes", optional_argument, NULL, 'c'},
        {"lines", optional_argument, NULL, 'n'},
        {"order", optional_argument, NULL, 'd'},
        {"enumurate", optional_argument, NULL, 'e'},
        {"verbose", no_argument, NULL, 'v'},
        {"help", no_argument, NULL, 'h'},
        {0,0,0,0}
    };
    char options[] = "xotvhcn:d:e:";
    FILE* fp;
    int index=0;
    int nbyte=-1, nline=-1, order=DEFAULT_ORDER;

    opterr=0;
    // parse command line arguments
    do{
        result = getopt_long(argc, argv, options, long_options, NULL);
        switch(result){
            case 'x':
                x_flag = 1;
                break;
            case 'o':
                o_flag = 1;
                break;
            case 't': // default
                t_flag = 1;
                break;
            case 'c':
                bytes_flag = 1;
                bytes_arg = optarg;
                break;
            case 'n':
                lines_flag = 1;
                lines_arg = optarg;
                break;
            case 'd':
                order_flag = 1;
                order_arg = optarg;
                break;
            case 'e':
                enumurate_flag = 0;
                enumurate_arg = optarg;
                break;
            case 'v':
                verbose_flag = 1;
                break;
            case 'h':
                help_flag = 1;
                fprintf(stdout, "mycat [-t(text)|-x(hex)|-o(octal)] [-c or --bytes < number >, default: 10] [-n or --lines < number >, default: 10] [-d or --order < ascending >=0 | descending order <0 >, default: ascending] [-e or --enumurate < enumurate >=0 | not enumurate <0 >, default: not enumurate] [files]");
                break;
            case '?':
                // parsing error check
                if(optopt != 0)
                    fprintf(stderr, "invalid switch -%c\n", optopt);
                else if(optopt == 'c') // --bytes
                    fprintf(stderr, "-c or --bytes option without argument\n");
                else if(optopt == 'n') // --lines
                    fprintf(stderr, "-n or --lines option without argument\n");
                else if(optopt == 'd') // --order
                    fprintf(stderr, "-d or --order option without argument\n");
                else
                    fprintf(stderr, "invalid switch --%s\n", argv[optopt - 1]); // not a portable way to check
                err_flag = 1;
                break;
            // note don't use default statement
        }
    }while(result != -1);

    // switch error check
    if(x_flag + o_flag + t_flag > 1){
        fprintf(stderr, "only one option must be specified from -o, -t, -x\n");
        err_flag = 1;
        //exit(EXIT_FAILURE);
    }
    if(bytes_flag + lines_flag > 1){
        fprintf(stderr, "only one option must be specified from -c, -n\n");
        err_flag = 1;
        //exit(EXIT_FAILURE);
    }
    if(optind == argc){
        // argc and optind only equal to 1 only if nothing specified
        fprintf(stderr, "At least one argument must be specified\n");
        err_flag = 1;
        //exit(EXIT_FAILURE);
    }

    // exit according to error check
    if(err_flag!= 0){
        exit(EXIT_FAILURE);
    }

    // set defaults
    if(bytes_flag){
        // default nbyte value
        nbyte = bytes_arg != NULL ? 
        (int)strtol(bytes_arg, NULL, 10) :
        DEFAULT_BYTE_LENGHT;
    }
    if(lines_flag){
        // default nline value
        nline = bytes_arg != NULL ? 
        (int)strtol(bytes_arg, NULL, 10) :
        DEFAULT_BYTE_LENGHT;
    }
    if(order_flag){
        // default order value
        order = order_arg != NULL ? 
        (int)strtol(order_arg, NULL, 10) :
        DEFAULT_ORDER;
    }
    if(enumurate_flag){
        // default not enumurate
        order = order_arg != NULL ? 
        (int)strtol(enumurate_arg, NULL, 10) :
        ENUMERATE;
    }

    if(x_flag + o_flag + t_flag == 0){
        // non of them specified. default = text(t_flag)
        t_flag = 1;
    }
    if(bytes_flag + lines_flag == 0){
        // non of them specified. default = text(t_flag)
        lines_flag = 1;
    }

    // process according to flags
    for(index = optind; index < argc; ++index){
        // open files if file couldn't open continue to next file
        char* filename = argv[index];
        fp = fopen(filename, "rb");
        if(fp == NULL){
            fprintf(stderr, "Cannot open file %s\n", filename);
            continue;
        }
        // print filenames
        if(verbose_flag){
            printf("=====> %s <=====\n\n", filename);
        }

        // how do you want to print?
        if(bytes_flag){
            // print_byte_text
            if(t_flag)
                result = print_text(fp, nbyte, 0, order);
            else if(x_flag)
                result = print_byte_hex_octal(fp, nbyte, PRINT_HEX, order);
            else if(o_flag)
                result = print_hex_octal(fp, nbyte, PRINT_OCTAL, order);
            else
                result = print_text(fp, nbyte, 0, order);
        }
        else if(lines_flag){
            // print_line_text
            if(t_flag)
                result = print_text(fp, nline, DELIM, order);
            else if(x_flag)
                result = print_hex_octal(fp, nline, PRINT_HEX, order);
            else if(o_flag)
                result = print_hex_octal(fp, nline, PRINT_OCTAL, order);
            else
                result = print_text(fp, nline, DELIM, order);
        }
        /*
        else{
            if(t_flag)
                result = print_text_last(fp, nbyte, nline);
            else if(x_flag)
                result = print_hex_octal_last(fp, nbyte, nline, PRINT_HEX);
            else if(o_flag)
                result = print_hex_octal_last(fp, nbyte, nline, PRINT_OCTAL);
            else
                result = print_text_last(fp, nbyte, nline);
        }
        */

        if(index != argc -1)
            putchar('\n');
        // read error
        if(!result)
            fprintf(stderr, "cannot read the file %s\n", filename);

        fclose(fp);
    }

    /*
    // arguments without options. they are filenames
    if(optind != argc){
        puts("arguments without options: ");
    }
    */


    exit(EXIT_SUCCESS);
}


// text
int print_text(FILE *fp, const int n, int ch, int order){
    // n == -1 => print all file
    long byte_count = 0, line_count = 0;
    if(ch != DELIM)
        ch=0;
    const long file_size = filesize(fp);
    if(order>=0)
        fseek(fp, 0, SEEK_SET);
    else
        fseek(fp, 0, SEEK_END); // fgetc reads and move cursor +1

    int read_until_byte=n;
    //if(read_until_byte<0) read_until_byte = file_size - n;

    if(read_until_byte == -1){
        // print all file
        for(; byte_count < file_size; ++byte_count){
        //while(ch != EOF){
            ch = fgetc(fp);
            if(order>=0) fseek(fp, -2, SEEK_CUR);
            putchar(ch);
        }
    }
    else{
        for(; (byte_count < file_size) && byte_count < read_until_byte; ++byte_count){
        //for(; (ch != EOF) && count < read_until_byte; ++count){
            ch = fgetc(fp);
            if(order>=0) fseek(fp, -2, SEEK_CUR);
            if(ch == DELIM) ++line_count;
            putchar(ch);
        }
    }
    return !ferror(fp);
}

// hex - octal
int print_hex_octal(FILE* fp, const int n, int ch, int hexflag, int order){
    // n == -1 => print all file
    if(ch != DELIM) ch=0;
    int line_mod=0;
    long byte_count=0;
    const char *off_str, *ch_str;

    off_str = hexflag ? "%07X" : "%012o";
    ch_str = hexflag? "%02X%c" : "%03o%c";

    const long file_size = filesize(fp);
    if (order >= 0) {
        fseek(fp, 0, SEEK_SET);
    }
    else {
        fseek(fp, 0, SEEK_END);
    }

    int read_until_byte=n;
    //if(read_until_byte<0) read_until_byte = file_size - n;

    if(read_until_byte == -1){
        // print all file
        for(; byte_count < file_size; ++byte_count){
        //for(; ch!=EOF; ++bye_count){
            ch = fgetc(fp);
            if(order>=0) fseek(fp, -2, SEEK_CUR);

            line_mod = byte_count % HEX_OCTAL_LINE_LENGHT;
            if(line_mod == 0)
                printf(off_str, byte_count);
            printf(ch_str, ch, line_mod == HEX_OCTAL_LINE_LENGHT - 1 ? '\n': ' ');
        }
    }
    else{
        int line_mod_count = 0;
        for(; (byte_count < file_size) && line_mod_count < read_until_byte; ++byte_count){
        //for(; ch != EOF && line_mod_count < read_until_byte; ++bye_count){
            ch = fgetc(fp);
            if(order>=0) fseek(fp, -2, SEEK_CUR);

            line_mod = byte_count % HEX_OCTAL_LINE_LENGHT;
            if(line_mod == 0)
                printf(off_str, byte_count);
            printf(ch_str, ch, line_mod == HEX_OCTAL_LINE_LENGHT - 1 ? '\n': ' ');

            if(ch==DELIM) ++line_mod_count;
        }
    }

    line_mod = byte_count % HEX_OCTAL_LINE_LENGHT;
    if(line_mod != 0)
        putchar('\n');

    return !ferror(fp);
}

long filesize(FILE* f) {
    fseek(f, 0, SEEK_END); // seek to end of file
    long size = ftell(f); // get current file pointer
    fseek(f, 0, SEEK_SET); // seek back to beginning of file
    return size;
}


// https://www.geeksforgeeks.org/print-last-10-lines-of-a-given-file/
 
/* Function to print last n lines of a given string */
void print_last_lines(char *str, int n)
{
    /* Base case */
    if (n <= 0)
       return;
 
    size_t cnt  = 0; // To store count of '\n' or DELIM
    char *target_pos   = NULL; // To store the output position in str
 
    /* Step 1: Find the last occurrence of DELIM or '\n' */
    target_pos = strrchr(str, DELIM);
 
    /* Error if '\n' is not present at all */
    if (target_pos == NULL){
        fprintf(stderr, "ERROR: string doesn't contain '\\n' character\n");
        return;
    }
 
    /* Step 2: Find the target position from where we need to print the string */
    while (cnt < n){
        // Step 2.a: Find the next instance of '\n'
        while (str < target_pos && *target_pos != DELIM)
            --target_pos;
 
         /* Step 2.b: skip '\n' and increment count of '\n' */
        if (*target_pos ==  DELIM)
            --target_pos, ++cnt;
 
        /* str < target_pos means str has less than 10 '\n' characters,
           so break from loop */
        else break;
    }
 
    /* In while loop, target_pos is decremented 2 times, that's why target_pos + 2 */
    if (str < target_pos)
        target_pos += 2;
 
    // Step 3: Print the string from target_pos
    printf("%s\n", target_pos);
}
