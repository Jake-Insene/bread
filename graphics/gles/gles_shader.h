#pragma once
#include "core/string.h"

inline StringView glsl_es_shader_header =R"(
#version 320 es
#extension GL_EXT_gpu_shader5 : require
)";

inline StringView glsl_core_shader_header = R"(
#version 450 core
)";

static constexpr usize gles_shader_constants_buffer_size = 512;
inline usize glsl_shader_constants_len = 0;
inline char glsl_shader_constants[gles_shader_constants_buffer_size] = {};
