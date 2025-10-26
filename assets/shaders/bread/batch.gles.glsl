#vertex
// Instance args
// always compact arguments
Input(0) vec4 instance_0;
Input(1) vec4 instance_1;
Input(2) vec4 instance_2;
Input(3) vec4 instance_3;
Input(4) vec4 instance_4;

// Sprites/UI Sprite
#if defined(SPRITE)
#define transform mat2(instance_0.xy, instance_0.zw)

#define transform_translation instance_1.xy
#define TextureInputSlot floatBitsToUint(instance_1.z)
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

#if defined(SPRITE)
Output(3) flat uint flags;
#endif

#if defined(CIRCLE)
Output(4) vec2 local_position;
Output(5) flat float radius;
#endif

#define FLAG_FLIP_H 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FONT 0x4U

void main()
{
    // Getting Vertex Index
#if defined(QUAD) || defined(CIRCLE) || defined(SPRITE)
    int index = gl_VertexID & 3;
#endif

    // Top left always
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
    // Always centered
    vertice += vec2(-0.5, 0.5);
    local_position = vertice * 2.0;
    radius = input_radius;
#endif
#endif

    // Getting vertex extension
#if defined(QUAD) || defined(SPRITE)
    vec4 VERTEX = vec4(rect.xy + (vertice.xy * rect.zw), 0, 1);
#elif defined(PRIMITIVE)
    vec4 VERTEX = vec4(input_point.x, input_point.y, 0, 1);
#elif defined(CIRCLE)
    vec4 VERTEX = vec4(vertice * 2.0 * input_radius, 0, 1);
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

    UV = out_uv;
    TextureUnit = TextureInputSlot;
#endif

    // Transforming
#if defined(QUAD) || defined(SPRITE)
    mat2 matrix_transform = transform;
    VERTEX.xy = matrix_transform * VERTEX.xy;
    VERTEX.xy += transform_translation;
#elif defined(CIRCLE)
    VERTEX.xy += input_point;
#endif

#if !defined(SPRITE_UI)
    VERTEX = SceneTransform * VERTEX;
#endif
    VERTEX = ViewportTransform * VERTEX;

#VERTEXCODE

    gl_Position = VERTEX;

    // Applying color
    Color = unpackUnorm4x8(input_color);
}


#fragment
precision mediump float;

#if defined(SPRITE)
Input(3) flat uint flags;
#endif

#if defined(CIRCLE)
Input(4) vec2 local_position;
Input(5) flat float radius;
#endif

#define FLAG_FLIP_H 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FONT 0x4U

void main()
{
    COLOR = Color;

#if defined(CUSTOM_FRAGMENT)
#FRAGMENTCODE
#elif defined(SPRITE)
    if(bool(flags & FLAG_FONT))
    {
        COLOR *= vec4(Sample(UV).r);
    }
    else
    {
        COLOR *= Sample(UV);
    }

#elif defined(CIRCLE)
    float edge_smoothness = 0.005;
    float dist = distance(vec2(0, 0), local_position);
    float alpha = 1.0 - smoothstep(1.0 - edge_smoothness, 1.0 + edge_smoothness, dist);
    COLOR.a *= alpha;
#endif
}
