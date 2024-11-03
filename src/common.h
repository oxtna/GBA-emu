#ifndef GBA_COMMON_H
#define GBA_COMMON_H

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>

namespace GBA {

struct Deleter
{
    void operator()(void* ptr) { free(ptr); }
};

template <class T>
using UniquePtr = std::unique_ptr<T, Deleter>;

}

#endif
