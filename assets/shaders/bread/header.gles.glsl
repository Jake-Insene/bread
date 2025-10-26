#define Input(N) layout(location = N) in
#define Output(N) layout(location = N) out
#define InputOutput(N) layout(location = N) inout

// Common Uniform Data
layout(std140, binding = 0) uniform SceneUniform
{
    mat4 ViewportTransform;
    mat4 SceneTransform;
    float Time;
};


#if defined(VERTEX_SHADER)

Output(0) vec4 Color;
Output(1) flat uint TextureUnit;
Output(2) vec2 UV;

#elif defined(FRAGMENT_SHADER)

Input(0) vec4 Color;
Input(1) flat uint TextureUnit;
Input(2) vec2 UV;

Output(0) vec4 COLOR;

layout(binding = 0) uniform sampler2D Texture0;
layout(binding = 1) uniform sampler2D Texture1;
layout(binding = 2) uniform sampler2D Texture2;
layout(binding = 3) uniform sampler2D Texture3;
layout(binding = 4) uniform sampler2D Texture4;
layout(binding = 5) uniform sampler2D Texture5;
layout(binding = 6) uniform sampler2D Texture6;
layout(binding = 7) uniform sampler2D Texture7;
layout(binding = 8) uniform sampler2D Texture8;
layout(binding = 9) uniform sampler2D Texture9;
layout(binding = 10) uniform sampler2D Texture10;
layout(binding = 11) uniform sampler2D Texture11;
layout(binding = 12) uniform sampler2D Texture12;
layout(binding = 13) uniform sampler2D Texture13;
layout(binding = 14) uniform sampler2D Texture14;
layout(binding = 15) uniform sampler2D Texture15;

vec4 Sample(vec2 UV)
{
    switch(int(TextureUnit))
    {
    case 0:
        return texture(Texture0, UV);
    case 1:
        return texture(Texture1, UV);
    case 2:
        return texture(Texture2, UV);
    case 3:
        return texture(Texture3, UV);
    case 4:
        return texture(Texture4, UV);
    case 5:
        return texture(Texture5, UV);
    case 6:
        return texture(Texture6, UV);
    case 7:
        return texture(Texture7, UV);
    case 8:
        return texture(Texture8, UV);
    case 9:
        return texture(Texture9, UV);
    case 10:
        return texture(Texture10, UV);
    case 11:
        return texture(Texture11, UV);
    case 12:
        return texture(Texture12, UV);
    case 13:
        return texture(Texture13, UV);
    case 14:
        return texture(Texture14, UV);
    case 15:
        return texture(Texture15, UV);
    }
}

#endif
