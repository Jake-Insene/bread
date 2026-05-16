#include "platform/android/android_file.h"

#include "platform/android/android_engine.h"



static constexpr usize StderrHandle = 1;
static constexpr usize StdoutHandle = 2;

thread_local u8 StderrBuffer[4096] = {};
thread_local usize StderrBufferCounter = 0;

thread_local u8 StdoutBuffer[4096] = {};
thread_local usize StdoutBufferCounter = 0;


Slice<u8> File::read_all(const mem::Allocator& allocator, StringView path)
{
    Slice<char> tmp = allocator.array<char>(path.len + 1);
    mem::copy(tmp, path);
    
    AAsset* asset = AAssetManager_open(AndroidEngine::data.asset_manager, tmp.ptr(), AASSET_MODE_UNKNOWN);
    const void* buffer = AAsset_getBuffer(asset);
    usize length = AAsset_getLength64(asset);

    allocator.free(mem::to_bytes(tmp));
    
    Slice<u8> bytes = allocator.alloc(length, alignof(usize));
    mem::copy(bytes, Slice(reinterpret_cast<const u8*>(buffer), length));
    
    AAsset_close(asset);
    
    return bytes;
}

File File::get_stderr()
{
    return File{ .handle = 1 };
}

File File::get_stdout()
{
    return File{ .handle = 2 };
}

File File::get_stdin()
{
    return File{ .handle = 0 };
}

File File::open(const mem::Allocator& allocator, StringView path, OpenMode)
{
    Slice<char> tmp = allocator.array<char>(path.len + 1);
    mem::copy(tmp, path);

    allocator.free(mem::to_bytes(tmp));

    return File{ .handle = 0 };
}

bool File::exists(const mem::Allocator& allocator, StringView path)
{
    Slice<char> tmp = allocator.array<char>(path.len + 1);
    mem::copy(tmp, path);

    AAsset* asset = AAssetManager_open(AndroidEngine::data.asset_manager, tmp.ptr(), AASSET_MODE_UNKNOWN);

    allocator.free(mem::to_bytes(tmp));

    if(asset)
    {
        AAsset_close(asset);
        return true;
    }

    return false;
}

void File::destroy()
{
    if (handle == 0)
    {
        return;
    }
}

void File::write(const Slice<const u8> bytes)
{
    DebugAssert(handle != 0, "invalid file handler");

    if(handle == StderrHandle)
    {
        Slice<u8> buffer = Slice(StderrBuffer);
        buffer = buffer.add(StderrBufferCounter);
        mem::copy(buffer, bytes);
        StderrBufferCounter += bytes.len;
    }
    else if(handle == StdoutHandle)
    {
        Slice<u8> buffer = Slice(StdoutBuffer);
        buffer = buffer.add(StdoutBufferCounter);
        mem::copy(buffer, bytes);
        StdoutBufferCounter += bytes.len;
    }

}

void File::put(u8)
{
}

void File::read(Slice<u8>)
{
    DebugAssert(handle != 0, "invalid file handler");
}

void File::flush()
{
    DebugAssert(handle != 0, "invalid file handler");

    if(handle == StderrHandle)
    {
        __android_log_print(ANDROID_LOG_ERROR, "Bread", "%.*s", (int)StderrBufferCounter, reinterpret_cast<char*>(StderrBuffer));
        StderrBufferCounter = 0;
        Slice<u8> buffer = Slice(StderrBuffer);
        mem::set<u8>(buffer, 0);
    }
    else if(handle == StdoutHandle)
    {
        __android_log_print(ANDROID_LOG_INFO, "Bread", "%.*s", (int)StdoutBufferCounter, reinterpret_cast<char*>(StdoutBuffer));
        StdoutBufferCounter = 0;
        Slice<u8> buffer = Slice(StdoutBuffer);
        mem::set<u8>(buffer, 0);
    }
}

