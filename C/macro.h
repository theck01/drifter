
#define max(a,b) \
 ({ __typeof__ (a) _a = (a); \
		 __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })

#define min(a,b) \
 ({ __typeof__ (a) _a = (a); \
		 __typeof__ (b) _b = (b); \
	 _a < _b ? _a : _b; })

#define clamp(a, lower_limit, upper_limit) \
 ({ __typeof__ (a) _a = (a); \
		 __typeof__ (lower_limit) _lower_limit = (lower_limit); \
		 __typeof__ (upper_limit) _upper_limit = (upper_limit); \
	 _a < _lower_limit ? _lower_limit : (_a > _upper_limit ? _upper_limit : _a); })

