//---------------------------------------------------------------------------

#pragma once
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

//---------------------------------------------------------------------------

#ifdef JAVELIN_THREADS

void RunParallel(void (*func1)(void *context), void *context1,
                 void (*func2)(void *context), void *context2);

#endif

//---------------------------------------------------------------------------
