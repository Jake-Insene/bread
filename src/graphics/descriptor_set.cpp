#include "graphics/descriptor_set.h"

#include "graphics/device.h"


namespace Graphics
{

void DescriptorSet::init(Mem::Allocator* _allocator, Device* _parent, const DescriptorSetInfo& info)
{
    DeviceObject::init(_allocator, _parent);

    Slice<GPU::DescriptorSetID> out_sets = Slice(&gpu_descriptor_set, 1);
    GPU::descriptor_set_allocate(
        {
            .device = info.gpu_device,
            .pool = info.gpu_descriptor_pool,
            .set_layouts = Slice(&info.gpu_descriptor_set_layout, 1),
        },
        out_sets
    );
    gpu_descriptor_pool = info.gpu_descriptor_pool;
    gpu_descriptor_set_layout = info.gpu_descriptor_set_layout;
}

void DescriptorSet::destroy()
{
    GPU::descriptor_set_free(gpu_descriptor_pool, Slice(&gpu_descriptor_set, 1));
}

}

