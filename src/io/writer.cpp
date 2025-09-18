#include "io/writer.h"
#include "collections/slice.h"



namespace io
{

void Writer::write(const Slice<const u8> bytes) const
{
    write_fn(self, bytes);
}

}