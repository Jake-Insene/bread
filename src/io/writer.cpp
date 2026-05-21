#include "io/writer.h"



namespace IO
{

void Writer::write(const Slice<const u8>& bytes) const
{
    write_fn(writable, bytes);
}

}