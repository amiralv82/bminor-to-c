// ./Parser <Bminor File> <name.c>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>

#define MAX_LINE_LENGTH 1024
#define MAX_SYMBOL_TABLE_SIZE 100
#define MAX_IDENTIFIER_LENGTH 100

// --- Symbol Table Implementation ---
typedef enum {
    TYPE_INTEGER,
    TYPE_BOOLEAN,
    TYPE_CHARACTER,
    TYPE_STRING,
    TYPE_UNKNOWN // For function calls or expressions
} VariableType;

typedef struct {
    char name[MAX_IDENTIFIER_LENGTH];
    VariableType type;
} SymbolEntry;

SymbolEntry symbol_table[MAX_SYMBOL_TABLE_SIZE];
int symbol_count = 0;

void add_symbol(const char* name, VariableType type) {
    if (symbol_count < MAX_SYMBOL_TABLE_SIZE) {
        strncpy(symbol_table[symbol_count].name, name, MAX_IDENTIFIER_LENGTH - 1);
        symbol_table[symbol_count].name[MAX_IDENTIFIER_LENGTH - 1] = '\0';
        symbol_table[symbol_count].type = type;
        symbol_count++;
    }
}

VariableType get_variable_type(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return symbol_table[i].type;
        }
    }
    return TYPE_UNKNOWN; // Not found in symbol table
}
// --- End Symbol Table Implementation ---


char* translate_line(const char* line);
char* translate_function(const char* function_block);
char* replace_power_operator(const char* line);
char* replace_true_false(const char* line);

char* replace_true_false(const char* line) {
    char *result = (char*)malloc(MAX_LINE_LENGTH * 2); // Increased size
    if (!result) return NULL;
    result[0] = '\0';

    const char *p = line;
    while (*p) {
        if (strncmp(p, "True", 4) == 0) {
            strcat(result, "true");
            p += 4;
        } else if (strncmp(p, "False", 5) == 0) {
            strcat(result, "false");
            p += 5;
        } else {
            strncat(result, p, 1);
            p++;
        }
    }
    return result;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.bminor output.c\n", argv[0]);
        return 1;
    }

    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror("Error opening input file");
        return 1;
    }

    FILE *output = fopen(argv[2], "w");
    if (!output) {
        perror("Error opening output file");
        fclose(input);
        return 1;
    }

    // نوشتن هدرهای استاندارد C
    fprintf(output,
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "#include <stdbool.h>\n"
        "#include <math.h>\n"
        "#include <string.h>\n\n"
    );

    char line[MAX_LINE_LENGTH];
    char full_input_buffer[50000] = ""; // برای خواندن کل فایل
    char temp_line[MAX_LINE_LENGTH];

    // Read the entire file into a buffer to correctly handle multi-line comments and functions
    while (fgets(temp_line, sizeof(temp_line), input)) {
        strcat(full_input_buffer, temp_line);
    }
    fclose(input);

    char *current_pos = full_input_buffer;

    while (*current_pos != '\0') {
        // Skip leading whitespace and comments
        while (isspace(*current_pos)) {
            current_pos++;
        }
        if (*current_pos == '\0') break; // End of buffer

        // Handle single-line comments
        if (strncmp(current_pos, "//", 2) == 0) {
            char *newline = strchr(current_pos, '\n');
            if (newline) {
                fprintf(output, "%.*s\n", (int)(newline - current_pos), current_pos);
                current_pos = newline + 1;
            } else {
                fprintf(output, "%s\n", current_pos);
                current_pos += strlen(current_pos);
            }
            continue;
        }

        // Handle multi-line comments
        if (strncmp(current_pos, "/*", 2) == 0) {
            char *end_comment = strstr(current_pos, "*/");
            if (end_comment) {
                fprintf(output, "%.*s*/\n", (int)(end_comment - current_pos), current_pos);
                current_pos = end_comment + 2;
            } else { // Comment not closed in this file
                fprintf(output, "%s\n", current_pos);
                current_pos += strlen(current_pos);
            }
            continue;
        }

        // Check for function definitions
        char temp_line_for_func_check[MAX_LINE_LENGTH];
        strncpy(temp_line_for_func_check, current_pos, sizeof(temp_line_for_func_check) - 1);
        temp_line_for_func_check[sizeof(temp_line_for_func_check) - 1] = '\0';
        char *newline_for_func = strchr(temp_line_for_func_check, '\n');
        if (newline_for_func) *newline_for_func = '\0'; // Null-terminate for strstr check

        if (strstr(temp_line_for_func_check, ": function") != NULL) {
            char function_block[10000] = "";
            char *block_start = current_pos;
            char *block_end = strstr(current_pos, "};");

            if (block_end) {
                strncpy(function_block, block_start, block_end - block_start + 2);
                function_block[block_end - block_start + 2] = '\0';
                current_pos = block_end + 2;
            } else { // Function not closed properly, read till end or max buffer
                strncpy(function_block, block_start, sizeof(function_block) - 1);
                function_block[sizeof(function_block) - 1] = '\0';
                current_pos += strlen(function_block);
            }

            char *translated = translate_function(function_block);
            if (translated != NULL) {
                fprintf(output, "%s\n", translated);
                free(translated);
            }
        } else {
            // Process a regular line
            char *newline = strchr(current_pos, '\n');
            if (newline) {
                strncpy(line, current_pos, newline - current_pos);
                line[newline - current_pos] = '\0';
                current_pos = newline + 1;
            } else {
                strcpy(line, current_pos);
                current_pos += strlen(current_pos);
            }

            char *translated = translate_line(line);
            if (translated != NULL) {
                fprintf(output, "%s\n", translated);
                free(translated);
            }
        }
    }

    fclose(output);
    return 0;
}

