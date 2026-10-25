//Name: Mihails, Lastname: Sazonovs, ID: 251RDC077
//Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// AI tool: ChatGPT (OpenAI).
// Conversation history: https://chatgpt.com/share/6ade87ff-edc8-83eb-a065-1f2cd615c499 
// Note: The linked document contains translated summaries of the original discussion, not a verbatim transcript.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h> // Provides LLONG_MIN for checking integer division overflow.

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
        // Positions are calculated relative to the original line so errors point
        // to the exact character without copying the expression into another buffer.
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
    // If no valid factor starts here, report the current position because
    // this is where the parser expected an operand.
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
        // Scientific notation is not allowed, so reject e/E at the character
        // where the unsupported number format begins.
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

// Exponentiation is handled at a deeper grammar level so it binds more
// tightly than multiplication, division, addition, and subtraction.
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
        // Save the position of the exponentiation operator for error reporting.
        const char *operator_position = *p;
        (*p) += 2;

        while(isspace((unsigned char)**p)){
            (*p)++;
        }

        const char *exponent_start = *p;
        int exponent_is_float = 0; 
        // Parsing the exponent with parse_power() makes exponentiation
        // right-associative: 2**3**2 is interpreted as 2**(3**2).
        double exponent = parse_power(p, start_line, error, error_position, &exponent_is_float);

        if(*error){
            return 0;
        }

        // The assignment permits negative exponents for floating-point arithmetic,
        // but rejects them when both the base and exponent are integers.
        if(!base_is_float && !exponent_is_float && exponent < 0){
            *error = 1;
            *error_position = (int)(exponent_start - start_line)+1;
            return 0;
        }else{
            number = pow(number, exponent);
            
            // Check whether exponentiation produced an infinite or invalid result.
            if(isinf(number) || isnan(number)){
                *error = 1;
                // Report the error at the exponentiation operator.
                *error_position = (int)(operator_position - start_line) +1;
                return 0;
            }
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

// Multiplication and division are handled below expression parsing,
// so they are evaluated before addition and subtraction.
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
        // Save the operator position to report arithmetic errors accurately.
        const char *operator_position = *p;
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

            // Check whether multiplication produced an infinite or invalid result.
            if(isinf(number) || isnan(number)){
                *error = 1; 
                // Report the error at the multiplication operator.
                *error_position = (int)(operator_position - start_line) +1;
                return 0;
            }

            if(number_is_float == 1 || second_is_float == 1){
                *is_float = 1;
            }else{
                *is_float = 0;
            }
            number_is_float = *is_float;
        }else if(op == '/'){
            // The error points to the beginning of the right operand because that
            // operand makes the division invalid when its value is zero.
            if(second == 0){
                *error = 1;
                *error_position = (int)(start-start_line)+1;
                return 0;
            }
            // Use integer division when both operands are integers; otherwise,
            // floating-point division preserves the fractional part.
            else if(number_is_float == 0 && second_is_float == 0){
                
                // Prevent integer division overflow when LLONG_MIN is divided by -1.
                if ((long long)number == LLONG_MIN && (long long)second == -1) {
                    *error = 1;
                    *error_position = (int)(operator_position - start_line) + 1;
                    return 0;
                }

                number = (long long)number / (long long)second; 
                *is_float = 0;
            }else{
                number = number / second;

                // Check whether floating-point division produced an infinite or invalid result.
                if(isinf(number) || isnan(number)){
                *error = 1; 
                // Report the error at the division operator.
                *error_position = (int)(operator_position - start_line) +1;
                return 0;
                }   

                *is_float = 1;
            }
            number_is_float = *is_float;
        }
    }
    *is_float = number_is_float;
    return number;
}

// Addition and subtraction are handled at this level because they have
// lower precedence than multiplication, division, and exponentiation.
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
        // Save the operator position for reporting arithmetic errors.
        const char *operator_position = *p;
        char op = **p;
        (*p)++;

        int second_is_float = 0;
        double second = parse_term(p, start_line, error, error_position, &second_is_float);

        if(*error){
            return 0;
        }

        if(op == '+'){
            number += second;

            // Check whether addition produced an infinite or invalid result.
            if (isinf(number) || isnan(number)) {
                *error = 1;
                // Report the error at the addition operator.
                *error_position = (int)(operator_position - start_line) + 1;
                return 0;
            }

            if(number_is_float == 1 || second_is_float == 1){
                *is_float = 1;
            }else{
                *is_float = 0;
            }
            number_is_float = *is_float;
        }else if(op == '-'){
            number -= second;

            // Check whether subtraction produced an infinite or invalid result.
            if (isinf(number) || isnan(number)) {
                *error = 1;
                // Report the error at the addition operator.
                *error_position = (int)(operator_position - start_line) + 1;
                return 0;
            }

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

    // Read the file incrementally instead of loading the entire file into memory.
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
        // The parser advances pointers through the existing line buffer,
        // avoiding repeated copies of expression substrings.
        double result = parse_expression(&expression, start_line, &error, &error_position, &result_is_float);

        while(isspace((unsigned char)*expression)){
            expression++;
        }

        // Any remaining non-whitespace character means the expression was not
        // fully consumed; report the first unexpected character.
        if(!error && *expression != '\0'){
            error =1;
            error_position = (int)(expression - start_line)+1;
        }

        if(error){
            first_error = 1;
            // Adding the number of characters in previous lines converts a line-relative
            // position into a 1-based position in the entire input file.  
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