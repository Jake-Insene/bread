#define Input(N) layout(location = N) in
#define Output(N) layout(location = N) out
#define Texture2D(N) layout(binding = N) uniform sampler2D


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

// For now Only 16 textures are available
Texture2D(0) Texture0;
Texture2D(1) Texture1;
Texture2D(2) Texture2;
Texture2D(3) Texture3;
Texture2D(4) Texture4;
Texture2D(5) Texture5;
Texture2D(6) Texture6;
Texture2D(7) Texture7;
Texture2D(8) Texture8;
Texture2D(9) Texture9;
Texture2D(10) Texture10;
Texture2D(11) Texture11;
Texture2D(12) Texture12;
Texture2D(13) Texture13;
Texture2D(14) Texture14;
Texture2D(15) Texture15;


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
