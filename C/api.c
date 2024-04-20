
#include "api.h"

static PlaydateAPI* api = NULL;

PlaydateAPI* get_api() {
	return api;
}

void set_api(PlaydateAPI* new_api) {
	api = new_api;
}
