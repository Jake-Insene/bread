#if defined(VERTEX_SHADER)

#define VertexIndex gl_VertexIndex

vec4 Mul(mat4x4 M, vec4 V)
{
    return M * V;
}

#elif defined(FRAGMENT_SHADER)
vec4 LoadPixel(sampler2D Texture, vec2 UV)
{
    return texture(Texture, UV);
}
#endif

