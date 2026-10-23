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

double parse_expression(const char **p, const char *start_line, int *error, int *error_position, int *is_float);
double parse_term(const char **p, const char *start_line, int *error, int *error_position, int *is_float);
double parse_power(const char **p, const char *start_line, int *error, int *error_position, int *is_float);
double parse_factor(const char **p, const char *start_line, int *error, int *error_position, int *is_float);

double parse_factor(const char **p, const char *start_line, int *error, int *error_position, int *is_float){
    while(isspace((unsigned char)**p)){
        (*p)++;
    }

    char *end;
    const char *start = *p;

    if(*start == '-'){
        *error = 1; 
        *error_position = (int)(start - start_line)+1;
        return 0;
    }

    if(**p == '('){
        (*p)++;
        double first = parse_expression(p, start_line, error, error_position, is_float);
        if(*error){
            return 0;
        }
        while(isspace((unsigned char)**p)){
            (*p)++;
        }

        if(**p != ')'){
            *error = 1;
            *error_position = (int)(*p - start_line)+1;
            return 0;
        }
        (*p)++;
        return first;
    }

    const char *number_start = *p;
    if(!isdigit((unsigned char)**p)){
        *error = 1;
        *error_position = (int)(start - start_line)+1;
        return 0; 
    }
    double number = strtod(*p, &end);
    int dot = 0;

    if(end == *p){
        *error = 1;
        *error_position = (int)(start - start_line)+1;
        return 0;
    }

    for(const char *q = number_start; q < end; q++){
        if(*q == 'e' || *q == 'E'){
            *error = 1;
            *error_position = (int)(q - start_line)+1;
            return 0;
        }else if(*q == '.'){
            dot = 1;
        }
    }

    if(dot && *(end-1) == '.'){
        *error = 1; 
        *error_position = (int)((end - 1) - start_line)+1;
        return 0;
    }

    *is_float = dot;
    *p = end;
    return number;
}

double parse_power(const char **p, const char *start_line, int *error, int *error_position, int *is_float){
    int base_is_float = 0; 
    double number = parse_factor(p, start_line, error, error_position, &base_is_float);

    if(*error){
        return 0;
    }

    while(isspace((unsigned char)**p)){
        (*p)++;
    }    

    if(**p == '*' && (*p)[1] == '*'){
        (*p) += 2;

        while(isspace((unsigned char)**p)){
            (*p)++;
        }

        const char *exponent_start = *p;
        int exponent_is_float = 0; 
        double exponent = parse_power(p, start_line, error, error_position, &exponent_is_float);

        if(*error){
            return 0;
        }

        if(!base_is_float && !exponent_is_float && exponent < 0){
            *error = 1;
            *error_position = (int)(exponent_start - start_line)+1;
            return 0;
        }else{
            number = pow(number, exponent);
        }

        if(base_is_float == 1 || exponent_is_float == 1){
            *is_float = 1;
        }else{
            *is_float = 0; 
        }
    }
    *is_float = base_is_float;
    return number;
}

double parse_term(const char **p, const char *start_line, int *error, int *error_position, int *is_float){
    int number_is_float = 0;
    double number = parse_power(p, start_line, error, error_position, &number_is_float);
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

        while(isspace((unsigned char)**p)){
            (*p)++;
        }

        const char *start = *p;
        int second_is_float = 0;
        double second = parse_power(p, start_line, error, error_position, &second_is_float);

        if(*error){
            return 0;
        }

        if(op == '*'){
            number = number * second;
            if(number_is_float == 1 || second_is_float == 1){
                *is_float = 1;
            }else{
                *is_float = 0;
            }
            number_is_float = *is_float;
        }else if(op == '/'){
            if(second == 0){
                *error = 1;
                *error_position = (int)(start-start_line)+1;
                return 0;
            }else if(number_is_float == 0 && second_is_float == 0){
                number = (long long)number / (long long)second; 
                *is_float = 0;
            }else{
                number = number / second;
                *is_float = 1;
            }
            number_is_float = *is_float;
        }
    }
    *is_float = number_is_float;
    return number;
}

double parse_expression(const char **p, const char *start_line, int *error, int *error_position, int *is_float){
    int number_is_float = 0;
    double number = parse_term(p, start_line, error, error_position, &number_is_float);
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

        int second_is_float = 0;
        double second = parse_term(p, start_line, error, error_position, &second_is_float);

        if(*error){
            return 0;
        }

        if(op == '+'){
            number += second;
            if(number_is_float == 1 || second_is_float == 1){
                *is_float = 1;
            }else{
                *is_float = 0;
            }
            number_is_float = *is_float;
        }else if(op == '-'){
            number -= second;
            if(number_is_float == 1 || second_is_float == 1){
                *is_float = 1;
            }else{
                *is_float = 0;
            }
            number_is_float = *is_float;
        }

    }
    *is_float = number_is_float;
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

    int result_types[1000];

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
        int result_is_float = 0;
        double result = parse_expression(&expression, start_line, &error, &error_position, &result_is_float);

        while(isspace((unsigned char)*expression)){
            expression++;
        }

        if(!error && *expression != '\0'){
            error =1;
            error_position = (int)(expression - start_line)+1;
        }

        if(error){
            first_error = 1;
            error_position_global = file_position + error_position;
            break;
        }else{
            results[result_count] = result;
            result_types[result_count] = result_is_float;
            result_count++;
        }
        file_position += strlen(line);
    }

    if(first_error){
        printf("ERROR:%ld\n", error_position_global);
    }else{
        for(int i = 0; i< result_count; i++){
            if(result_types[i] == 0){
                printf("%lld\n", (long long)results[i]);
            }else{
                printf("%.15g\n", results[i]);
            }
        }
    }

    fclose(fp);

    return 0;
}