char* translate_line(const char* line) {
    char *translated = (char*)malloc(MAX_LINE_LENGTH * 4); // Increased size for printf formatting
    if (!translated) return NULL;
    strcpy(translated, "");

    const char *p = line;
    while (isspace(*p)) p++;

    // Single-line comment
    if (strncmp(p, "//", 2) == 0) {
        strcpy(translated, p);
        return translated;
    }

    // Multi-line comment start (handled in main loop, but here for completeness)
    if (strncmp(p, "/*", 2) == 0) {
        strcpy(translated, p);
        return translated;
    }

    // Variable declarations
    char name[MAX_IDENTIFIER_LENGTH];
    char value[MAX_LINE_LENGTH];

    if (strstr(p, ": integer") != NULL) {
        sscanf(p, "%[^:]: integer = %[^;];", name, value);
        sprintf(translated, "int %s = %s;", name, value);
        add_symbol(name, TYPE_INTEGER);
        return translated;
    }

    if (strstr(p, ": boolean") != NULL) {
        sscanf(p, "%[^:]: boolean = %[^;];", name, value);
        char *bool_value = replace_true_false(value); // Ensure "True" -> "true"
        sprintf(translated, "bool %s = %s;", name, bool_value);
        free(bool_value);
        add_symbol(name, TYPE_BOOLEAN);
        return translated;
    }

    if (strstr(p, ": character") != NULL) {
        sscanf(p, "%[^:]: character = %[^;];", name, value);
        sprintf(translated, "char %s = %s;", name, value);
        add_symbol(name, TYPE_CHARACTER);
        return translated;
    }

    if (strstr(p, ": string") != NULL) {
        sscanf(p, "%[^:]: string = %[^;];", name, value);
        sprintf(translated, "char %s[] = %s;", name, value);
        add_symbol(name, TYPE_STRING);
        return translated;
    }

    if (strstr(p, ": array") != NULL) {
        char size[10];
        // Note: sscanf format for array value needs to be careful about spaces
        sscanf(p, "%[^:]: array[%[^]]] of integer = {%[^}]", name, size, value);
        sprintf(translated, "int %s[%s] = { %s };", name, size, value);
        // For simplicity, array elements are assumed integer. Not adding specific array type to symbol table.
        return translated;
    }
    // --- Reworked print statement translation ---
if (strstr(p, "print") == p) {
    const char* q = p + strlen("print");
    while (isspace(*q)) q++; // Skip spaces after "print"

    char format_str[MAX_LINE_LENGTH * 2] = "";
    char args_str[MAX_LINE_LENGTH * 2] = "";
    bool first_arg_in_printf_call = true; // Tracks arguments for the printf call's comma separation

    // Start constructing the format string for printf
    strcat(format_str, "\""); // Start the format string with an opening quote

    const char* current_print_arg = q;
    while (*current_print_arg && *current_print_arg != ';') {
        if (*current_print_arg == '"') { // Handle string literals
            current_print_arg++; // Skip opening quote of the original Bminor string
            while (*current_print_arg && *current_print_arg != '"') {
                if (*current_print_arg == '\\') {
                    // Copy escape sequence directly, e.g., \n, \t, \\, \"
                    strncat(format_str, current_print_arg, 2);
                    current_print_arg += 2;
                } else if (*current_print_arg == '\"') {
                    // Escape double quotes within the format string
                    strcat(format_str, "\\\"");
                    current_print_arg++;
                } else {
                    strncat(format_str, current_print_arg, 1);
                    current_print_arg++;
                }
            }
            if (*current_print_arg == '"') current_print_arg++; // Skip closing quote
        } else if (*current_print_arg == ',') {
            current_print_arg++;
            while (isspace(*current_print_arg)) current_print_arg++;
        } else { // Handle variables or function calls
            char token_buf[MAX_IDENTIFIER_LENGTH];
            int token_idx = 0;
            while (*current_print_arg && *current_print_arg != ',' && *current_print_arg != ';' && !isspace(*current_print_arg) && *current_print_arg != '"') {
                token_buf[token_idx++] = *current_print_arg;
                current_print_arg++;
            }
            token_buf[token_idx] = '\0';

            if (strlen(token_buf) > 0) {
                VariableType type = get_variable_type(token_buf);
                char current_format_specifier[10];

                if (strstr(token_buf, "(") && strstr(token_buf, ")")) { // Likely a function call
                    strcpy(current_format_specifier, "%d"); // Assume int return for simplicity in print
                } else {
                    switch (type) {
                        case TYPE_INTEGER:
                            strcpy(current_format_specifier, "%d");
                            break;
                        case TYPE_BOOLEAN:
                            strcpy(current_format_specifier, "%d"); // boolean in C is int for printf
                            break;
                        case TYPE_CHARACTER:
                            strcpy(current_format_specifier, "%c");
                            break;
                        case TYPE_STRING:
                            strcpy(current_format_specifier, "%s");
                            break;
                        case TYPE_UNKNOWN:
                        default:
                            // Try to infer from content, or default to %d
                            if (isdigit(token_buf[0]) || (token_buf[0] == '-' && isdigit(token_buf[1]))) {
                                strcpy(current_format_specifier, "%d"); // Numeric literal
                            } else if (token_buf[0] == '\'') {
                                strcpy(current_format_specifier, "%c"); // Character literal
                            } else if (token_buf[0] == '"') {
                                strcpy(current_format_specifier, "%s"); // String literal
                            } else {
                                strcpy(current_format_specifier, "%d"); // Fallback for unknown variables/expressions
                            }
                            break;
                    }
                }

                strcat(format_str, current_format_specifier); // Add the format specifier to the format string

                if (!first_arg_in_printf_call) {
                    strcat(args_str, ", ");
                }
                strcat(args_str, token_buf);
                first_arg_in_printf_call = false;
            }
        }
        while (isspace(*current_print_arg) && *current_print_arg != ';') current_print_arg++;
    }

    // Always add a newline at the end of the format string if not already present
    // This assumes Bminor `print` inherently adds a newline, similar to Python's print
    if (strstr(format_str, "\\n\"") == NULL) { // Check if "\n" followed by " exists
        strcat(format_str, "\\n");
    }
    strcat(format_str, "\""); // Close the format string

    if (strlen(args_str) > 0) {
        sprintf(translated, "printf(%s, %s);", format_str, args_str);
    } else {
        sprintf(translated, "printf(%s);", format_str);
    }

    return translated;
}
// --- End reworked print statement translation ---


    // return
    if (strstr(p, "return") == p) {
        char return_val[MAX_LINE_LENGTH];
        sscanf(p, "return %[^;];", return_val);
        sprintf(translated, "return %s;", return_val);
        return translated;
    }

    // while loop
    if (strstr(p, "while") == p) {
        char condition[MAX_LINE_LENGTH];
        // Expecting "while (condition) {"
        char* brace_open = strchr(p, '{');
        if (brace_open) {
            strncpy(condition, p + strlen("while ("), brace_open - (p + strlen("while ( ")) - 1);
            condition[brace_open - (p + strlen("while ( ")) - 1] = '\0';
            sprintf(translated, "while (%s) {", condition);
        } else {
             // If brace is on next line or not found, try to parse condition up to newline or end of line
            char* end_cond = strchr(p, ')');
            if (end_cond) {
                strncpy(condition, p + strlen("while ("), end_cond - (p + strlen("while ( ")));
                condition[end_cond - (p + strlen("while ( "))] = '\0';
                sprintf(translated, "while (%s)", condition);
            } else {
                strcpy(translated, p); // Fallback
            }
        }
        return translated;
    }
    
    // if statement
    if (strstr(p, "if") == p) {
        char condition[MAX_LINE_LENGTH];
        char* brace_open = strchr(p, '{');
        if (brace_open) {
            strncpy(condition, p + strlen("if ("), brace_open - (p + strlen("if ( ")) - 1);
            condition[brace_open - (p + strlen("if ( ")) - 1] = '\0';
            sprintf(translated, "if (%s) {", condition);
        } else {
            char* end_cond = strchr(p, ')');
            if (end_cond) {
                strncpy(condition, p + strlen("if ("), end_cond - (p + strlen("if ( ")));
                condition[end_cond - (p + strlen("if ( "))] = '\0';
                sprintf(translated, "if (%s)", condition);
            } else {
                strcpy(translated, p); // Fallback
            }
        }
        return translated;
    }

    // Function end (};) - handled in translate_function for the block context
    if (strstr(p, "};") != NULL) {
        strcpy(translated, "}");
        return translated;
    }
    
    // Assignment statements and general expressions
    char* equals_sign = strchr(p, '=');
    char* semicolon = strchr(p, ';');

    if (equals_sign && semicolon && equals_sign < semicolon) {
        char var_name[MAX_IDENTIFIER_LENGTH];
        strncpy(var_name, p, equals_sign - p);
        var_name[equals_sign - p] = '\0';
        // Trim whitespace from var_name
        char *end = var_name + strlen(var_name) - 1;
        while(end > var_name && isspace((unsigned char)*end)) end--;
        *(end+1) = '\0';

        char expression[MAX_LINE_LENGTH];
        strncpy(expression, equals_sign + 1, semicolon - (equals_sign + 1));
        expression[semicolon - (equals_sign + 1)] = '\0';
        // Trim whitespace from expression
        char *exp_end = expression + strlen(expression) - 1;
        char *exp_start = expression;
        while(isspace((unsigned char)*exp_start)) exp_start++;
        while(exp_end > exp_start && isspace((unsigned char)*exp_end)) exp_end--;
        *(exp_end+1) = '\0';

        char *power_fixed_expr = replace_power_operator(exp_start);
        char *true_false_fixed_expr = replace_true_false(power_fixed_expr);
        free(power_fixed_expr);

        sprintf(translated, "%s = %s;", var_name, true_false_fixed_expr);
        free(true_false_fixed_expr);
        return translated;
    }

    // Default: return the line as is if no specific translation rule applies
    // This is a simplified approach; a real compiler would report an error for unhandled syntax
    strcpy(translated, p);
    return translated;
}

