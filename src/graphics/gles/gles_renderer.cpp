#include "graphics/gles/gles_renderer.h"

#include "graphics/gles/gles_driver.h"
#include "graphics/gles/gles_memory_allocator.h"
#include "graphics/gles/gles_utility.h"
#include "graphics/gles/gles_vtable.h"
#include "graphics/viewport.h"
#include "math/projection.h"


static inline void _vertex_attrib_divisor(GLint index, GLenum type, GLsizei component_count, 
    GLsizei stride, GLsizei offset)
{
    gl.glVertexAttribPointer(index, component_count, type, GL_FALSE, stride, (const void*)u64(offset));
    gl.glEnableVertexAttribArray(index);
    gl.glVertexAttribDivisor(index, 1);
}

static inline void _vertex_attrib(GLint index, GLenum type, GLsizei component_count,
    GLsizei stride, GLsizei offset)
{
    gl.glVertexAttribPointer(index, component_count, type, GL_FALSE, stride, (const void*)u64(offset));
    gl.glEnableVertexAttribArray(index);
}

// Global quad index buffer
static constexpr const u8 indices[] =
{
    0, 1, 2, 2, 3, 0
};

void GLESRenderer::initialize(mem::Allocator allocator)
{
    data.allocator = allocator;

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
        for (u32 i = 0; i < SpriteInstanceAttribCount; i++)
        {
            _vertex_attrib_divisor(i, GL_FLOAT, 4, sizeof(SpriteInstance), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);
        
        data.sprite_batch.program = gles::compile_program("shaders/batch.gles.glsl", "#define SPRITE");
    }

    // UI sprite batch
    {
        data.ui_sprite_batch = {};

        gl.glGenVertexArrays(1, &data.ui_sprite_batch.vao);
        data.ui_sprite_batch.instance_buffer_object = GLESMemoryAllocator::buffer_allocate_handle();
        gl.glBindVertexArray(data.ui_sprite_batch.vao);

        GLESMemoryAllocator::buffer_fill_memory(
            data.ui_sprite_batch.instance_buffer_object, Slice<const u8>(nullptr, sizeof(UISpriteInstance) * MaxInstancesPerBatch),
            GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW
        );
        data.ui_sprite_batch.instances = allocator.array<UISpriteInstance>(MaxInstancesPerBatch);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.ui_sprite_batch.instance_buffer_object);
        for (u32 i = 0; i < CanvasElementInstanceAttribCount; i++)
        {
            _vertex_attrib_divisor(i, GL_FLOAT, 4, sizeof(UISpriteInstance), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);

        data.ui_sprite_batch.program = gles::compile_program("shaders/batch.gles.glsl", "#define SPRITE\n#define UI_SPRITE");
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
        
        data.quad_batch.program = gles::compile_program("shaders/batch.gles.glsl", "#define QUAD");
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

        data.primitive_batch.program = gles::compile_program("shaders/batch.gles.glsl", "#define PRIMITIVE");
    }

    // Primitive circle batch
    {
        data.primitive_circle_batch = {};

        gl.glGenVertexArrays(1, &data.primitive_circle_batch.vao);
        data.primitive_circle_batch.instance_buffer_object = GLESMemoryAllocator::buffer_allocate_handle();
        gl.glBindVertexArray(data.primitive_circle_batch.vao);

        GLESMemoryAllocator::buffer_fill_memory(
            data.primitive_circle_batch.instance_buffer_object,
            Slice<const u8>(nullptr, sizeof(PrimitiveCircle) * MaxPrimitiveCirclesPerBatch),
            GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW
        );
        data.primitive_circle_batch.primitives = data.allocator.array<PrimitiveCircle>(MaxPrimitiveCirclesPerBatch);

        gl.glBindBuffer(GL_ARRAY_BUFFER, data.primitive_circle_batch.instance_buffer_object);
        for (u32 i = 0; i < PrimitiveCircleAttribCount; i++)
        {
            _vertex_attrib_divisor(i, GL_FLOAT, 4, sizeof(PrimitiveCircle), i * sizeof(Vector4));
        }

        gl.glBindVertexArray(0);

        data.primitive_circle_batch.program = gles::compile_program("shaders/batch.gles.glsl", "#define CIRCLE");
    }

    data.scene_data_ubo = GLESMemoryAllocator::buffer_allocate_handle();
    GLESMemoryAllocator::buffer_fill_memory(
        data.scene_data_ubo, Slice<const u8>(nullptr, sizeof(SceneUniform)), GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW
    );
    
    data.scene_data.viewport_transform = Mat4::identity();
    data.scene_data.scene_transform = Mat4::identity();
    data.scene_data_ubo_update = true;
    
    data.state =
    {
        .frame_index = 0,
    };
}

