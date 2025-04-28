#include "platform/android/android_file.h"

#include "platform/android/android_engine.h"

#include <memory>

Slice<u8> File::read_all(mem::Allocator& allocator, StringView path)
{
    char tmp[256] = {};
    std::memcpy(tmp, path.ptr(), path.len);
    
    AAsset* asset = AAssetManager_open(AndroidEngine::asset_manager, tmp, AASSET_MODE_UNKNOWN);
    const void* buffer = AAsset_getBuffer(asset);
    usize length = AAsset_getLength64(asset);
    
    Slice<u8> bytes = allocator.alloc(length, alignof(usize));
    std::memcpy(bytes.ptr(), buffer, length);
    
    AAsset_close(asset);
    
    return bytes;
}
