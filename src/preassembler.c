#include "../include/preassembler.h"

<<<<<<< HEAD
int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray) {
    FILE *input_fd = fopen(input_path, "r");
    if (!input_fd) return STATUS_CATASTROPHIC;
=======
int ParseMacros(char *file_path, Macro macros[MAX_MACROS], size_t *macro_count) {
    if (
        file_path == NULL
        || macros == NULL
        || macro_count == NULL) return STATUS_CATASTROPHIC;
    
    

    FILE *file_fd = fopen(file_path, "r");
    if (file_fd == NULL) return STATUS_CATASTROPHIC;

    Macro *curr = malloc(sizeof(Macro));
    if (curr == NULL) {
        fclose(file_fd);
        return STATUS_CATASTROPHIC;
    }

    while (*macro_count < MAX_MACROS) {
        memset(curr, 0, sizeof(*curr));

        //Checks if the name of the macro is a name of an existing command
        if (curr == NULL) {
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }

        int status = AddMacro(file_fd, curr);

        if (status == STATUS_CATASTROPHIC) {
            free(curr);
            fclose(file_fd);
            return STATUS_CATASTROPHIC;
        }

        
        

        if (status == STATUS_NO_RESULT) break; // No more Macros


        // Macro already exists
        if (FindMacro(curr->name, macros, macro_count) != NULL) {
            CleanUpMacro(curr);
            fclose(file_fd);
            return STATUS_NO_RESULT;
        }

        if (IsCommandName(curr->name)) {
            CleanUpMacro(curr);
            fclose(file_fd);
            return STATUS_NO_RESULT;
        }

        macros[*macro_count] = *curr; // Copy struct
        (*macro_count)++;
    }

    fclose(file_fd);
    free(curr);
    return 0;
}


int ExpandMacros(const char *input_path, const char *output_path) {
    Macro macros[MAX_MACROS] = {0};
    size_t macro_count = 0;
    
    int status = ParseMacros((char *)input_path, macros, &macro_count);
    if (status != 0) {
        return status;
    }
    
    FILE *input_fd = fopen(input_path, "r");
    if (!input_fd) {
        return STATUS_CATASTROPHIC;
    }
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
    
    FILE *output_fd = fopen(output_path, "w");
    if (!output_fd) {
        fclose(input_fd);
        return STATUS_CATASTROPHIC;
    }
    
    char line[MAX_LINE_LENGTH] = {0};
<<<<<<< HEAD
    while (fgets(line, MAX_LINE_LENGTH, input_fd)) {
        /* Skip macro definition blocks */
        if (strncmp(line, MACRO_START, strlen(MACRO_START)) == 0) {
            // Skip lines until the macro end marker is found
            while (fgets(line, MAX_LINE_LENGTH, input_fd)) {
                if (strncmp(line, MACRO_END, strlen(MACRO_END)) == 0) {
                    break;
                }
            }
            continue; // Skip this macro definition block entirely
        }
        
        // Check if the line is a macro call.
=======
    Macro *curr = NULL;
    
    while (fgets(line, MAX_LINE_LENGTH, input_fd)) {
        if (line[0] == '\n' || line[0] == '\r') {
            fprintf(output_fd, "%s", line);
            continue;
        }
        
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        size_t start = strspn(line, " \t");
        size_t name_length = 0;
        while (line[start + name_length] != '\0' && !isspace(line[start + name_length])) {
            name_length++;
        }
<<<<<<< HEAD
=======
        
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
        char *macro_name = strndup(line + start, name_length);
        if (!macro_name) {
            fclose(input_fd);
            fclose(output_fd);
            return STATUS_CATASTROPHIC;
        }
        
<<<<<<< HEAD
        Macro *macro = FindMacroDynamic(macro_name, mArray);
        free(macro_name);
        
        if (macro != NULL) {
            // Expand the macro body by writing its lines to output.
            for (size_t i = 0; i < macro->line_count; i++) {
                if (fprintf(output_fd, "%s", macro->body[i]) < 0) {
=======
        curr = FindMacro(macro_name, macros, &macro_count);
        free(macro_name);
        
        if (curr != NULL) {
            for (size_t i = 0; i < curr->line_count; i++) {
                if (fprintf(output_fd, "%s", curr->body[i]) < 0) {
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
                    fclose(input_fd);
                    fclose(output_fd);
                    return STATUS_CATASTROPHIC;
                }
            }
        } else {
<<<<<<< HEAD
            // Not a macro call; write the line as-is.
=======
>>>>>>> fd3497d43053c2f28805d3c8b5d843c434e1719f
            if (fprintf(output_fd, "%s", line) < 0) {
                fclose(input_fd);
                fclose(output_fd);
                return STATUS_CATASTROPHIC;
            }
        }
    }
    
    fclose(input_fd);
    fclose(output_fd);
    return 0;
}


// Copies a macro, frees original macro
Macro *DeepCopyMacro(Macro *src) {
    if (src == NULL) return NULL;
    Macro *dst = malloc(sizeof(Macro));
    if (dst == NULL) return NULL;
    memset(dst, 0, sizeof(*dst));

    strncpy(dst->name, src->name, MAX_MACRO_NAME - 1);
    dst->name[MAX_MACRO_NAME - 1] = '\0';
    dst->line_count = src->line_count;

    for (size_t i = 0; i < src->line_count; i++) {
        strncpy(dst->body[i], src->body[i], MAX_LINE_LENGTH - 1);
        dst->body[i][MAX_LINE_LENGTH - 1] = '\0';
    }

    CleanUpMacro(src);
    return dst;
}

