#vertex

#VERTEXCODE

void main()
{
    vec4 Vertex = vec4(0.0, 0.0, 0.0, 1.0);
    vec2 out_uv = vec2(0.0);
    int index = gl_VertexID & 3;
    
    // Vertex
    // Indices: 0, 1, 2, 2, 3, 0
    // 0 -> -1, -1
    // 1 -> 1, -1
    // 2 -> 1, 1
    // 3 -> -1, 1
    // UV:
    // 0 -> 0, 0
    // 1 -> 1, 0
    // 2 -> 1, 1
    // 3 -> 0, 1
    if(index == 0)
    {
        Vertex.xy = vec2(-1, -1);
        out_uv = vec2(0, 0);
    }
    else if(index == 1)
    {
        Vertex.xy = vec2(1, -1);
        out_uv = vec2(1, 0);
    }
    else if(index == 2)
    {
        Vertex.xy = vec2(1.0, 1.0);
        out_uv = vec2(1.0, 1.0);
    }
    else //if(index == 3)
    {
        Vertex.xy = vec2(-1.0, 1.0);
        out_uv = vec2(0.0, 1.0);
    }

#if defined(CUSTOM_VERTEX)
    gl_Position = vertex(Vertex);
#else
    gl_Position = Vertex;
#endif

    Color = vec4(1.0, 1.0, 1.0, 1.0);
    UV = out_uv;
    TextureUnit = uint(0.0);
}


#fragment

#FRAGMENTCODE

void main()
{
#if defined(CUSTOM_FRAGMENT)
    COLOR = fragment(COLOR);
#else
    COLOR = Sample(UV);
#endif
}

