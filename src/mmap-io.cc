/*
    Licensed under The MIT License (MIT)
    You will find the full license legal mumbo jumbo in file "LICENSE"

    Original Author: Oscar Campbell
    Rewritten for N-API (Node.js 24+) by <your-name>
*/

#include <napi.h>
#include <errno.h>
#include <string>

#ifdef _WIN32
#include <windows.h>
#include "mman.h"
#else
#include <unistd.h>
#include <sys/mman.h>
#endif

// Struct to keep track of mmap'ed memory
struct MMap {
    MMap(char* data, size_t size) : data(data), size(size) {}
    char* data = nullptr;
    size_t size = 0;
};

// Cleanup callback when Buffer is garbage-collected
void do_mmap_cleanup(Napi::Env env, void* data, void* hint) {
    auto map_info = static_cast<MMap*>(hint);
    munmap(data, map_info->size);
    delete map_info;
}

// Helper to call madvise
inline int do_mmap_advice(char* addr, size_t length, int advise) {
    return madvise(static_cast<void*>(addr), length, advise);
}

// === Utility functions to extract values from Napi::Value ===
template <typename T>
inline T get_v(const Napi::Value& v) {
    return v.As<Napi::Number>().Int64Value();
}

template <>
inline bool get_v<bool>(const Napi::Value& v) {
    return v.As<Napi::Boolean>().Value();
}

template <typename T>
inline T get_v(const Napi::Value& v, T default_value) {
    return v.IsUndefined() ? default_value : get_v<T>(v);
}

