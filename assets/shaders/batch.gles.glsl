#vertex
// Instance args
// always compact arguments
layout(location = 0) in vec4 instance_0;
layout(location = 1) in vec4 instance_1;
layout(location = 2) in vec4 instance_2;
layout(location = 3) in vec4 instance_3;
layout(location = 4) in vec4 instance_4;

// Sprites
#if defined(SPRITE) || defined(CANVAS_ELEMENT)
#define transform mat2(instance_0.xy, instance_0.zw)

#define transform_translation instance_1.xy
#define texture_input_slot floatBitsToUint(instance_1.z)
#define input_flags floatBitsToUint(instance_1.w)

#define rect instance_2
#define src_rect instance_3

#define input_color floatBitsToUint(instance_4.x)

// Quads
#elif defined(QUAD)

#define transform mat2(instance_0.xy, instance_0.zw)

#define transform_translation instance_1.xy
#define rect instance_2

#define input_color floatBitsToUint(instance_3.x)

// Points
#elif defined(PRIMITIVE)

#define input_point instance_0.xy
#define input_color floatBitsToUint(instance_0.z)
#define input_flags floatBitsToUint(instance_0.w)

#elif defined(CIRCLE)

#define input_point instance_0.xy
#define input_color floatBitsToUint(instance_0.z)
#define input_radius instance_0.w

#endif

layout(location = 0) out vec4 color;

#if defined(SPRITE) || defined(CANVAS_ELEMENT)
layout(location = 1) flat out uint texture_slot;
layout(location = 2) out vec2 uv;
layout(location = 3) flat out uint flags;
#endif

#if defined(CIRCLE)
layout(location = 4) out vec2 local_position;
layout(location = 5) out float radius;
#endif

#define FLAG_FLIP_H 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FONT 0x4U

layout(std140, binding = 0) uniform SceneUniform
{
    mat4 viewport_transform;
    mat4 scene_transform;
};

void main()
{
    // Getting Vertex Index
#if defined(QUAD) || defined(CIRCLE) || defined(SPRITE)
    int index = gl_VertexID & 3;
#endif

    // Top left always, center 'rect'
#if defined(QUAD) || defined(CIRCLE) || defined(SPRITE)
    // Vertex
    // Indices 0, 1, 2, 2, 3, 0
    // Top left quad
    // 0 ->  0, -1
    // 1 ->  1, -1
    // 2 ->  1,  0
    // 3 ->  0,  0
    vec2 vertice = vec2(0, 0);
    if(index == 0)
    {
        vertice = vec2(0, -1);
    }
    else if(index == 1)
    {
        vertice = vec2(1, -1);
    }
    else if(index == 2)
    {
        vertice = vec2(1, 0);
    }

#if defined(CIRCLE)
    local_position = vertice * 2;
    radius = input_radius;
#endif
#endif

    // Getting vertex extension
#if defined(QUAD) || defined(SPRITE)
    vec4 out_pos = vec4(rect.xy + (vertice.xy * rect.zw), 0, 1);
#elif defined(PRIMITIVE)
    vec4 out_pos = vec4(input_point.x, input_point.y, 0, 1);
#elif defined(CIRCLE)
    vec4 out_pos = vec4(vertice * input_radius, 0, 1);
#endif
    
    // Getting UV
#if defined(SPRITE)
    // UV
    // 0 -> 0, 0
    // 1 -> 1, 0
    // 2 -> 1, 1
    // 3 -> 0, 1
    vec2 min_corner = src_rect.xy;
    vec2 max_corner = src_rect.zw;

    vec2 out_uv = vec2(min_corner.x, max_corner.y);
    if(index == 0)
    {
        out_uv = min_corner;
    }
    else if(index == 1)
    {
        out_uv = vec2(max_corner.x, min_corner.y);
    }
    else if(index == 2)
    {
        out_uv = max_corner;
    }

    // Applying flags
    flags = input_flags;
    
    // Fliping on demand
    out_uv = mix(
        out_uv, 1.0 - out_uv, bvec2(bool(flags & FLAG_FLIP_H), bool(flags & FLAG_FLIP_V))
    );

    uv = out_uv;
    texture_slot = texture_input_slot;
#endif

#if defined(QUAD) || defined(SPRITE)
    mat2 matrix_transform = transform;
    out_pos.xy = matrix_transform * out_pos.xy;
    out_pos.xy += transform_translation;
#endif

#if !defined(NO_SCENE_TRANSFORM)
    out_pos = scene_transform * out_pos;
#endif
    out_pos = viewport_transform * out_pos;

    gl_Position = out_pos;

    // Applying color
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

layout(location = 0) in vec4 color;

#if defined(SPRITE)
layout(location = 1) flat in uint texture_slot;
layout(location = 2) in vec2 uv;
layout(location = 3) flat in uint flags;
#endif

#if defined(CIRCLE)
layout(location = 4) in vec2 local_position;
layout(location = 5) in float radius;
#endif

layout(location = 0) out vec4 frag_color;

#define FLAG_FLIP_H 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FONT 0x4U

#if defined(SPRITE)
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
#endif

void main()
{
#if defined(SPRITE)
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
#elif defined(QUAD) || defined(PRIMITIVE)
    frag_color = color;
#elif defined(CIRCLE)
    float distance = 1.0 - length(vec3(local_position, 0));
    float circle = smoothstep(0.0, 0.005, distance);
    circle *= smoothstep(10 + 0.005, 10, distance);
    frag_color = color;
    frag_color.a *= circle;
#endif
}
