#include "Debug/Assertion.hpp"

#include "Debug/Log.hpp"


void Debug::assert_info(const char* path, usize file_path_len,
    usize line, const char* msg, usize msg_len)
{
    Collections::StringView path_view{path, file_path_len};
    Collections::StringView msg_view{msg, msg_len};

    Log::error("{} at line {}: {}", path_view, line, msg_view);
}
