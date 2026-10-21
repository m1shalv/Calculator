//Name: Mihails, Lastname: Sazonovs, ID: 251RDC077
//Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

double parse_expression(const char **p, const char *start_line, int *error, int *error_position);
double parse_term(const char **p, const char *start_line, int *error, int *error_position);
double parse_factor(const char **p, const char *start_line, int *error, int *error_position);

double parse_factor(const char **p, const char *start_line, int *error, int *error_position){
    while(isspace((unsigned char)**p)){
        (*p)++;
    }

    char *end;
    const char *start = *p;
    double number = strtod(*p, &end);

    if(end == *p){
        *error = 1;
        *error_position = (int)(start - start_line)+1;
        return 0;
    }

    *p = end;
    return number;
}

double parse_term(const char **p, const char *start_line, int *error, int *error_position){
    double number = parse_factor(p, start_line, error, error_position);
    if(*error){
        return 0;
    }

    while(1){
        while(isspace((unsigned char)**p)){
            (*p)++;
        }
        if(**p != '*' && **p != '/'){
            break;
        }
        char op = **p;
        (*p)++;

        const char *start = *p;

        double second = parse_factor(p, start_line, error, error_position);

        if(*error){
            return 0;
        }

        if(op == '*'){
            number = number * second;
        }else if(op == '/'){
            if(second == 0){
                *error = 1;
                *error_position = (int)(start-start_line)+1;
                return 0;
            }else{
                number = number / second; 
            }
        }
    }

    return number;
}

double parse_expression(const char **p, const char *start_line, int *error, int *error_position){
    double number = parse_term(p, start_line, error, error_position);
    if(*error){
        return 0;
    }

    while(1){
        while(isspace((unsigned char)**p)){
            (*p)++;
        }
        if(**p != '+' && **p != '-'){
            break;
        }
        char op = **p;
        (*p)++;

        double second = parse_term(p, start_line ,error, error_position);

        if(*error){
            return 0;
        }

        if(op == '+'){
            number += second;
        }else if(op == '-'){
            number -= second;
        }

    }
    return number;
}

long long calculate (const char *line, int *error, int *error_position){
    *error = 0; 
    char *end;
    char *newline = strchr(line, '\n');

    long long number = strtoll(line, &end, 10);
    
    if(line == end){
        *error_position = (int)(end - line) + 1;
        *error = 1;
        return 0;
    }
    
    while(*end != '\0'){
        while(isspace((unsigned char)*end)){
            end++;
        }
        
        if(*end == '\0'){
            break;
        }
        
        if(*end == '+'){
            end++;
            
            while(isspace((unsigned char)*end)){
            end++;
            }
            
            if(*end == '\0' || *end == '\n'){
                if(newline != NULL){
                    *error_position = (int)(newline - line) + 1;
                }else{
                    *error_position = (int)strlen(line) + 1;
                }
                *error = 1; 
                return 0;
            }
            
            char *start = end;
            long long second = strtoll(end, &end, 10);
            
            if(start == end){
                *error_position  = (int)(start-line) + 1;
                *error = 1; 
                return 0;
            }else{
                number +=second; 
            }

        }else if(*end == '-'){
            end++;
            
            while(isspace((unsigned char)*end)){
            end++;
            }
            
            if(*end == '\0' || *end == '\n'){
                if(newline != NULL){
                    *error_position = (int)(newline - line) + 1;
                }else{
                    *error_position = (int)strlen(line) + 1;
                }
                *error = 1;
                return 0;
            }
            
            char *start = end;
            
            long long third = strtoll(end, &end, 10);
            if(start == end){
                *error_position  = (int)(start-line) + 1;
                *error = 1;
                return 0;
            }else{
                number -= third;
            }
        } else if(*end != '-' && *end !='+'){
            *error_position  = (int)(end-line) + 1;
            *error = 1;
            return 0;
        }
        
    }
    return number;
}

int main(int argc, char*argv[]){
    (void)argc;
    (void)argv;

    if (argc < 2) {
        fprintf(stderr, "Usage: ./calc input.txt\n");
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");

    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open input file\n");
        return 1;
    }

    char line[1024];
    double results[1000];
    int result_count = 0;

    int first_error = 0;
    long error_position_global = 0;

    long file_position = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *p = line;
        while(isspace((unsigned char)*p)){
            p++;
        }

        if(*p == '\0' || *p == '\n'){
            file_position += strlen(line);
            continue;
        }

        if(*p == '#'){
            file_position += strlen(line);
            continue;
        }

        int error = 0;
        int error_position = 0;
        
        const char *expression = line;
        const char *start_line = expression;
        double result = parse_expression(&expression, start_line, &error, &error_position);
        if(error){
            first_error = 1;
            error_position_global = file_position + error_position;
            break;
        }else{
            results[result_count] = result;
            result_count++;
        }
        file_position += strlen(line);
    }

    if(first_error){
        printf("ERROR:%ld\n", error_position_global);
    }else{
        for(int i = 0; i< result_count; i++){
            printf("%g\n", results[i]);
        }
    }

    fclose(fp);

    return 0;
}