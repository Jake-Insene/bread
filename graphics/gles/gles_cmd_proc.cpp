#include "graphics/gles/gles_cmd_proc.h"

#include "debug/debug.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/gles/gles_utility.h"
#include "graphics/gles/gles_driver.h"
#include "graphics/gles/gles_shader.h"
#include "graphics/egl/egl.h"
#include "math/projection.h"

void GLESCommandProcessor::initialize(mem::Allocator allocator)
{
    data.allocator = allocator;

    data.commands = Array<RenderCommand>::with_allocator(
        data.allocator
    );

    // Global quad index buffer
    u8 indices[] = {
        0, 1, 2, 2, 3, 0
    };
    
    gl.glGenBuffers(1, &data.global_quad_ibo);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);
    gl.glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6, indices, GL_STATIC_DRAW);

    // Sprite_batch
    {
        data.sprite_batch = {};

        gl.glGenVertexArrays(1, &data.sprite_batch.vao);
        gl.glGenBuffers(1, &data.sprite_batch.instancebo);

        gl.glBindVertexArray(data.sprite_batch.vao);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.sprite_batch.instancebo);
        gl.glBufferData(GL_ARRAY_BUFFER, sizeof(SpriteInstance) * MaxInstancesPerBatch, nullptr, GL_DYNAMIC_DRAW);
        data.sprite_batch.instances = data.allocator.alloc(sizeof(SpriteInstance) * MaxInstancesPerBatch, alignof(SpriteInstance));
    
        for(u32 i = 0; i < SpriteInstanceAttribCount; i++)
        {
            gl.glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(SpriteInstance), (void*)u64(i*sizeof(Vector4)));
            gl.glEnableVertexAttribArray(i);
            gl.glVertexAttribDivisor(i, 1);
        }

        gl.glBindVertexArray(0);
        
        data.sprite_batch.program = gles::compile_program("sprite.glsl", {});
    }
    
    // Quad batch
    {
        data.quad_batch = {};

        gl.glGenVertexArrays(1, &data.quad_batch.vao);
        gl.glGenBuffers(1, &data.quad_batch.instancebo);

        gl.glBindVertexArray(data.quad_batch.vao);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.quad_batch.instancebo);
        gl.glBufferData(GL_ARRAY_BUFFER, sizeof(QuadInstance) * MaxInstancesPerBatch, nullptr, GL_DYNAMIC_DRAW);
        data.quad_batch.instances = data.allocator.alloc(sizeof(QuadInstance) * MaxInstancesPerBatch, alignof(QuadInstance));
        
        for(u32 i = 0; i < QuadInstanceAttribCount; i++)
        {
            gl.glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(QuadInstance), (void*)u64(i*sizeof(Vector4)));
            gl.glEnableVertexAttribArray(i);
            gl.glVertexAttribDivisor(i, 1);
        }

        gl.glBindVertexArray(0);
        
        data.quad_batch.program = gles::compile_program("primitive.glsl", "#define QUAD");
    }

    // Primitive batch
    {
        data.primitive_batch = {};

        gl.glGenVertexArrays(1, &data.primitive_batch.vao);
        gl.glGenBuffers(1, &data.primitive_batch.instancebo);

        gl.glBindVertexArray(data.primitive_batch.vao);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.primitive_batch.instancebo);
        gl.glBufferData(GL_ARRAY_BUFFER, sizeof(PrimitivePoint) * MaxInstancesPerBatch, nullptr, GL_DYNAMIC_DRAW);
        data.primitive_batch.primitives = data.allocator.alloc(sizeof(PrimitivePoint) * MaxPrimitivePointsPerBatch, alignof(PrimitivePoint));

        for(u32 i = 0; i < PrimitivePointAttribCount; i++)
        {
            gl.glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, sizeof(PrimitivePoint), (void*)u64(i*sizeof(Vector4)));
            gl.glEnableVertexAttribArray(i);
        }

        gl.glBindVertexArray(0);

        data.primitive_batch.program = gles::compile_program("primitive.glsl", "#define PRIMITIVE");
    }

    data.blit_program = gles::compile_program("blit.glsl", {});

    gl.glGenBuffers(1, &data.scene_data_ubo);
    gl.glBindBuffer(GL_UNIFORM_BUFFER, data.scene_data_ubo);
    gl.glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneUniform), nullptr, GL_STATIC_DRAW);
    
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

    gl.glDeleteBuffers(1, &data.global_quad_ibo);
    
    // Sprite batch
    {
        gl.glDeleteBuffers(1, &data.sprite_batch.instancebo);
        gl.glDeleteVertexArrays(1, &data.sprite_batch.vao);
        gl.glDeleteProgram(data.sprite_batch.program);
        
        data.allocator.free(data.sprite_batch.instances);
    }
    
    // Quad batch
    {
        gl.glDeleteBuffers(1, &data.quad_batch.instancebo);
        gl.glDeleteVertexArrays(1, &data.quad_batch.vao);
        gl.glDeleteProgram(data.quad_batch.program);
        
        data.allocator.free(data.quad_batch.instances);
    }

    // Primitive batch
    {
        gl.glDeleteBuffers(1, &data.primitive_batch.instancebo);
        gl.glDeleteVertexArrays(1, &data.primitive_batch.vao);
        gl.glDeleteProgram(data.primitive_batch.program);

        data.allocator.free(data.primitive_batch.primitives);
    }

    gl.glDeleteProgram(data.blit_program);
}


