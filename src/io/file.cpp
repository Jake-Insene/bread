#include "IO/File.hpp"

#include "Collections/Slice.hpp"
#include "IO/Writer.hpp"


namespace IO
{

Writer File::writer()
{
    Writer writer = {};
    writer.writable = Core::Opaque::from(*this);
    writer.write_fn = [](Core::Opaque* self, const Slice<const u8>& bytes) -> void
    {
        File& file = *self->cast<File*>();
        file.write(bytes);
    };
    return writer;
}

}