void GLESRenderer::shutdown()
{
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
            data.ui_sprite_batch.instance_buffer_object, sizeof(UISpriteInstance) * MaxInstancesPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.ui_sprite_batch.vao);
        gl.glDeleteProgram(data.ui_sprite_batch.program);

        data.allocator.free(mem::to_bytes(data.ui_sprite_batch.instances));
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

    // Primitive point batch
    {
        GLESMemoryAllocator::buffer_deallocate_handle(
            data.primitive_batch.instance_buffer_object, sizeof(PrimitivePoint) * MaxPrimitivePointsPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.primitive_batch.vao);
        gl.glDeleteProgram(data.primitive_batch.program);

        data.allocator.free(mem::to_bytes(data.primitive_batch.primitives));
    }

    // Primitive circle batch
    {
        GLESMemoryAllocator::buffer_deallocate_handle(
            data.primitive_circle_batch.instance_buffer_object, sizeof(PrimitiveCircle) * MaxPrimitiveCirclesPerBatch
        );
        gl.glDeleteVertexArrays(1, &data.primitive_circle_batch.vao);
        gl.glDeleteProgram(data.primitive_circle_batch.program);

        data.allocator.free(mem::to_bytes(data.primitive_circle_batch.primitives));
    }

    GLESMemoryAllocator::buffer_deallocate_handle(data.scene_data_ubo, sizeof(SceneUniform));
}

void GLESRenderer::update_viewport_transform(const Vector2I& viewport_size)
{
    // Don't get confused, keep_viewport keep the screen_transform, so we are
    data.scene_data.viewport_transform = Projection::orthographic(
        0, f32(viewport_size.width), -f32(viewport_size.height), 0,
        1.f, -1.f
    );
    data.scene_data.viewport_transform.transpose();
    data.scene_data_ubo_update = true;
    update_scene_uniform();
}

void GLESRenderer::bind_program(GLID program)
{
    gl.glUseProgram(program);
}

void GLESRenderer::bind_scene_buffer()
{
    gl.glBindBufferBase(GL_UNIFORM_BUFFER, 0, data.scene_data_ubo);
}

void GLESRenderer::update_scene_uniform()
{
    if (!data.scene_data_ubo_update)
        return;

    data.scene_data_ubo_update = false;
    GLESMemoryAllocator::buffer_update_memory(
        data.scene_data_ubo, 0, Slice<const u8>((u8*)&data.scene_data, sizeof(SceneUniform)),
        GL_UNIFORM_BUFFER
    );
}

void GLESRenderer::end_sprite_batch()
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
        mem::to_const_bytes(data.sprite_batch.instances.slice(data.sprite_batch.count)), 
        GL_ARRAY_BUFFER,
        GLESMemoryAllocator::UMHWriteOnly
    );
    
    for(i32 i = 0; i < GLESRenderer::data.sprite_batch.texture_index; i++)
    {
        gl.glActiveTexture(GL_TEXTURE0 + i);
        gl.glBindTexture(GL_TEXTURE_2D, data.sprite_batch.texture_units[i]);
    }

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.sprite_batch.count);
    data.sprite_batch.count = 0;
    data.sprite_batch.texture_index = 0;
}

