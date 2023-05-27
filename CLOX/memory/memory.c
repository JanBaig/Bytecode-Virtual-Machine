#include <stdlib.h>
#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0) {
		free(pointer);
		return NULL;
	} 

	void* result = realloc(pointer, newSize);
	if (result == NULL) exit(1); // will be NULL when not enough memory to allocate
	return result;
}

/*
How do mallc() & free() work internally?

- What C compiler is currently set to run my code here? 
- How can I change the compilers here?
- How many C compilers are there? Which ones are the best? 
- Where are there open source implementations? 
- Proceeed to study malloc.


*/