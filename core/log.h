#pragma once


struct Log
{
    static void error(const char* fmt, ...);
    
    static void warning(const char* fmt, ...);
    
    static void info(const char* fmt, ...);
};
