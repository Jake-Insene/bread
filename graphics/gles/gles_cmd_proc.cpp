#include "graphics/gles/gles_cmd_proc.h"

#include "engine/engine.h"
#include "graphics/gles/gles_driver.h"
#include "graphics/gles/gles_memory_allocator.h"
#include "graphics/gles/gles_utility.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/egl/egl.h"
#include "math/projection.h"

static inline void _vertex_attrib_divisor(GLint index, GLenum type, GLsizei component_count, 
    GLsizei stride, GLsizei offset)
{
    gl.glVertexAttribPointer(index, component_count, type, GL_FALSE, stride, (const void*)(u64)offset);
    gl.glEnableVertexAttribArray(index);
    gl.glVertexAttribDivisor(index, 1);
}

static inline void _vertex_attrib(GLint index, GLenum type, GLsizei component_count,
    GLsizei stride, GLsizei offset)
{
    gl.glVertexAttribPointer(index, component_count, type, GL_FALSE, stride, (const void*)(u64)offset);
    gl.glEnableVertexAttribArray(index);
}

// Global quad index buffer
static constexpr const u8 indices[] = {
    0, 1, 2, 2, 3, 0
};

void GLESCommandProcessor::initialize(mem::Allocator allocator)
{
    data.allocator = allocator;

    data.commands = Array<RenderCommand>::with_size(data.allocator, 64);

    data.global_quad_ibo = GLESMemoryAllocator::buffer_allocate_handle();
    GLESMemoryAllocator::buffer_fill_memory(
        data.global_quad_ibo, indices, GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW
    );

    data.usable_texture_units = 16;

    // Sprite batch
    {
        data.sprite_batch = {};

        gl.glGenVertexArrays(1, &data.sprite_batch.vao);
        data.sprite_batch.instance_buffer_object = GLESMemoryAllocator::buffer_allocate_handle();
        gl.glBindVertexArray(data.sprite_batch.vao);

        GLESMemoryAllocator::buffer_fill_memory(
            data.sprite_batch.instance_buffer_object, Slice<const u8>(nullptr, sizeof(SpriteInstance) * MaxInstancesPerBatch),
            GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW
        );
        data.sprite_batch.instances = allocator.array<SpriteInstance>(MaxInstancesPerBatch);
    
        gl.glBindBuffer(GL_ARRAY_BUFFER, data.sprite_batch.instance_buffer_object);
        for(u32 i = 0; i < SpriteInstanceAttribCount; i++)
        {
            _vertex_attrib_divisor(i, GL_FLOAT, 4, sizeof(SpriteInstance), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);
        
        data.sprite_batch.program = gles::compile_program("shaders/sprite.gles.glsl", {});
    }

    // Canvas element batch
    {
        data.canvas_element_batch = {};

        gl.glGenVertexArrays(1, &data.canvas_element_batch.vao);
        data.canvas_element_batch.instance_buffer_object = GLESMemoryAllocator::buffer_allocate_handle();
        gl.glBindVertexArray(data.canvas_element_batch.vao);

        GLESMemoryAllocator::buffer_fill_memory(
            data.canvas_element_batch.instance_buffer_object, Slice<const u8>(nullptr, sizeof(CanvasElementInstance) * MaxInstancesPerBatch),
            GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW
        );
        data.canvas_element_batch.instances = allocator.array<CanvasElementInstance>(MaxInstancesPerBatch);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.canvas_element_batch.instance_buffer_object);
        for (u32 i = 0; i < CanvasElementInstanceAttribCount; i++)
        {
            _vertex_attrib_divisor(i, GL_FLOAT, 4, sizeof(CanvasElementInstance), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);

        data.canvas_element_batch.program = gles::compile_program("shaders/canvas.gles.glsl", {});
    }
    
    // Quad batch
    {
        data.quad_batch = {};

        gl.glGenVertexArrays(1, &data.quad_batch.vao);
        data.quad_batch.instance_buffer_object = GLESMemoryAllocator::buffer_allocate_handle();
        gl.glBindVertexArray(data.quad_batch.vao);

        GLESMemoryAllocator::buffer_fill_memory(
            data.quad_batch.instance_buffer_object, Slice<const u8>(nullptr, sizeof(QuadInstance) * MaxInstancesPerBatch),
            GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW
        );
        data.quad_batch.instances = data.allocator.array<QuadInstance>(MaxInstancesPerBatch);
        
        gl.glBindBuffer(GL_ARRAY_BUFFER, data.quad_batch.instance_buffer_object);
        for(u32 i = 0; i < QuadInstanceAttribCount; i++)
        {
            _vertex_attrib_divisor(i, GL_FLOAT, 4, sizeof(QuadInstance), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);
        
        data.quad_batch.program = gles::compile_program("shaders/primitive.gles.glsl", "#define QUAD");
    }

    // Primitive batch
    {
        data.primitive_batch = {};

        gl.glGenVertexArrays(1, &data.primitive_batch.vao);
        data.primitive_batch.instance_buffer_object = GLESMemoryAllocator::buffer_allocate_handle();
        gl.glBindVertexArray(data.primitive_batch.vao);

        GLESMemoryAllocator::buffer_fill_memory(
            data.primitive_batch.instance_buffer_object, 
            Slice<const u8>(nullptr, sizeof(PrimitivePoint) * MaxPrimitivePointsPerBatch),
            GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW
        );
        data.primitive_batch.primitives = data.allocator.array<PrimitivePoint>(MaxPrimitivePointsPerBatch);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.primitive_batch.instance_buffer_object);
        for(u32 i = 0; i < PrimitivePointAttribCount; i++)
        {
            _vertex_attrib(i, GL_FLOAT, 4, sizeof(PrimitivePoint), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);

        data.primitive_batch.program = gles::compile_program("shaders/primitive.gles.glsl", "#define PRIMITIVE");
    }

    data.scene_data_ubo = GLESMemoryAllocator::buffer_allocate_handle();
    GLESMemoryAllocator::buffer_fill_memory(
        data.scene_data_ubo, Slice<const u8>(nullptr, sizeof(SceneUniform)), GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW
    );
    
    data.scene_data.screen_transform = Mat4::identity();
    data.scene_data.scene_transform = Mat4::identity();
    data.scene_data_ubo_update = true;
    
    data.state =
    {
        .last_fbo = 0,
        .current_fbo = 0,
        .current_fb = {},
    };
}

void GLESCommandProcessor::shutdown()
{
    data.commands.destroy();

    GLESMemoryAllocator::buffer_deallocate_handle(data.global_quad_ibo, sizeof(indices));
    
    // Sprite batch
    {
        GLESMemoryAllocator::buffer_deallocate_handle(
            data.sprite_batch.instance_buffer_object, sizeof(SpriteInstance) * MaxInstancesPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.sprite_batch.vao);
        gl.glDeleteProgram(data.sprite_batch.program);
        
        data.allocator.free(mem::to_bytes(data.sprite_batch.instances));
    }

    // Canvas element batch
    {
        GLESMemoryAllocator::buffer_deallocate_handle(
            data.canvas_element_batch.instance_buffer_object, sizeof(CanvasElementInstance) * MaxInstancesPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.canvas_element_batch.vao);
        gl.glDeleteProgram(data.canvas_element_batch.program);

        data.allocator.free(mem::to_bytes(data.canvas_element_batch.instances));
    }
    
    // Quad batch
    {
        GLESMemoryAllocator::buffer_deallocate_handle(
            data.quad_batch.instance_buffer_object, sizeof(QuadInstance) * MaxInstancesPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.quad_batch.vao);
        gl.glDeleteProgram(data.quad_batch.program);
        
        data.allocator.free(mem::to_bytes(data.quad_batch.instances));
    }

    // Primitive batch
    {
        GLESMemoryAllocator::buffer_deallocate_handle(
            data.primitive_batch.instance_buffer_object, sizeof(PrimitivePoint) * MaxPrimitivePointsPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.primitive_batch.vao);
        gl.glDeleteProgram(data.primitive_batch.program);

        data.allocator.free(mem::to_bytes(data.primitive_batch.primitives));
    }

    GLESMemoryAllocator::buffer_deallocate_handle(data.scene_data_ubo, sizeof(SceneUniform));
}

void GLESCommandProcessor::recreate_window_transform(Vector2I window_size)
{
}

void GLESCommandProcessor::bind_program(GLID program)
{
    gl.glUseProgram(program);
}

void GLESCommandProcessor::bind_scene_buffer()
{
    gl.glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.scene_data_ubo);
}

void GLESCommandProcessor::update_scene_uniform()
{
    if(data.scene_data_ubo_update)
    {
        GLESMemoryAllocator::buffer_update_memory(
            data.scene_data_ubo, 0, Slice<const u8>((u8*)&data.scene_data, sizeof(SceneUniform)), 
            GL_UNIFORM_BUFFER
        );
        data.scene_data_ubo_update = false;
    }
}

void GLESCommandProcessor::end_sprite_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.sprite_batch.program);
    bind_scene_buffer();
    gl.glBindVertexArray(data.sprite_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);

    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.sprite_batch.instance_buffer_object, 0, 
        mem::to_const_bytes(data.sprite_batch.instances), GL_ARRAY_BUFFER
    );
    
    for(i32 i = 0; i < GLESCommandProcessor::data.sprite_batch.texture_index; i++)
    {
        gl.glActiveTexture(GL_TEXTURE0 + i);
        gl.glBindTexture(GL_TEXTURE_2D, data.sprite_batch.texture_units[i]);
    }

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.sprite_batch.count);
    data.sprite_batch.count = 0;
    data.sprite_batch.texture_index = 0;
}

