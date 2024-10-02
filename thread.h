//---------------------------------------------------------------------------

#pragma once
#include <assert.h>

//---------------------------------------------------------------------------

#ifdef JAVELIN_THREADS

void RunParallel(void (*func1)(void *context), void *context1,
                 void (*func2)(void *context), void *context2);

#endif

//---------------------------------------------------------------------------
