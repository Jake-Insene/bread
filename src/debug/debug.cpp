#include "debug/debug.h"
#include "log/log.h"

void Debug::assert_info(const char* file_path, usize file_path_len,
    usize line, const char* msg, usize msg_len)
{
    StringView file_path_view = StringView(file_path, file_path_len);
    StringView msg_view = StringView(msg, msg_len);

    Log::error("{}:{}: {}", file_path_view, line, msg_view);
}