void GLESRenderer::end_ui_sprite_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.ui_sprite_batch.program);
    bind_scene_buffer();

    gl.glBindVertexArray(data.ui_sprite_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);
    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.ui_sprite_batch.instance_buffer_object, 0, 
        mem::to_const_bytes(data.ui_sprite_batch.instances.slice(data.ui_sprite_batch.count)),
        GL_ARRAY_BUFFER, 
        GLESMemoryAllocator::UMHWriteOnly
    );

    for (i32 i = 0; i < GLESRenderer::data.ui_sprite_batch.texture_index; i++)
    {
        gl.glActiveTexture(GL_TEXTURE0 + i);
        gl.glBindTexture(GL_TEXTURE_2D, data.ui_sprite_batch.texture_units[i]);
    }

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.ui_sprite_batch.count);
    data.ui_sprite_batch.count = 0;
    data.ui_sprite_batch.texture_index = 0;
}

void GLESRenderer::end_quad_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.quad_batch.program);
    bind_scene_buffer();
    
    gl.glBindVertexArray(data.quad_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);
    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.quad_batch.instance_buffer_object, 0,
        mem::to_const_bytes(data.quad_batch.instances.slice(data.quad_batch.count)), 
        GL_ARRAY_BUFFER,
        GLESMemoryAllocator::UMHWriteOnly
    );

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.quad_batch.count);
    data.quad_batch.count = 0;
}

void GLESRenderer::end_primitive_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.primitive_batch.program);
    bind_scene_buffer();

    gl.glBindVertexArray(data.primitive_batch.vao);
    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.primitive_batch.instance_buffer_object, 0,
        mem::to_const_bytes(data.primitive_batch.primitives.slice(data.primitive_batch.count)), 
        GL_ARRAY_BUFFER,
        GLESMemoryAllocator::UMHWriteOnly
    );

    gl.glDrawArrays(GL_LINES, 0, (GLsizei)data.primitive_batch.count);
    data.primitive_batch.count = 0;
}

void GLESRenderer::end_primitive_circle_batch()
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count++;
#endif

    bind_program(data.primitive_circle_batch.program);
    bind_scene_buffer();

    gl.glBindVertexArray(data.primitive_circle_batch.vao);
    gl.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.global_quad_ibo);
    GLESMemoryAllocator::buffer_bind_and_update_memory(
        data.primitive_circle_batch.instance_buffer_object, 0,
        mem::to_const_bytes(data.primitive_circle_batch.primitives.slice(data.primitive_circle_batch.count)),
        GL_ARRAY_BUFFER,
        GLESMemoryAllocator::UMHWriteOnly
    );

    gl.glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, nullptr, (GLsizei)data.primitive_circle_batch.count);
    data.primitive_circle_batch.count = 0;
}

void GLESRenderer::render(Viewport* viewport)
{
#if SHOW_DEBUG_INFO
    data.debug.draw_call_count = 0;
#endif

    RenderTargetID rt_id = viewport->rt.render_target_id;
	// Only bind framebuffer if not main render target
    if (rt_id != GLESDriver::get_main_render_target())
    {
        auto& rt = GLESMemoryAllocator::render_target_get(rt_id);
        gl.glBindFramebuffer(GL_FRAMEBUFFER, rt.framebuffer);
        gl.glViewport(0, 0, rt.size.width, rt.size.height);
    }

    const Transform2D scene_transform = viewport->get_scene_transform();
    const Vector2 translation = scene_transform.get_position() * -1;
    const Vector2 scale = scene_transform.get_scale();

    data.scene_data.scene_transform =
        Mat4::translation(Vector3(translation.x, translation.y, 0))
        * Mat4::scaling(Vector3(scale.x, scale.y, 1))
        * Mat4::rotation_z(math::degrees(scene_transform.get_rotation()));

    data.scene_data.scene_transform.transpose();
    data.scene_data_ubo_update = true;

    if (viewport->must_sync)
    {
        update_viewport_transform(viewport->viewport_size);
        viewport->must_sync = false;
    }
    update_scene_uniform();

    gl.glClearColor(
        f32(viewport->clear_color.r / 255.f),
        f32(viewport->clear_color.g / 255.f),
        f32(viewport->clear_color.b / 255.f),
        f32(viewport->clear_color.a / 255.f)
    );
    gl.glClear(GL_COLOR_BUFFER_BIT);


    auto ot = viewport->get_order_table();
    for (auto& layer : ot.layers)
    {
        for (auto item_id : layer.items.iter())
        {
            auto& item = viewport->items.get(item_id);
            _render_item_draw(item);
        }
    }

    if(data.sprite_batch.count > 0)
    {
        end_sprite_batch();
    }

    if(data.quad_batch.count > 0)
    {
        end_quad_batch();
    }

    if(data.primitive_batch.count > 0)
    {
        end_primitive_batch();
    }

    if (data.primitive_circle_batch.count > 0)
    {
        end_primitive_circle_batch();
    }

    if (data.ui_sprite_batch.count > 0)
    {
        end_ui_sprite_batch();
    }

    data.state.frame_index ^= 1;

	// Restoring viewport if not main render target
    if (rt_id != GLESDriver::get_main_render_target())
    {
        Vector2I last_viewport_size = GLESDriver::get_current_viewport_size();
        gl.glViewport(0, 0, last_viewport_size.width, last_viewport_size.height);
    }
}

