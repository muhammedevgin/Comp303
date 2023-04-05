#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/wait.h>
#include <stdbool.h>

#define READ 0
#define WRITE 1

//0 read 1 write
char* read_file(char* file_name) {     // https://stackoverflow.com/questions/3747086/reading-the-whole-text-file-into-a-char-array-in-c

    long lSize;
    char* buffer;
    FILE* file = fopen(file_name, "rb");

    if (!file) {
        fclose(file);
        perror("Occurred an error while reading file!\n");
        exit(1);
    }

    fseek( file , 0L , SEEK_END);
    lSize = ftell( file );
    rewind( file );

    /* Allocate memory for entire content */
    buffer = calloc( 1, lSize+1 );
    if( !buffer ) {
        fclose(file);
        fputs("Memory alloc fails!\n", stderr);
        exit(1);
    }
    /*char c =fgetc(file);
    while (c!=EOF){
        printf("%c",c);
        c=fgetc(file);
    }*/
    /* Copy the file into the buffer */
    if( 1!=fread( buffer , lSize, 1 , file) ) {
        //printf("%s\n",buffer);
        fclose(file);
        free(buffer);
        fputs("Entire read fails!\n", stderr);
        exit(1);
    }

    fclose(file);
    return buffer;
}

static int myCompare (const void * a, const void * b) {    //https://www.geeksforgeeks.org/c-program-sort-array-names-strings/
    return strcmp (*(const char **) a, *(const char **) b);
}

void sort(char *arr[], int n) {
    qsort (arr, n, sizeof (const char *), myCompare);
}

int psearch2(char* search_word, int input_file_count, char* input_filenames[], char* output_filename){
    int pfd[input_file_count][2];
    for (int i = 0; i < input_file_count; i++) {
        if (pipe(pfd[i]) != 0) {
            perror("Pipe Failed!");
            exit(1);
        }
        char *reading_file = input_filenames[i];
        pid_t pid =fork();
        if (pid < 0) {// Fork Error
            fprintf(stderr, "Fork Failed!\n");
            return 1;
        }
        else if (0 == pid) {// process of child
            close(pfd[i][READ]);
            char *file_content = read_file(reading_file);
            int line_number = 0;
            char *line = strtok(file_content, "\n");
            char *result=(char*) malloc(sizeof(char)*1000000);
            close(pfd[i][READ]);
            while (line != NULL) {
                char* new_message;
                new_message =(char *) malloc(sizeof(line)+100000);
                if(strstr(line,search_word)!=NULL){
                    sprintf(new_message, "%s, %d, %s\n", reading_file, line_number, line);
                    strcat(result, new_message);
                }
                line_number++;
                line = strtok(NULL, "\n");
                free(new_message);
            }
            write(pfd[i][WRITE], result, strlen(result)+1);
            close(pfd[i][WRITE]);
            exit(0);
        }
    }
    //Parent process

    char* final_output = (char*) malloc(sizeof(char)*1000000*input_file_count);
    for (int k = 0; k < input_file_count; k++) {
        wait(NULL);
        close(pfd[k][WRITE]);
        char* temp_output = (char*) malloc(sizeof(char)*1000000);
        if (read(pfd[k][READ], temp_output, 1000000) == -1) {
            perror("Read didn't return expected value!");
            exit(3);
        }
        strcat(final_output, temp_output);
        close(pfd[k][READ]);
    }

    FILE *final_output_file = fopen(output_filename , "w");
    if (final_output_file == NULL) {
        perror("Error occurred when writing file!\n");
        exit(0);
    }
    fprintf(final_output_file,"%s",final_output);
    fclose(final_output_file);
    return 1;
}
int main(int argc, char *argv[]) {
    char* search_word = argv[1];
    int count_of_inout_file = atoi(argv[2]);
    char* input_filenames[count_of_inout_file];
    char* output_filename = argv[count_of_inout_file+3];
    for (int i = 0; i < count_of_inout_file; i++) {
        input_filenames[i] = argv[i+3];
    }
    int n = sizeof(input_filenames)/sizeof(input_filenames[0]);
    sort(input_filenames, n);
    psearch2(search_word, count_of_inout_file, input_filenames, output_filename);
    return 0;
}