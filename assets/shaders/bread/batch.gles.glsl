#vertex
// Instance args
// Always compact arguments
Input(0) vec4 Instance0;
Input(1) vec4 Instance1;
Input(2) vec4 Instance2;
Input(3) vec4 Instance3;
Input(4) vec4 Instance4;

// Sprites/UI Sprite
#if defined(SPRITE)
#define Transform mat2(Instance0.xy, Instance0.zw)

#define TransformTranslation Instance1.xy
#define TextureInputSlot floatBitsToUint(Instance1.z)
#define InputFlags floatBitsToUint(Instance1.w)

#define Rect Instance2
#define SrcRect Instance3

#define InputColor floatBitsToUint(Instance4.x)

// Quads
#elif defined(QUAD)

#define Transform mat2(Instance0.xy, Instance0.zw)

#define TransformTranslation Instance1.xy
#define Rect Instance2

#define InputColor floatBitsToUint(Instance3.x)

// Points
#elif defined(PRIMITIVE)

#define InputPoint Instance0.xy
#define InputColor floatBitsToUint(Instance0.z)
#define InputFlags floatBitsToUint(Instance0.w)

#elif defined(CIRCLE)

#define InputPoint Instance0.xy
#define InputColor floatBitsToUint(Instance0.z)
#define InputRadius Instance0.w

#endif

#if defined(SPRITE)
Output(3) flat uint Flags;
#endif

#if defined(CIRCLE)
Output(4) vec2 LocalPosition;
Output(5) flat float Radius;
#endif

#define FLAG_FLIP_H 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FONT 0x4U

#VERTEXCODE

void main()
{
    // Getting Vertex Index
#if defined(QUAD) || defined(CIRCLE) || defined(SPRITE)
    int index = gl_VertexID & 3;
#endif

#if defined(QUAD) || defined(CIRCLE) || defined(SPRITE)
    // Vertex
    // Indices 0, 1, 2, 2, 3, 0
    // 0 ->  0, -1
    // 1 ->  1, -1
    // 2 ->  1,  0
    // 3 ->  0,  0
    vec2 vertice = vec2(0);
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
    else //if(index == 3)
    {
        vertice = vec2(0, 0);
    }

#if defined(CIRCLE)
    // Always centered
    vertice += vec2(-0.5, 0.5);
    LocalPosition = vertice * 2.0;
    Radius = InputRadius;
#endif
#endif

    // Getting vertex extension
#if defined(QUAD) || defined(SPRITE)
    vec4 Vertex = vec4(Rect.xy + (vertice.xy * Rect.zw), 0, 1);
#elif defined(PRIMITIVE)
    vec4 Vertex = vec4(InputPoint.x, InputPoint.y, 0, 1);
#elif defined(CIRCLE)
    vec4 Vertex = vec4(vertice * 2.0 * InputRadius, 0, 1);
#endif

#if defined(CUSTOM_VERTEX)
    Vertex = vertex(Vertex);
#endif
    
    // Getting UV
#if defined(SPRITE)
    // UV
    // 0 -> 0, 0
    // 1 -> 1, 0
    // 2 -> 1, 1
    // 3 -> 0, 1
    vec2 min_corner = SrcRect.xy;
    vec2 max_corner = SrcRect.zw;

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
    Flags = InputFlags;
    
    // Fliping on demand
    out_uv = mix(
        out_uv, 1.0 - out_uv, bvec2(bool(InputFlags & FLAG_FLIP_H), bool(InputFlags & FLAG_FLIP_V))
    );

    UV = out_uv;
    TextureUnit = TextureInputSlot;
#endif

    // Transforming
#if defined(QUAD) || defined(SPRITE)
    mat2 matrix_transform = Transform;
    Vertex.xy = matrix_transform * Vertex.xy;
    Vertex.xy += TransformTranslation;
#elif defined(CIRCLE)
    Vertex.xy += InputPoint;
#endif

#if !defined(SPRITE_UI)
    Vertex = SceneTransform * Vertex;
#endif
    Vertex = ViewportTransform * Vertex;

    gl_Position = Vertex;

    // Applying color
    Color = unpackUnorm4x8(InputColor);
}

#fragment

#if defined(SPRITE)
Input(3) flat uint Flags;
#endif

#if defined(CIRCLE)
Input(4) vec2 LocalPosition;
Input(5) flat float Radius;
#endif

#define FLAG_FLIP_H 0x1U
#define FLAG_FLIP_V 0x2U
#define FLAG_FONT 0x4U

#FRAGMENTCODE

void main()
{
    COLOR = Color;

#if defined(CUSTOM_FRAGMENT)
    COLOR = fragment(COLOR);
#elif defined(SPRITE)
    if(bool(Flags & FLAG_FONT))
    {
        COLOR *= vec4(Sample(UV).r);
    }
    else
    {
        COLOR *= Sample(UV);
    }
#elif defined(CIRCLE)
    float edge_smoothness = 0.005;
    float dist = distance(vec2(0, 0), LocalPosition);
    float alpha = 1.0 - smoothstep(1.0 - edge_smoothness, 1.0 + edge_smoothness, dist);
    COLOR.a *= alpha;
#endif
}
