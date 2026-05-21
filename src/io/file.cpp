#include "io/file.h"

#include "collections/slice.h"
#include "io/writer.h"


IO::Writer File::writer()
{
    IO::Writer writer = {};
    writer.writable = reinterpret_cast<Opaque*>(this);
    writer.write_fn = [](Opaque* self, const Slice<const u8>& bytes) -> void
    {
        File& file = *self->cast<File*>();
        file.write(bytes);
    };
    return writer;
}