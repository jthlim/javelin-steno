//---------------------------------------------------------------------------

#include <pthread.h>

//---------------------------------------------------------------------------

#ifdef JAVELIN_THREADS

void RunParallel(void (*func1)(void *context), void *context1,
                 void (*func2)(void *context), void *context2) {

  pthread_t thread;
  pthread_create(&thread, nullptr, (void *(*)(void *))func1, context1);
  (*func2)(context2);

  void *result;
  pthread_join(thread, &result);
}

#endif

//---------------------------------------------------------------------------
