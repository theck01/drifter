
#ifndef API_PROVIDER
#define API_PROVIDER

#include "pd_api.h"

typedef struct provider_struct {
	PlaydateAPI* api;
} provider;

static provider PD = { .api = NULL };

#endif
