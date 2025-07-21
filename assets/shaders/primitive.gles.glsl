#vertex
// Instance args
// always compact arguments
layout(location = 0) in vec4 instance_0;

#if defined(QUAD)
layout(location = 1) in vec4 instance_1;
layout(location = 2) in vec4 instance_2;

#define transform_0 instance_0.xy
#define transform_1 instance_0.zw

#define transform_2 instance_1.xy
#define size instance_1.zw

#define input_color floatBitsToUint(instance_2.x)

#elif defined(PRIMITIVE)

#define input_point instance_0.xy
#define input_color floatBitsToUint(instance_0.z)
#define input_flags floatBitsToUint(instance_0.w)

#endif

layout(location = 0) out vec4 color;

layout(std140, binding = 0) uniform SceneUniform
{
    mat4 screen_transform;
    mat4 scene_transform;
    mat4 canvas_transform;
};

void main()
{
    // Transform
    // TODO: we still needing to negate the Y axis to correct Y behavior.
    // (in the engine Y positive is up)
#if defined(QUAD)
    int index = gl_VertexID & 3;
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

    vec4 out_pos = vec4(vertice * size, 0, 1);
#elif defined(PRIMITIVE)
    vec4 out_pos = vec4(input_point.x, input_point.y, 0, 1);
#endif

#if defined(QUAD)
    mat2 rot = transpose(mat2(transform_0, transform_1));
    out_pos.xy = rot * out_pos.xy;
    out_pos.xy += transform_2;
    out_pos = scene_transform * out_pos;
#endif
    out_pos = screen_transform * out_pos;

    // Output
    gl_Position = out_pos;
    
    // Color
    uint color_uint = input_color;
    uint color_r = color_uint & 255U;
    uint color_g = (color_uint >> 8U) & 255U;
    uint color_b = (color_uint >> 16U) & 255U;
    uint color_a = (color_uint >> 24U) & 255U;
    color = vec4(
        float(color_r) / 255.0,
        float(color_g) / 255.0,
        float(color_b) / 255.0,
        float(color_a) / 255.0
    );
}


#fragment
precision mediump float;

layout(location = 0) in vec4 color;

layout(location = 0) out vec4 frag_color;

void main()
{
    frag_color = color;
}

