#pragma once
#include "object/object.h"
#include "render/render_manager.h"
#include "math/color.h"
#include "math/transform_2d.h"
#include "math/rect_2d.h"


struct Material;


/*
* Represent a object in a 2D world.
* It is affected by 2D cameras.
*/
struct Object2D : Object
{
    OBJECT(Object2D, Object);
    MARKS(MARK_2D);

    DefineVTable(Object)
    {
        Event<void(Object2D::*)(), false> transform_changed;
    };
    
    static void _bind_vtable(Object2D::VTable& vtable);

    struct InternalData
    {
        RenderItemID render_item;
        Material* material;
        RenderManager::RenderLayerMask render_layers;

        Transform2D transform{};
        Transform2D global_transform_cache{};
        Vector2 pos_cache = Vector2(0, 0);
        Vector2 scale_cache = Vector2(1, 1);
        // In Radians
        f32 rot_cache = 0.f;
    } data;
    
    void init(const CreateInfo&);
    void deinit();
    void enter();
    void exit();

    /*
    * Called when the transform of the object is modified.
    */
    void transform_changed() Function(FunctionPropagate);
    
    /*
    * @return The RenderItem ID owned by the object.
    */
    RenderItemID get_render_item() Function(FunctionNormal) 
    {
        return data.render_item;
    }

    void set_material(Material* new_material);
    Material* get_material();

    void set_render_layer(RenderManager::RenderLayerMask render_layer);
    RenderManager::RenderLayerMask get_render_layers();

    /*
    * Set the object current position.
    * 
    * @param new_pos The new object position
    */
    void set_position(const Vector2& new_pos) Function(FunctionNormal);

    /*
    * @return The object current position.
    */
    Vector2 get_position() const Function(FunctionNormal);

    /*
    * @return Translate the object by the given amount.
    */
    void translate(const Vector2& translation) Function(FunctionNormal);
    
    /*
    * Set the object current scale.
    * 
    * @param new_scale The new scale to apply.
    */
    void set_scale(const Vector2& new_scale);

    /*
    * @return The object current scale.
    */
    Vector2 get_scale() const;
    
    /*
    * Set the object current rotation.
    * 
    * @param new_rot Rotation in radias.
    */
    void set_rotation(const f32 new_rot);

    /*
    * @return Get object current rotation
    */
    [[nodiscard]] f32 get_rotation() const;

    /*
    * Rotate the object by the given amount.
    * 
    * @param rads Rotation in radians.
    */
    void rotate(const f32 rads);

    /*
    * @return The object transformation matrix.
    */
    Transform2D get_transform() const;
    
    /*
    * @return The object global transformation matrix.
    */
    Transform2D get_global_transform() const;

    /*
    * Draw a sprite using the object RenderItem ID.
    * 
    * @param transform The object transformation matrix.
    * @param texture The texture handle of the texture to render.
    * @param rect The sprite rectangle where it's going to be renderer based on the transformation position.
    * @param src_rect Source rect of the texture to draw.
    * @param mod_color Modulation color.
    * @param flags See Viewport::RenderFlags
    */
    void draw_sprite(const Transform2D& transform, Graphics::TextureID texture, const Rect2D& rect,
        const Rect2D& src_rect, Color mod_color, u32 flags);

    void _update_transform() Function(FunctionInternal);
    Transform2D _make_global_transform() const Function(FunctionInternal);
};