// =============================================
// mmap_map(size, protection, flags, fd, [offset], [advise])
// =============================================
Napi::Value mmap_map(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 4 || info.Length() > 6) {
        Napi::TypeError::New(env,
            "map() takes 4, 5 or 6 arguments: "
            "(size:int, protection:int, flags:int, fd:int [, offset:int [, advise:int]])"
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    if (!info[0].IsNumber() || !info[1].IsNumber() ||
        !info[2].IsNumber() || !info[3].IsNumber()) {
        Napi::TypeError::New(env,
            "Arguments 0-3 must be integers"
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    constexpr void* hinted_address = nullptr;
    const size_t size       = get_v<size_t>(info[0]);
    const int protection    = get_v<int>(info[1]);
    const int flags         = get_v<int>(info[2]);
    const int fd            = get_v<int>(info[3]);
    const size_t offset     = get_v<size_t>(info[4], 0);
    const int advise        = get_v<int>(info[5], 0);

    char* data = static_cast<char*>(mmap(hinted_address, size, protection, flags, fd, offset));

    if (data == MAP_FAILED) {
        Napi::Error::New(env,
            "mmap failed, errno=" + std::to_string(errno)
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    if (advise != 0) {
        int ret = do_mmap_advice(data, size, advise);
        if (ret) {
            Napi::Error::New(env,
                "madvise() failed, errno=" + std::to_string(errno)
            ).ThrowAsJavaScriptException();
            return env.Null();
        }
    }

    auto* map_info = new MMap(data, size);

    // Create a Node.js Buffer backed by mmap'd memory
    return Napi::Buffer<char>::New(env, data, size, do_mmap_cleanup, map_info);
}

// =============================================
// mmap_advise(buffer, advise) OR (buffer, offset, length, advise)
// =============================================
Napi::Value mmap_advise(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if ((info.Length() != 2 && info.Length() != 4) || !info[0].IsBuffer()) {
        Napi::TypeError::New(env,
            "advise() takes 2 or 4 arguments: "
            "(buffer:Buffer, advise:int) | "
            "(buffer:Buffer, offset:int, length:int, advise:int)"
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    auto buf = info[0].As<Napi::Buffer<char>>();
    char* data = buf.Data();
    size_t size = buf.Length();

    int ret = 0;
    if (info.Length() == 2) {
        int advise = get_v<int>(info[1], 0);
        ret = do_mmap_advice(data, size, advise);
    } else {
        int offset = get_v<int>(info[1], 0);
        int length = get_v<int>(info[2], 0);
        int advise = get_v<int>(info[3], 0);
        ret = do_mmap_advice(data + offset, length, advise);
    }

    if (ret) {
        Napi::Error::New(env,
            "madvise() failed, errno=" + std::to_string(errno)
        ).ThrowAsJavaScriptException();
    }

    return env.Undefined();
}

// =============================================
// mmap_incore(buffer)
// =============================================
Napi::Value mmap_incore(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 1 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env,
            "incore() takes 1 argument: (buffer:Buffer)"
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    auto buf = info[0].As<Napi::Buffer<char>>();
    char* data = buf.Data();
    size_t size = buf.Length();

#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    size_t page_size = sysinfo.dwPageSize;
#else
    size_t page_size = sysconf(_SC_PAGESIZE);
#endif

    size_t needed_bytes = (size + page_size - 1) / page_size;
    size_t pages = size / page_size;

    auto* result_data = static_cast<char*>(malloc(needed_bytes));
    if (!result_data) {
        Napi::Error::New(env, "malloc() failed").ThrowAsJavaScriptException();
        return env.Null();
    }

    if (size % page_size > 0) pages++;

    int ret = mincore(data, size, result_data);
    if (ret) {
        free(result_data);
        Napi::Error::New(env,
            errno == ENOSYS ? "mincore() not implemented"
                            : "mincore() failed, errno=" + std::to_string(errno)
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    uint32_t pages_mapped = 0, pages_unmapped = 0;
    for (size_t i = 0; i < pages; i++) {
        if (result_data[i] & 0x1)
            pages_mapped++;
        else
            pages_unmapped++;
    }

    free(result_data);

    Napi::Array arr = Napi::Array::New(env, 2);
    arr.Set(uint32_t(0), Napi::Number::New(env, pages_unmapped));
    arr.Set(uint32_t(1), Napi::Number::New(env, pages_mapped));
    return arr;
}

// =============================================
// mmap_sync_lib_private_(buffer, offset, length, blocking, invalidate)
// =============================================
Napi::Value mmap_sync_lib_private_(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() != 5 || !info[0].IsBuffer()) {
        Napi::TypeError::New(env,
            "sync() takes 5 arguments: "
            "(buffer:Buffer, offset:int, length:int, blocking:bool, invalidate:bool)"
        ).ThrowAsJavaScriptException();
        return env.Null();
    }

    auto buf = info[0].As<Napi::Buffer<char>>();
    char* data = buf.Data();

    int offset         = get_v<int>(info[1], 0);
    size_t length      = get_v<size_t>(info[2], 0);
    bool blocking_sync = get_v<bool>(info[3], false);
    bool invalidate    = get_v<bool>(info[4], false);

    int flags = (blocking_sync ? MS_SYNC : MS_ASYNC) |
                (invalidate ? MS_INVALIDATE : 0);

    int ret = msync(data + offset, length, flags);
    if (ret) {
        Napi::Error::New(env,
            "msync() failed, errno=" + std::to_string(errno)
        ).ThrowAsJavaScriptException();
    }

    return env.Undefined();
}

// =============================================
// Module Init
// =============================================
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Define constants
    exports.Set("PROT_READ", Napi::Number::New(env, PROT_READ));
    exports.Set("PROT_WRITE", Napi::Number::New(env, PROT_WRITE));
    exports.Set("PROT_EXEC", Napi::Number::New(env, PROT_EXEC));
    exports.Set("PROT_NONE", Napi::Number::New(env, PROT_NONE));

    exports.Set("MAP_SHARED", Napi::Number::New(env, MAP_SHARED));
    exports.Set("MAP_PRIVATE", Napi::Number::New(env, MAP_PRIVATE));

#ifdef MAP_NONBLOCK
    exports.Set("MAP_NONBLOCK", Napi::Number::New(env, MAP_NONBLOCK));
#endif
#ifdef MAP_POPULATE
    exports.Set("MAP_POPULATE", Napi::Number::New(env, MAP_POPULATE));
#endif

    exports.Set("MADV_NORMAL", Napi::Number::New(env, MADV_NORMAL));
    exports.Set("MADV_RANDOM", Napi::Number::New(env, MADV_RANDOM));
    exports.Set("MADV_SEQUENTIAL", Napi::Number::New(env, MADV_SEQUENTIAL));
    exports.Set("MADV_WILLNEED", Napi::Number::New(env, MADV_WILLNEED));
    exports.Set("MADV_DONTNEED", Napi::Number::New(env, MADV_DONTNEED));

#ifdef _WIN32
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    exports.Set("PAGESIZE", Napi::Number::New(env, sysinfo.dwPageSize));
#else
    exports.Set("PAGESIZE", Napi::Number::New(env, sysconf(_SC_PAGESIZE)));
#endif

    // Define functions
    exports.Set("map", Napi::Function::New(env, mmap_map));
    exports.Set("advise", Napi::Function::New(env, mmap_advise));
    exports.Set("incore", Napi::Function::New(env, mmap_incore));
    exports.Set("sync_lib_private__", Napi::Function::New(env, mmap_sync_lib_private_));

    return exports;
}

NODE_API_MODULE(mmap_io, Init)
