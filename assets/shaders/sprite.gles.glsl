#vertex
// Instance args
// always compact arguments
layout(location = 0) in vec4 instance_0;
layout(location = 1) in vec4 instance_1;
layout(location = 2) in vec4 instance_2;
layout(location = 3) in vec4 instance_3;
layout(location = 4) in vec4 instance_4;

#define transform mat2(instance_0.xy, instance_0.zw)

#define transform_translation instance_1.xy
#define texture_input_slot floatBitsToUint(instance_1.z)
#define input_flags floatBitsToUint(instance_1.w)

#define texture_extent instance_2.xy
#define dest_extent instance_2.zw

#define src_rect instance_3

#define input_color floatBitsToUint(instance_4.x)

layout(location = 0) flat out uint texture_slot;
layout(location = 1) out vec2 uv;
layout(location = 2) flat out uint flags;
layout(location = 3) out vec4 color;

#define FLAG_TOP_LEFT 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FLIP_H 0x4U
#define FLAG_FONT 0x8U

layout(std140, binding = 0) uniform SceneUniform
{
    mat4 screen_transform;
    mat4 scene_transform;
};

void main()
{
    int index = gl_VertexID & 3;
    flags = input_flags;

    // Vertex
    // Indices 0, 1, 2, 2, 3, 0
    // Center quad
    // 0 -> -0.5, -0.5
    // 1 ->  0.5, -0.5
    // 2 ->  0.5,  0.5
    // 3 -> -0.5,  0.5
    vec2 vertice = vec2(-0.5, 0.5);
    if(index == 0)
    {
        vertice = vec2(-0.5, -0.5);
    }
    else if(index == 1)
    {
        vertice = vec2(0.5, -0.5);
    }
    else if(index == 2)
    {
        vertice = vec2(0.5, 0.5);
    }

    vertice.x += float(flags & FLAG_TOP_LEFT) * 0.5;
    vertice.y += float(flags & FLAG_TOP_LEFT) * -0.5;

    vec4 out_pos = vec4(vertice * dest_extent, 0, 1);

    // UV
    // 0 -> 0, 0
    // 1 -> 1, 0
    // 2 -> 1, 1
    // 3 -> 0, 1
    vec2 position = src_rect.xy;
    vec2 size = src_rect.zw;

    vec2 out_uv = vec2(position.x, size.y) / texture_extent;
    if(index == 0)
    {
        out_uv = position / texture_extent;
    }
    else if(index == 1)
    {
        out_uv = vec2(size.x, position.y) / texture_extent;
    }
    else if(index == 2)
    {
        out_uv = size / texture_extent;
    }

    out_uv.x = mix(out_uv.x, 1.0 - out_uv.x, float(bool(flags & FLAG_FLIP_H)));
    out_uv.y = mix(out_uv.y, 1.0 - out_uv.y, float(bool(flags & FLAG_FLIP_V)));
    
    mat2 matrix_transform = transform;
    out_pos.xy = matrix_transform * out_pos.xy;
    out_pos.xy += transform_translation;
    
    out_pos = scene_transform * out_pos;
    out_pos = screen_transform * out_pos;

    gl_Position = out_pos;
    uv = out_uv;
    texture_slot = texture_input_slot;

    // Color
    uint color_uint = input_color;
    uint color_r = color_uint & 255U;
    uint color_g = (color_uint >> 8U) & 255U;
    uint color_b = (color_uint >> 16U) & 255U;
    uint color_a = (color_uint >> 24U) & 255U;
    color = vec4(
        float(color_r),
        float(color_g),
        float(color_b),
        float(color_a)
    );
    color /= 255.0;
}


#fragment
precision mediump float;

layout(location = 0) flat in uint texture_slot;
layout(location = 1) in vec2 uv;
layout(location = 2) flat in uint flags;
layout(location = 3) in vec4 color;

layout(location = 0) out vec4 frag_color;

#define FLAG_TOP_LEFT 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FLIP_H 0x4U
#define FLAG_FONT 0x8U

layout(binding = 0) uniform sampler2D texture0;
layout(binding = 1) uniform sampler2D texture1;
layout(binding = 2) uniform sampler2D texture2;
layout(binding = 3) uniform sampler2D texture3;
layout(binding = 4) uniform sampler2D texture4;
layout(binding = 5) uniform sampler2D texture5;
layout(binding = 6) uniform sampler2D texture6;
layout(binding = 7) uniform sampler2D texture7;
layout(binding = 8) uniform sampler2D texture8;
layout(binding = 9) uniform sampler2D texture9;
layout(binding = 10) uniform sampler2D texture10;
layout(binding = 11) uniform sampler2D texture11;
layout(binding = 12) uniform sampler2D texture12;
layout(binding = 13) uniform sampler2D texture13;
layout(binding = 14) uniform sampler2D texture14;
layout(binding = 15) uniform sampler2D texture15;

void main()
{
    switch(int(texture_slot))
    {
    case 0:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture0, uv).r); }
        else { frag_color = texture(texture0, uv); }
        break;
    case 1:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture1, uv).r); }
        else { frag_color = texture(texture1, uv); }
        break;
    case 2:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture2, uv).r); }
        else { frag_color = texture(texture2, uv); }
        break;
    case 3:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture3, uv).r); }
        else { frag_color = texture(texture3, uv); }
        break;
    case 4:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture4, uv).r); }
        else { frag_color = texture(texture4, uv); }
        break;
    case 5:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture5, uv).r); }
        else { frag_color = texture(texture5, uv); }
        break;
    case 6:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture6, uv).r); }
        else { frag_color = texture(texture6, uv); }
        break;
    case 7:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture7, uv).r); }
        else { frag_color = texture(texture7, uv); }
        break;
    case 8:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture8, uv).r); }
        else { frag_color = texture(texture8, uv); }
        break;
    case 9:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture9, uv).r); }
        else { frag_color = texture(texture9, uv); }
        break;
    case 10:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture10, uv).r); }
        else { frag_color = texture(texture10, uv); }
        break;
    case 11:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture11, uv).r); }
        else { frag_color = texture(texture11, uv); }
        break;
    case 12:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture12, uv).r); }
        else { frag_color = texture(texture12, uv); }
        break;
    case 13:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture13, uv).r); }
        else { frag_color = texture(texture13, uv); }
        break;
    case 14:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture14, uv).r); }
        else { frag_color = texture(texture14, uv); }
        break;
    case 15:
        if(bool(flags & FLAG_FONT)) { frag_color = vec4(texture(texture15, uv).r); }
        else { frag_color = texture(texture15, uv); }
        break;
    }

    frag_color *= color;
}