void GLESCommandProcessor::update_scene_uniform()
{
    if(data.scene_data_ubo_update)
    {
        gl.glBindBuffer(GL_UNIFORM_BUFFER, data.scene_data_ubo);
        gl.glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneUniform), &data.scene_data);
        data.scene_data_ubo_update = false;
    }
}

void GLESCommandProcessor::end_sprite_batch()
{
    gl.glUseProgram(data.sprite_batch.program);
    gl.glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.scene_data_ubo);

    gl.glBindVertexArray(data.sprite_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);
    gl.glBindBuffer(GL_ARRAY_BUFFER, data.sprite_batch.instancebo);
    gl.glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeof(SpriteInstance) * data.sprite_batch.count, data.sprite_batch.instances.ptr());

    for(u32 i = 0; i < GLESDriver::data.limits.max_texture_units; i++)
    {
        gl.glActiveTexture(GL_TEXTURE0 + i);
        gl.glBindTexture(GL_TEXTURE_2D, data.sprite_batch.texture_units[i]);
    }

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.sprite_batch.count);
    data.sprite_batch.count = 0;
    data.sprite_batch.texture_index = 0;
}

void GLESCommandProcessor::end_quad_batch()
{
    gl.glUseProgram(data.quad_batch.program);
    gl.glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.scene_data_ubo);

    gl.glBindVertexArray(data.quad_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);
    gl.glBindBuffer(GL_ARRAY_BUFFER, data.quad_batch.instancebo);
    gl.glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeof(QuadInstance) * data.quad_batch.count, data.quad_batch.instances.ptr());

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.quad_batch.count);
    data.quad_batch.count = 0;
}

void GLESCommandProcessor::end_primitive_batch()
{
    gl.glUseProgram(data.primitive_batch.program);
    gl.glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.scene_data_ubo);

    gl.glBindVertexArray(data.primitive_batch.vao);
    gl.glBindBuffer(GL_ARRAY_BUFFER, data.primitive_batch.instancebo);
    gl.glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)sizeof(PrimitivePoint) * data.primitive_batch.count, data.primitive_batch.primitives.ptr());

    gl.glDrawArrays(GL_LINES, 0, (GLsizei)data.primitive_batch.count);
    data.primitive_batch.count = 0;
}

