#pragma once
#include "collections/array.h"
#include "collections/string_view.h"
#include "gpu/gpu.h"
#include "log/log.h"


#define GPUDebugInfo(...) Log::debug("[GPU]: " __VA_ARGS__)

#if defined(BREAD_SHOW_DEBUG_INFO) && !defined(BREAD_ENABLE_GPU_DEBUG_LAYER)
#define BREAD_ENABLE_GPU_DEBUG_LAYER
#endif

#if defined(BREAD_ENABLE_GPU_DEBUG_LAYER)
struct ResourceInfo
{
	StringView resource_name;
	StringView free_func_name;
};

struct ResourceAllocator
{
	Array<u32> allocated;
	ResourceInfo info;

	void init(const mem::Allocator& allocator, const ResourceInfo& _info)
	{
		allocated = Array<u32>::with_size(allocator, 4);
		info = _info;
	}

	void destroy()
	{
		for(u32 id : allocated.iter())
		{
			GPUDebugInfo(
				"{}({}): Was not deallocated correctly, you may forget to call {}",
				info.resource_name, id, info.free_func_name
			);
		}

		allocated.destroy();
	}

	void add(u32 id)
	{
		(void)allocated.add(id);
	}

	void remove(u32 id)
	{
		allocated.remove(id);
	}
};

struct GPUDebugLayer
{
	Array<ResourceAllocator> resource_allocators;

	static constexpr ResourceInfo infos[] =
	{
		{
			.resource_name = "Unknown",
			.free_func_name = "<unknown>",
		},
		{
			.resource_name = "GPU::PhysicalDeviceID",
			.free_func_name = "<unknown>",
		},
		{
			.resource_name = "GPU::SurfaceID",
			.free_func_name = "GPU::surface_destroy",
		},
		{
			.resource_name = "GPU::DeviceID",
			.free_func_name = "GPU::device_destroy",
		},
		{
			.resource_name = "GPU::SwapChainID",
			.free_func_name = "GPU::swap_chain_destroy",
		},
		{
			.resource_name = "GPU::FenceID",
			.free_func_name = "GPU::fence_destroy",
		},
		{
			.resource_name = "GPU::SemaphoreID",
			.free_func_name = "GPU::semaphore_destroy",
		},
		{
			.resource_name = "GPU::QueueID",
			.free_func_name = "GPU::queue_destroy",
		},
		{
			.resource_name = "GPU::MemoryHeaID",
			.free_func_name = "GPU::memory_heap_destroy",
		},
		{
			.resource_name = "GPU::BufferID",
			.free_func_name = "GPU::buffer_destroy",
		},
		{
			.resource_name = "GPU::SamplerID",
			.free_func_name = "GPU::sampler_destroy",
		},
		{
			.resource_name = "GPU::TextureID",
			.free_func_name = "GPU::texture_destroy",
		},
		{
			.resource_name = "GPU::DescriptorSetLayoutID",
			.free_func_name = "GPU::descriptor_set_layout_destroy",
		},
		{
			.resource_name = "GPU::DescriptorPoolID",
			.free_func_name = "GPU::descriptor_pool_destroy",
		},
		{
			.resource_name = "GPU::DescriptorSetID",
			.free_func_name = "GPU::descriptor_set_free",
		},
		{
			.resource_name = "GPU::PipelineID",
			.free_func_name = "GPU::pipeline_destroy",
		},
		{
			.resource_name = "GPU::CommandPoolID",
			.free_func_name = "GPU::command_pool_destroy",
		},
		{
			.resource_name = "GPU::CommandBufferID",
			.free_func_name = "GPU::command_buffer_free",
		},
	};

	static constexpr usize GetAllocatorIndexByName(StringView resource_name)
	{
		for(usize i = 0; i < ArraySize(infos); i++)
		{
			if(infos[i].resource_name.ends_with(resource_name))
			{
				return i;
			}
		}

		DebugAssert(false, "unknown resource");
		return MaxValue<usize>;
	}