void GLESCommandProcessor::end_canvas_element_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.canvas_element_batch.program);
    bind_scene_buffer();
    gl.glBindVertexArray(data.canvas_element_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);

    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.canvas_element_batch.instance_buffer_object, 0, 
        mem::to_const_bytes(data.canvas_element_batch.instances), GL_ARRAY_BUFFER
    );

    for (i32 i = 0; i < GLESCommandProcessor::data.canvas_element_batch.texture_index; i++)
    {
        gl.glActiveTexture(GL_TEXTURE0 + i);
        gl.glBindTexture(GL_TEXTURE_2D, data.canvas_element_batch.texture_units[i]);
    }

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.canvas_element_batch.count);
    data.canvas_element_batch.count = 0;
    data.canvas_element_batch.texture_index = 0;
}

void GLESCommandProcessor::end_quad_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.quad_batch.program);
    bind_scene_buffer();
    gl.glBindVertexArray(data.quad_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);

    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.quad_batch.instance_buffer_object, 0, mem::to_const_bytes(data.quad_batch.instances), 
        GL_ARRAY_BUFFER
    );

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.quad_batch.count);
    data.quad_batch.count = 0;
}

void GLESCommandProcessor::end_primitive_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.primitive_batch.program);
    bind_scene_buffer();

    gl.glBindVertexArray(data.primitive_batch.vao);
    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.primitive_batch.instance_buffer_object, 0, mem::to_const_bytes(data.primitive_batch.primitives),
        GL_ARRAY_BUFFER
    );

    gl.glDrawArrays(GL_LINES, 0, (GLsizei)data.primitive_batch.count);
    data.primitive_batch.count = 0;
}