void GLESCommandProcessor::render()
{
    //Debug::info("Processing commands: %llu", data.commands.count);
    for(usize i = 0; i < data.commands.count; i++)
    {
        RenderCommand& cmd = data.commands[i];
        switch(cmd.type)
        {
        case RenderCommand::BIND_RENDER_TARGET:
        {
            auto& rt = GLESDriver::render_targets_get(cmd.bind.source_id);
            data.scene_data.screen_transform = Projection::orthographic(0, (f32)rt.size.width, (f32)rt.size.height, 0, 1, -1);
            data.scene_data.screen_transform.transpose();

            data.scene_data_ubo_update = true;
           
            //Log::info("fbo: %i, current: %i", rt.gl_id, current_fbo);
            if(data.state.current_fbo != rt.gl_id)
            {
                data.state.current_fb = cmd.bind.source_id;
                gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.gl_id);
                gl.glViewport(0, 0, rt.size.width, rt.size.height);
                
                data.state.last_fbo = data.state.current_fbo;
                data.state.current_fbo = rt.gl_id;
            }
        }
            break;
        case RenderCommand::CLEAR_RENDER_TARGET:
        {
            if(data.state.current_fb != cmd.clear.rid)
            {
                auto& rt = GLESDriver::render_targets_get(cmd.bind.source_id);
                gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.gl_id);
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
            if(data.sprite_batch.count >= MaxInstancesPerBatch || data.sprite_batch.texture_index >= GLESDriver::data.limits.max_texture_units)
            {
                Debug::info("Sprite Batch full, flushing...");
                update_scene_uniform();
                end_sprite_batch();
            }
            
            auto& tex = GLESDriver::textures_get(cmd.sprite.texture);

            u32 texunit = u32(-1);
            for(u32 t = 0; t < GLESDriver::data.limits.max_texture_units; t++)
            {
                if(data.sprite_batch.texture_units[t] == tex.gl_id)
                {
                    texunit = t;
                    break;
                }
            }

            if(texunit == u32(-1))
            {
                texunit = data.sprite_batch.texture_index;
                data.sprite_batch.texture_units[data.sprite_batch.texture_index] = tex.gl_id;
                data.sprite_batch.texture_index++;
            }

            u32 index = data.sprite_batch.count;
            
            Slice<SpriteInstance> instances = mem::from_bytes<SpriteInstance>(data.sprite_batch.instances);
            instances[index].transform_0 = cmd.sprite.transform[0];
            instances[index].transform_1 = cmd.sprite.transform[1];
            
            instances[index].transform_2 = cmd.sprite.transform[2];
            instances[index].unit = texunit;
            instances[index].flags = cmd.sprite.flags;
            
            instances[index].texture_extent = cmd.sprite.texture_extent;
            instances[index].color = cmd.sprite.color;
            
            instances[index].src_rect = cmd.sprite.src_rect;

            data.sprite_batch.count++;
        }
            break;
        case RenderCommand::DRAW_QUAD:
        {
            if(data.quad_batch.count >= MaxInstancesPerBatch)
            {
                Debug::info("Quad Batch full, flushing...");
                update_scene_uniform();
                end_quad_batch();
            }

            u32 index = data.quad_batch.count;
            
            Slice<QuadInstance> instances = mem::from_bytes<QuadInstance>(data.quad_batch.instances);
            instances[index].transform_0 = cmd.quad.transform[0];
            instances[index].transform_1 = cmd.quad.transform[1];
            instances[index].transform_2 = cmd.quad.transform[2];
            
            instances[index].size = cmd.quad.size;
            instances[index].color = cmd.quad.color;

            data.quad_batch.count++;
        }
            break;
        case RenderCommand::DRAW_LINE:
        {
            if(data.primitive_batch.count >= MaxPrimitivePointsPerBatch)
            {
                Debug::info("Primitive Batch full, flushing...");
                update_scene_uniform();
                end_primitive_batch();
            }

            u32 index = data.primitive_batch.count;

            Slice<PrimitivePoint> primitives = mem::from_bytes<PrimitivePoint>(data.primitive_batch.primitives);
            primitives[index].point = cmd.line.start;
            primitives[index].color = cmd.line.color;
            primitives[index].flags = 0;

            primitives[index+1].point = cmd.line.end;
            primitives[index+1].color = cmd.line.color;
            primitives[index+1].flags = 0;

            data.primitive_batch.count += 2;
        }
            break;
        default:
            break;
        }
    }
    
    if(data.sprite_batch.count > 0)
    {
        update_scene_uniform();
        end_sprite_batch();
    }
    
    if(data.quad_batch.count > 0)
    {
        update_scene_uniform();
        end_quad_batch();
    }

    if(data.primitive_batch.count > 0)
    {
        update_scene_uniform();
        end_primitive_batch();
    }
  
    data.commands.clear();
}

