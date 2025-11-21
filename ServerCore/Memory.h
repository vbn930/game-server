#pragma once
#include "Allocator.h"

/*------------------
	Memory
------------------*/

template<typename Type, typename... Args>
Type* xnew(Args&& ...args) {
	Type* memory = static_cast<Type*>(xxalloc(sizeof(Type)));

	// placement new
	new(memory) Type(std::forward<Args>(args)...);
	return memory;
}

template<typename Type>
void xdelete(Type* obj) {
	obj->~Type();
	xxrelease(obj);
}