	template<typename ResourceID>
	static constexpr GPU::ObjectType GetObjectTypeByIDType()
	{
		if constexpr (IsSame<ResourceID, GPU::SurfaceID>)
		{
			return GPU::ObjectType::Surface;
		}
		else if constexpr (IsSame<ResourceID, GPU::DeviceID>)
		{
			return GPU::ObjectType::Device;
		}
		else if constexpr (IsSame<ResourceID, GPU::SwapChainID>)
		{
			return GPU::ObjectType::SwapChain;
		}
		else if constexpr (IsSame<ResourceID, GPU::FenceID>)
		{
			return GPU::ObjectType::Fence;
		}
		else if constexpr (IsSame<ResourceID, GPU::SemaphoreID>)
		{
			return GPU::ObjectType::Semaphore;
		}
		else if constexpr (IsSame<ResourceID, GPU::QueueID>)
		{
			return GPU::ObjectType::Queue;
		}
		else if constexpr (IsSame<ResourceID, GPU::MemoryHeapID>)
		{
			return GPU::ObjectType::MemoryHeap;
		}
		else if constexpr (IsSame<ResourceID, GPU::BufferID>)
		{
			return GPU::ObjectType::Buffer;
		}
		else if constexpr (IsSame<ResourceID, GPU::SamplerID>)
		{
			return GPU::ObjectType::Sampler;
		}
		else if constexpr (IsSame<ResourceID, GPU::TextureID>)
		{
			return GPU::ObjectType::Texture;
		}
		else if constexpr (IsSame<ResourceID, GPU::DescriptorSetLayoutID>)
		{
			return GPU::ObjectType::DescriptorSetLayout;
		}
		else if constexpr (IsSame<ResourceID, GPU::DescriptorPoolID>)
		{
			return GPU::ObjectType::DescriptorPool;
		}
		else if constexpr (IsSame<ResourceID, GPU::DescriptorSetID>)
		{
			return GPU::ObjectType::DescriptorSet;
		}
		else if constexpr (IsSame<ResourceID, GPU::PipelineID>)
		{
			return GPU::ObjectType::Pipeline;
		}
		else if constexpr (IsSame<ResourceID, GPU::CommandPoolID>)
		{
			return GPU::ObjectType::CommandPool;
		}
		else if constexpr (IsSame<ResourceID, GPU::CommandBufferID>)
		{
			return GPU::ObjectType::CommandBuffer;
		}
	}

	void init(const mem::Allocator& allocator)
	{
		resource_allocators = Array<ResourceAllocator>::with_size(allocator, usize(GPU::ObjectType::ObjectCount) + 1);

		for(usize i = 0; i < usize(GPU::ObjectType::ObjectCount) + 1; i++)
		{
			ResourceAllocator resource_allocator = {};
			resource_allocator.init(allocator, infos[i]);
			(void)resource_allocators.add(resource_allocator);
		}
	}

	void destroy()
	{
		for(ResourceAllocator& ra : resource_allocators.iter())
		{
			ra.destroy();
		}

		resource_allocators.destroy();
	}

	template<typename ResourceID>
	void add(ResourceID resource_id)
	{
		u32 as_integer = u32(resource_id.id);
		(void)resource_allocators.get(
			usize(GetObjectTypeByIDType<ResourceID>())
		).add(as_integer);
	}

	template<typename ResourceID>
	void remove(ResourceID resource_id)
	{
		u32 as_integer = u32(resource_id.id);
		(void)resource_allocators.get(
			usize(GetObjectTypeByIDType<ResourceID>())
		).remove(as_integer);
	}
};
static inline GPUDebugLayer gpu_debug_layer;

#define GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(x) \
	auto resource_id = x;\
	gpu_debug_layer.add(resource_id);\
	return resource_id;

#define GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(id, x) \
	x;\
	gpu_debug_layer.remove(id);
#else
#define GPU_DEBUG_LAYER_HANDLE_RESOURCE_ALLOCATION(x) return x
#define GPU_DEBUG_LAYER_HANDLE_RESOURCE_DEALLOCATION(id, x) x;
#endif