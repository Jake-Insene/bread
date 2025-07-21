#include "fs/file.h"

io::Writer File::writer()
{
    io::Writer writer = io::Writer();
    writer.self = this;
    writer.write_fn = [](void* self, const Slice<const u8> bytes) -> void
    {
        File* file = (File*)self;
        file->write(bytes);
    };
    return writer;
}