void GLESRenderer::_render_item_draw(Viewport::RenderItem& item)
{
    Viewport::RenderItem::Command* cmd = item.begin();
    for (; cmd != item.end(); cmd = item.get_command_at(cmd->next))
    {
        switch (cmd->type)
        {
        case Viewport::RenderItem::CMD_RECT:
        {
            auto rect = reinterpret_cast<Viewport::RenderItem::CommandRect*>(cmd);
            if (data.quad_batch.count >= MaxInstancesPerBatch)
            {
                end_quad_batch();
            }

            u32 index = data.quad_batch.count;

            data.quad_batch.instances[index].transform_0 = rect->transform[0];
            data.quad_batch.instances[index].transform_1 = rect->transform[1];
            data.quad_batch.instances[index].transform_2 = rect->transform[2];

            data.quad_batch.instances[index].rect = rect->rect;
            data.quad_batch.instances[index].color = rect->color;

            data.quad_batch.count++;
        }
        break;
        case Viewport::RenderItem::CMD_LINE:
        {
            auto line = reinterpret_cast<Viewport::RenderItem::CommandLine*>(cmd);
            if (data.primitive_batch.count >= MaxPrimitivePointsPerBatch)
            {
                end_primitive_batch();
            }

            u32 index = data.primitive_batch.count;

            data.primitive_batch.primitives[index].point = line->point1;
            data.primitive_batch.primitives[index].color = line->color;
            data.primitive_batch.primitives[index].flags = 0;

            data.primitive_batch.primitives[index + 1].point = line->point2;
            data.primitive_batch.primitives[index + 1].color = line->color;
            data.primitive_batch.primitives[index + 1].flags = 0;

            data.primitive_batch.count += 2;
        }
        break;
        case Viewport::RenderItem::CMD_CIRCLE:
        {
            auto circle = reinterpret_cast<Viewport::RenderItem::CommandCircle*>(cmd);
            if (data.primitive_circle_batch.count >= MaxPrimitiveCirclesPerBatch)
            {
                end_primitive_circle_batch();
            }

            u32 index = data.primitive_circle_batch.count;

            data.primitive_circle_batch.primitives[index].point = circle->center;
            data.primitive_circle_batch.primitives[index].color = circle->color;
            data.primitive_circle_batch.primitives[index].radius = circle->radius;

            data.primitive_circle_batch.count++;
        }
        break;
        case Viewport::RenderItem::CMD_SPRITE:
        {
            auto sprite = reinterpret_cast<Viewport::RenderItem::CommandSprite*>(cmd);

            if (data.sprite_batch.count >= MaxInstancesPerBatch ||
                data.sprite_batch.texture_index >= data.usable_texture_units)
            {
                end_sprite_batch();
            }


            GLID tex = GLESMemoryAllocator::texture_get_handle(sprite->texture);

            GLID tex_unit = GLID(-1);
            for (i32 t = 0; t < data.sprite_batch.texture_index; t++)
            {
                if (data.sprite_batch.texture_units[t] == tex)
                {
                    tex_unit = t;
                    break;
                }
            }

            if (tex_unit == GLID(-1))
            {
                tex_unit = data.sprite_batch.texture_index;
                data.sprite_batch.texture_units[data.sprite_batch.texture_index] = tex;
                data.sprite_batch.texture_index++;
            }

            u32 index = data.sprite_batch.count;

            data.sprite_batch.instances[index].transform_0 = sprite->transform.get_column(0);
            data.sprite_batch.instances[index].transform_1 = sprite->transform.get_column(1);
            data.sprite_batch.instances[index].transform_2 = sprite->transform[2];

            data.sprite_batch.instances[index].unit = tex_unit;

            data.sprite_batch.instances[index].flags = 0;
            if (sprite->flags & Viewport::RENDER_FLAG_FLIP_H)
            {
                data.sprite_batch.instances[index].flags |= FLAG_FLIP_H;
            }

            if (sprite->flags & Viewport::RENDER_FLAG_FLIP_V)
            {
                data.sprite_batch.instances[index].flags |= FLAG_FLIP_V;
            }

            if (sprite->flags & Viewport::RENDER_FLAG_FONT_CHAR)
            {
                data.sprite_batch.instances[index].flags |= FLAG_FONT_CHAR;
            }

            Vector2 texture_extent = Vector2(
                GLESMemoryAllocator::texture_get_size(sprite->texture)
            );

            data.sprite_batch.instances[index].rect = sprite->rect;

            data.sprite_batch.instances[index].src_rect = Rect2D(
                sprite->src_rect.position / texture_extent,
                (sprite->src_rect.position + sprite->src_rect.size) / texture_extent
            );

            data.sprite_batch.instances[index].color = sprite->mod_color;

            data.sprite_batch.count++;
        }
        break;
        case Viewport::RenderItem::CMD_UI_SPRITE:
        {
            auto ui_sprite = reinterpret_cast<Viewport::RenderItem::CommandUISprite*>(cmd);

            if (data.ui_sprite_batch.count >= MaxInstancesPerBatch ||
                data.ui_sprite_batch.texture_index >= data.usable_texture_units)
            {
                end_ui_sprite_batch();
            }


            GLID tex = GLESMemoryAllocator::texture_get_handle(ui_sprite->texture);

            GLID tex_unit = GLID(-1);
            for (i32 t = 0; t < data.ui_sprite_batch.texture_index; t++)
            {
                if (data.ui_sprite_batch.texture_units[t] == tex)
                {
                    tex_unit = t;
                    break;
                }
            }

            if (tex_unit == GLID(-1))
            {
                tex_unit = data.ui_sprite_batch.texture_index;
                data.ui_sprite_batch.texture_units[data.ui_sprite_batch.texture_index] = tex;
                data.ui_sprite_batch.texture_index++;
            }

            u32 index = data.ui_sprite_batch.count;

            data.ui_sprite_batch.instances[index].transform_0 = ui_sprite->transform.get_column(0);
            data.ui_sprite_batch.instances[index].transform_1 = ui_sprite->transform.get_column(1);
            data.ui_sprite_batch.instances[index].transform_2 = ui_sprite->transform[2];

            data.ui_sprite_batch.instances[index].unit = tex_unit;

            data.ui_sprite_batch.instances[index].flags = 0;
            if (ui_sprite->flags & Viewport::RENDER_FLAG_FLIP_H)
            {
                data.ui_sprite_batch.instances[index].flags |= FLAG_FLIP_H;
            }

            if (ui_sprite->flags & Viewport::RENDER_FLAG_FLIP_V)
            {
                data.ui_sprite_batch.instances[index].flags |= FLAG_FLIP_V;
            }

            if (ui_sprite->flags & Viewport::RENDER_FLAG_FONT_CHAR)
            {
                data.ui_sprite_batch.instances[index].flags |= FLAG_FONT_CHAR;
            }

            Vector2 texture_extent = Vector2(
                GLESMemoryAllocator::texture_get_size(ui_sprite->texture)
            );

            data.ui_sprite_batch.instances[index].rect = ui_sprite->rect;

            data.ui_sprite_batch.instances[index].src_rect = Rect2D(
                ui_sprite->src_rect.position / texture_extent,
                (ui_sprite->src_rect.position + ui_sprite->src_rect.size) / texture_extent
            );

            data.ui_sprite_batch.instances[index].color = ui_sprite->mod_color;

            data.ui_sprite_batch.count++;
        }
        break;
        default:
            break;
        }
    }
}

