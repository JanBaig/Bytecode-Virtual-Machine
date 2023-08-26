#include <stdlib.h>

#include "memory.h"
#include "../objects/objects.h"
#include "../vm/vm.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0) {
		free(pointer);
		return NULL;
	} 

	void* result = realloc(pointer, newSize);
	if (result == NULL) exit(1); // will be NULL when not enough memory in system to allocate
	return result;
}

static void freeObj(Obj* object) { 
	switch (object->type) {
		case OBJ_STRING: {
			ObjString* string = (ObjString*)object;
			FREE_ARRAY(char, string->chars, string->length + 1);
			FREE(ObjString, object);
			break;
		}
	}
}

void freeObjects() {
	Obj* object = vm.objects;
	while (object != NULL) {
		Obj* next = object->next;
		freeObj(object);
		object = next;
	}
}

