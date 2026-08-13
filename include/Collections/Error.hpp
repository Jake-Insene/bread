#pragma once
#include "Core/Header.hpp"


enum class ErrorCode : usize
{
	Ok = 0,

	InvalidArgument,
	DivisionByZero,

	// IO
	
	FileNotFound,

	// Resource loading
	ResourceNotFound,
	InvalidResourceType,

	ImageCorrupted,

	// Graphics
	MaterialCompilationFail,
};


struct [[nodiscard]] Error
{
	ErrorCode code;
	const char* file_name;
	usize line;

	constexpr Error(ErrorCode c, const char* source_file_name, usize source_line) 
		:code(c), file_name(source_file_name), line(source_line) {}

	constexpr Error(ErrorCode c) : code(c), file_name(), line() {}
	
	constexpr operator ErrorCode() const { return code; }

	constexpr operator bool() const { return code == ErrorCode::Ok; }
};

#define MakeError(code) Error(code, __FILE__, __LINE__)