// تبدیل بلوک کامل تابع
char* translate_function(const char* function_block) {
    char *translated = (char*)malloc(20000); // Increased size
    if (!translated) return NULL;
    strcpy(translated, "");

    // Find '{' and '};'
    const char* brace_pos = strchr(function_block, '{');
    const char* end_pos = strstr(function_block, "};");
    if (!brace_pos || !end_pos) {
        free(translated);
        return strdup(function_block); // Return as is if format is bad
    }

    // Extract function header (before '{')
    char header[600];
    strncpy(header, function_block, brace_pos - function_block);
    header[brace_pos - function_block] = '\0';

    // Function name, return type, parameters
    char name[MAX_IDENTIFIER_LENGTH], rettype[50], params[500];
    int ret = sscanf(header, "%[^:]: function %s (%[^)]) =", name, rettype, params);
    if (ret < 2) {
        free(translated);
        return NULL;
    }

    // Convert return type
    char typeC[50];
    if (strcmp(rettype, "integer") == 0) strcpy(typeC, "int");
    else if (strcmp(rettype, "boolean") == 0) strcpy(typeC, "bool");
    else if (strcmp(rettype, "character") == 0) strcpy(typeC, "char");
    else if (strcmp(rettype, "string") == 0) strcpy(typeC, "char*");
    else strcpy(typeC, "void");

    // Convert parameters
    char params_c[500] = "";
    if (ret == 3) {
        char params_copy[500];
        strcpy(params_copy, params);

        char *param_token = strtok(params_copy, ",");
        bool first_param = true;
        while (param_token != NULL) {
            while (isspace(*param_token)) param_token++; // Trim leading space

            char param_name[MAX_IDENTIFIER_LENGTH], param_type[50];
            if (sscanf(param_token, "%[^:]: %s", param_name, param_type) == 2) {
                char param_type_c[50];
                if (strcmp(param_type, "integer") == 0) strcpy(param_type_c, "int");
                else if (strcmp(param_type, "boolean") == 0) strcpy(param_type_c, "bool");
                else if (strcmp(param_type, "character") == 0) strcpy(param_type_c, "char");
                else if (strcmp(param_type, "string") == 0) strcpy(param_type_c, "char*");
                else strcpy(param_type_c, "void"); // Fallback

                if (!first_param) strcat(params_c, ", ");
                strcat(params_c, param_type_c);
                strcat(params_c, " ");
                strcat(params_c, param_name);
                first_param = false;

                // Add parameters to symbol table for local scope (simplified)
                if (strcmp(param_type, "integer") == 0) add_symbol(param_name, TYPE_INTEGER);
                else if (strcmp(param_type, "boolean") == 0) add_symbol(param_name, TYPE_BOOLEAN);
                else if (strcmp(param_type, "character") == 0) add_symbol(param_name, TYPE_CHARACTER);
                else if (strcmp(param_type, "string") == 0) add_symbol(param_name, TYPE_STRING);
            }
            param_token = strtok(NULL, ",");
        }
    }

    // Extract function body between '{' and '};'
    int body_len = (int)(end_pos - brace_pos - 1);
    char body[9000];
    strncpy(body, brace_pos + 1, body_len);
    body[body_len] = '\0';

    // Start function translation
    sprintf(translated, "%s %s(%s) {\n", typeC, name, params_c);

    // Process body line by line
    char *body_copy = strdup(body);
    char *line_start = body_copy;
    char *line_end;

    while (line_start && *line_start != '\0') {
        line_end = strchr(line_start, '\n');
        char current_line[MAX_LINE_LENGTH];
        if (line_end) {
            strncpy(current_line, line_start, line_end - line_start);
            current_line[line_end - line_start] = '\0';
            line_start = line_end + 1;
        } else {
            strcpy(current_line, line_start);
            line_start = NULL; // Mark end of loop
        }

        // Trim leading whitespace for processing
        char *trimmed_line = current_line;
        while (isspace(*trimmed_line)) trimmed_line++;

        if (strlen(trimmed_line) > 0) {
            // Apply transformations
            char *line_tf = replace_true_false(trimmed_line);
            char *power_fixed = replace_power_operator(line_tf);
            free(line_tf);

            char *line_translated = translate_line(power_fixed);
            if (line_translated != NULL) {
                // Add indentation for function body
                strcat(translated, "    ");
                strcat(translated, line_translated);
                strcat(translated, "\n");
                free(line_translated);
            }
            free(power_fixed);
        }
    }

    strcat(translated, "}\n");
    free(body_copy);
    return translated;
}

