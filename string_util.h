//---------------------------------------------------------------------------

#pragma once
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------

// Slightly different to asprintf, as it returns the allocated string.
char *rasprintf(const char *p, ...) __printflike(1, 2);

extern "C" char *strndup(const char *p, size_t length);

inline bool streq(const char *a, const char *b) { return strcmp(a, b) == 0; }

bool str_has_prefix(const char *p, const char *prefix);

// Returns the end of the write area. p must have enough space to store
// the result;
char *WriteJsonString(char *p, const char *text);

//---------------------------------------------------------------------------
