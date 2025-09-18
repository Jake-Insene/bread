#include "fs/file.h"

#include "collections/slice.h"
#include "io/writer.h"


io::Writer File::writer()
{
    io::Writer writer = {};
    writer.self = this;
    writer.write_fn = [](void* self, const Slice<const u8> bytes) -> void
    {
        File* file = (File*)self;
        file->write(bytes);
    };
    return writer;
}