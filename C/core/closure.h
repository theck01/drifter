
#ifndef UTIL_CLOSURE
#define UTIL_CLOSURE

#include <stdarg.h>

typedef struct closure_struct closure;

// Function can be called with a context known before calltime and an
// arbitrary number of arguments at calltime.
//
// va_list is already initialized for use with va_arg and should not be closed
// with va_end.
typedef void* (*closure_fn)(void* context, va_list args);
closure* closure_create(void* context, closure_fn fn);

typedef void (*closure_context_cleanup_fn)(void* context);
closure* closure_create_with_cleanup(
  void* context,
  closure_fn fn,
  closure_context_cleanup_fn cleanup_fn
);

void* closure_call(closure* c, ...);
void* closure_vcall(closure* c, va_list args);

// Should be called for an additional references to the closure beyond the first
// Will ensure that closures are only truly destroyed when all reference holders
// have called `destroy` rather than on first call.
void closure_retain(closure* c);

void closure_destroy(closure* c);

#endif
