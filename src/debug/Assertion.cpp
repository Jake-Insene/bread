#include "Debug/Assertion.hpp"

#include "Debug/Log.hpp"


void Debug::assert_info(const char* path, usize file_path_len,
    usize line, const char* msg, usize msg_len)
{
    StringView path_view = StringView(path, file_path_len);
    StringView msg_view = StringView(msg, msg_len);

    Log::error("{} at line {}: {}", path_view, line, msg_view);
}