char* replace_power_operator(const char* line) {
    char *result = (char*)malloc(MAX_LINE_LENGTH * 4); // Increased size
    if (!result) return NULL;
    result[0] = '\0';

    const char *p = line;
    while (*p) {
        const char *caret = strchr(p, '^');
        if (!caret) {
            strcat(result, p);
            break;
        }

        // Find left operand (number or variable)
        const char *left_end = caret - 1;
        while (left_end >= p && isspace(*left_end)) left_end--;
        const char *left_start = left_end;
        // Consider parentheses for expressions like (a + b)^2
        int paren_count = 0;
        if (*left_start == ')') {
            paren_count++;
            left_start--;
            while(left_start >= p && paren_count > 0) {
                if (*left_start == '(') paren_count--;
                else if (*left_start == ')') paren_count++;
                left_start--;
            }
            if(left_start < p) left_start = p; // Should not go before beginning of string
            left_start++; // Move to the char after '(' or to the start of the operand
        } else {
            while (left_start >= p && (isalnum(*(left_start)) || *(left_start) == '_')) {
                left_start--;
            }
            left_start++; // Move to the start of the operand
        }


        // Find right operand (number or variable)
        const char *right_start = caret + 1;
        while (*right_start && isspace(*right_start)) right_start++;
        const char *right_end = right_start;
        // Consider parentheses for expressions like 2^(a + b)
        paren_count = 0;
        if (*right_end == '(') {
            paren_count++;
            right_end++;
            while(*right_end && paren_count > 0) {
                if (*right_end == '(') paren_count++;
                else if (*right_end == ')') paren_count--;
                right_end++;
            }
        } else {
            while (*right_end && (isalnum(*right_end) || *right_end == '_')) {
                right_end++;
            }
        }

        // Extract a and b
        char left[128] = "", right[128] = "";
        strncpy(left, left_start, left_end - left_start + 1);
        left[left_end - left_start + 1] = '\0';
 
        strncpy(right, right_start, right_end - right_start);
        right[right_end - right_start] = '\0';
        
        // Copy part before 'a'
        strncat(result, p, left_start - p);
        strcat(result, "(int)pow(");
        strcat(result, left);
        strcat(result, ", ");
        strcat(result, right);
        strcat(result, ")");

        p = right_end;
    }

    return result;
}