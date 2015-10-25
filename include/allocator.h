#ifndef __V8_ARRAY_ALLOCATOR__
#define __V8_ARRAY_ALLOCATOR__

#include <include/v8.h>
#include <cstring>

using namespace v8;

class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
    virtual void* Allocate(size_t length)
    {
        void* data = AllocateUninitialized(length);
        return data == NULL ? data : memset(data, 0, length);
    }
    virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
    virtual void Free(void* data, size_t) { free(data); }
};


#endif
