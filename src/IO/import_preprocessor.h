#ifndef IMPORT_PREPROCESSOR_H
#define IMPORT_PREPROCESSOR_H

#include <stdbool.h>
#include <stdint.h>

#include "stb_c_lexer.h"

bool replace_macros(const char* filename, stb_lexer* lex, char** file_data, int64_t* size);

#endif