void GLESCommandProcessor::render()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count = 0;
#endif

    for(usize i = 0; i < data.commands.count; i++)
    {
        RenderCommand& cmd = data.commands[i];
        switch(cmd.type)
        {
        case RenderCommand::BIND_RENDER_TARGET:
        {
            auto& rt = GLESMemoryAllocator::render_target_get(cmd.bind.source_id);
            data.scene_data.screen_transform = Projection::orthographic(
                0, rt.size.width, -rt.size.height, 0,
                1, -1
            );
            data.scene_data.screen_transform.transpose();

            data.scene_data_ubo_update = true;
           
            if(data.state.current_fbo != rt.framebuffer)
            {
                data.state.current_fb = cmd.bind.source_id;
                gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.framebuffer);
                gl.glViewport(0, 0, rt.size.width, rt.size.height);
                
                data.state.last_fbo = data.state.current_fbo;
                data.state.current_fbo = rt.framebuffer;
            }
        }
            break;
        case RenderCommand::CLEAR_RENDER_TARGET:
        {
            if(data.state.current_fb != cmd.clear.rid)
            {
                auto& rt = GLESMemoryAllocator::render_target_get(cmd.clear.rid);
                gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.framebuffer);
            }

            gl.glClearColor(
                (f32)cmd.clear.color.r / 255.f,
                (f32)cmd.clear.color.g / 255.f,
                (f32)cmd.clear.color.b / 255.f,
                (f32)cmd.clear.color.a / 255.f
            );
            gl.glClear(GL_COLOR_BUFFER_BIT);
            
            if(data.state.current_fb != cmd.clear.rid)
            {
                gl.glBindFramebuffer(GL_FRAMEBUFFER, data.state.current_fbo);
            }
        }
            break;
        case RenderCommand::DRAW_SPRITE:
        {
            if (data.sprite_batch.count >= MaxInstancesPerBatch ||
                data.sprite_batch.texture_index >= data.usable_texture_units)
            {
                DebugInfo("Sprite Batch full, flushing...");
                update_scene_uniform();
                end_sprite_batch();
            }

            GLID tex = GLESMemoryAllocator::texture_get_handle(cmd.sprite.texture);

            i32 tex_unit = -1;
            for (i32 t = 0; t < data.sprite_batch.texture_index; t++)
            {
                if (data.sprite_batch.texture_units[t] == i32(tex))
                {
                    tex_unit = t;
                    break;
                }
            }

            if (tex_unit == -1)
            {
                tex_unit = data.sprite_batch.texture_index;
                data.sprite_batch.texture_units[data.sprite_batch.texture_index] = tex;
                data.sprite_batch.texture_index++;
            }

            u32 index = data.sprite_batch.count;

            data.sprite_batch.instances[index].transform_0 = cmd.sprite.transform[0];
            data.sprite_batch.instances[index].transform_1 = cmd.sprite.transform[1];
            data.sprite_batch.instances[index].transform_2 = cmd.sprite.transform[2];

            data.sprite_batch.instances[index].unit = tex_unit;
            data.sprite_batch.instances[index].flags = cmd.sprite.flags;
            
            data.sprite_batch.instances[index].texture_extent = cmd.sprite.texture_extent;
            data.sprite_batch.instances[index].dest_extent = cmd.sprite.dest_extent;
            
            data.sprite_batch.instances[index].src_rect = cmd.sprite.src_rect;
            data.sprite_batch.instances[index].color = cmd.sprite.color;

            data.sprite_batch.count++;
        }
            break;
        case RenderCommand::DRAW_CANVAS_ELEMENT:
        {
            if (data.canvas_element_batch.count >= MaxInstancesPerBatch ||
                data.canvas_element_batch.texture_index >= data.usable_texture_units)
            {
                DebugInfo("Canvas Element Batch full, flushing...");
                update_scene_uniform();
                end_canvas_element_batch();
            }

            GLID tex = GLESMemoryAllocator::texture_get_handle(cmd.canvas_element.texture);

            i32 tex_unit = -1;
            for (i32 t = 0; t < data.canvas_element_batch.texture_index; t++)
            {
                if (data.canvas_element_batch.texture_units[t] == i32(tex))
                {
                    tex_unit = t;
                    break;
                }
            }

            if (tex_unit == -1)
            {
                tex_unit = data.canvas_element_batch.texture_index;
                data.canvas_element_batch.texture_units[data.canvas_element_batch.texture_index] = tex;
                data.canvas_element_batch.texture_index++;
            }

            u32 index = data.canvas_element_batch.count;

            data.canvas_element_batch.instances[index].transform_0 = cmd.canvas_element.transform[0];
            data.canvas_element_batch.instances[index].transform_1 = cmd.canvas_element.transform[1];
            data.canvas_element_batch.instances[index].transform_2 = cmd.canvas_element.transform[2];

            data.canvas_element_batch.instances[index].unit = tex_unit;
            data.canvas_element_batch.instances[index].flags = cmd.canvas_element.flags;

            data.canvas_element_batch.instances[index].texture_extent = cmd.canvas_element.texture_extent;
            data.canvas_element_batch.instances[index].dest_extent = cmd.canvas_element.dest_extent;

            data.canvas_element_batch.instances[index].src_rect = cmd.canvas_element.src_rect;
            data.canvas_element_batch.instances[index].color = cmd.canvas_element.color;

            data.canvas_element_batch.count++;
        }
        break;
        case RenderCommand::DRAW_QUAD:
        {
            if(data.quad_batch.count >= MaxInstancesPerBatch)
            {
                DebugInfo("Quad Batch full, flushing...");
                update_scene_uniform();
                end_quad_batch();
            }

            u32 index = data.quad_batch.count;
            
            data.quad_batch.instances[index].transform_0 = cmd.quad.transform[0];
            data.quad_batch.instances[index].transform_1 = cmd.quad.transform[1];
            data.quad_batch.instances[index].transform_2 = cmd.quad.transform[2];
            
            data.quad_batch.instances[index].size = cmd.quad.size;
            data.quad_batch.instances[index].color = cmd.quad.color;

            data.quad_batch.count++;
        }
            break;
        case RenderCommand::DRAW_LINE:
        {
            if(data.primitive_batch.count >= MaxPrimitivePointsPerBatch)
            {
                DebugInfo("Primitive Batch full, flushing...");
                update_scene_uniform();
                end_primitive_batch();
            }

            u32 index = data.primitive_batch.count;

            data.primitive_batch.primitives[index].point = cmd.line.start;
            data.primitive_batch.primitives[index].color = cmd.line.color;
            data.primitive_batch.primitives[index].flags = 0;

            data.primitive_batch.primitives[index+1].point = cmd.line.end;
            data.primitive_batch.primitives[index+1].color = cmd.line.color;
            data.primitive_batch.primitives[index+1].flags = 0;

            data.primitive_batch.count += 2;
        }
            break;
        case RenderCommand::SET_SCENE_TRANSFORM:
        {
            const Vector2 translation = cmd.transform[2] * -1;

            data.scene_data.scene_transform = Mat4(
                Vector4(1, 0, 0, 0),
                Vector4(0, 1, 0, 0),
                Vector4(0, 0, 1, 0),
                Vector4(translation, 0, 1)
            );

            data.scene_data_ubo_update = true;
        }
            break;
        default:
            break;
        }
    }

    update_scene_uniform();
    if(data.sprite_batch.count > 0)
    {
        end_sprite_batch();
    }

    if (data.canvas_element_batch.count > 0)
    {
        end_canvas_element_batch();
    }
    
    if(data.quad_batch.count > 0)
    {
        end_quad_batch();
    }

    if(data.primitive_batch.count > 0)
    {
        end_primitive_batch();
    }
  
    data.commands.clear();
}

