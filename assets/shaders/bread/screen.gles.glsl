#vertex
Input(0) vec4 VertexAttrib0;

#define TextureInputSlot floatBitsToUint(VertexAttrib0.x)

#VERTEXCODE

void main()
{
    vec4 Vertex = vec4(0);
    vec4 out_uv = vec2(0);
    uint index = gl_VertexID & 3;

    // Vertex
    // Indices: 0, 1, 2, 2, 3, 0
    // 0:
    // 1:
    // 2:
    // 3:
    // UV:
    // 0:
    // 1:
    // 2:
    // 3:
    if(index == 0)
    {

    }
    else if(index == 1)
    {

    }
    else if(index == 2)
    {

    }
    else //if(index == 3)
    {

    }

#if defined(CUSTOM_VERTEX)
    Vertex = vertex(Vertex);
#else
    gl_Position = Vertex;
#endif

    Color = vec4(1, 1, 1, 1);
    UV = out_uv;
    TextureUnit = TextureInputSlot;
}


#fragment

Output(0) vec4 COLOR;

#FRAGMENTCODE

void main()
{
#if defined(CUSTOM_FRAGMENT)
    COLOR = fragment(COLOR);
#else
    COLOR *= Sample(UV);
#endif
}

