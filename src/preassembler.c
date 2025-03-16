#include "../include/preassembler.h"

int ExpandMacros(const char *input_path, const char *output_path, MacroArray *mArray) {
    FILE *input_fd = fopen(input_path, "r");
    if (!input_fd) return STATUS_CATASTROPHIC;
    
    FILE *output_fd = fopen(output_path, "w");
    if (!output_fd) {
        fclose(input_fd);
        return STATUS_CATASTROPHIC;
    }
    
    char line[MAX_LINE_LENGTH] = {0};
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
        size_t start = strspn(line, " \t");
        size_t name_length = 0;
        while (line[start + name_length] != '\0' && !isspace(line[start + name_length])) {
            name_length++;
        }
        char *macro_name = strndup(line + start, name_length);
        if (!macro_name) {
            fclose(input_fd);
            fclose(output_fd);
            return STATUS_CATASTROPHIC;
        }
        
        Macro *macro = FindMacroDynamic(macro_name, mArray);
        free(macro_name);
        
        if (macro != NULL) {
            // Expand the macro body by writing its lines to output.
            for (size_t i = 0; i < macro->line_count; i++) {
                if (fprintf(output_fd, "%s", macro->body[i]) < 0) {
                    fclose(input_fd);
                    fclose(output_fd);
                    return STATUS_CATASTROPHIC;
                }
            }
        } else {
            // Not a macro call; write the line as-is.
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

