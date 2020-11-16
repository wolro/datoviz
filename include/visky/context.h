#ifndef VKL_CONTEXT_HEADER
#define VKL_CONTEXT_HEADER

#include "vklite2.h"



/*************************************************************************************************/
/*  Constants                                                                                    */
/*************************************************************************************************/

#define VKL_MAX_BUFFERS   16
#define VKL_MAX_COMPUTES  256
#define VKL_MAX_TEXTURES  256
#define VKL_MAX_TRANSFERS 256

#define VKL_DEFAULT_WIDTH  800
#define VKL_DEFAULT_HEIGHT 600

#define VKL_DEFAULT_BUFFER_STAGING_SIZE (16 * 1024 * 1024)
#define VKL_DEFAULT_BUFFER_VERTEX_SIZE  (16 * 1024 * 1024)
#define VKL_DEFAULT_BUFFER_INDEX_SIZE   (16 * 1024 * 1024)
#define VKL_DEFAULT_BUFFER_STORAGE_SIZE (16 * 1024 * 1024)
#define VKL_DEFAULT_BUFFER_UNIFORM_SIZE (4 * 1024 * 1024)



/*************************************************************************************************/
/*  Type definitions                                                                             */
/*************************************************************************************************/

typedef struct VklTexture VklTexture;
typedef struct VklTransfer VklTransfer;
typedef struct VklTransferFifo VklTransferFifo;
typedef struct VklTransferBuffer VklTransferBuffer;
typedef struct VklTransferTexture VklTransferTexture;
typedef union VklTransferUnion VklTransferUnion;



/*************************************************************************************************/
/*  Enums                                                                                      */
/*************************************************************************************************/

typedef enum
{
    VKL_CONTEXT_TRANSFER_SYNC,
    VKL_CONTEXT_TRANSFER_ASYNC,
} VklTransferMode;



typedef enum
{
    VKL_DEFAULT_BUFFER_STAGING,
    VKL_DEFAULT_BUFFER_VERTEX,
    VKL_DEFAULT_BUFFER_INDEX,
    VKL_DEFAULT_BUFFER_STORAGE,
    VKL_DEFAULT_BUFFER_UNIFORM,
    VKL_DEFAULT_BUFFER_COUNT,
} VklDefaultBuffer;



typedef enum
{
    VKL_DEFAULT_QUEUE_TRANSFER,
    VKL_DEFAULT_QUEUE_COMPUTE,
    VKL_DEFAULT_QUEUE_RENDER,
    VKL_DEFAULT_QUEUE_PRESENT,
    VKL_DEFAULT_QUEUE_COUNT,
} VklDefaultQueue;



typedef enum
{
    VKL_FILTER_MIN,
    VKL_FILTER_MAX,
} VklFilterType;



typedef enum
{
    VKL_TRANSFER_BUFFER_UPLOAD,
    VKL_TRANSFER_BUFFER_DOWNLOAD,
    VKL_TRANSFER_TEXTURE_UPLOAD,
    VKL_TRANSFER_TEXTURE_DOWNLOAD,
} VklDataTransferType;



/*************************************************************************************************/
/*  Transfer structs                                                                             */
/*************************************************************************************************/

struct VklTransferBuffer
{
    VklBufferRegions regions;
    VkDeviceSize offset, size;
    void* data;
};



struct VklTransferTexture
{
    VklTexture* texture;
    uvec3 offset, size;
    void* data;
};



union VklTransferUnion
{
    VklTransferBuffer buf;
    VklTransferTexture tex;
};



struct VklTransfer
{
    VklDataTransferType type;
    VklTransferUnion u;
};



struct VklTransferFifo
{
    uint32_t head, tail;
    VklTransfer transfers[VKL_MAX_TRANSFERS];
};



/*************************************************************************************************/
/*  Structs                                                                                      */
/*************************************************************************************************/

struct VklContext
{
    VklObject obj;
    VklGpu* gpu;

    VklTransferMode transfer_mode;
    VklCommands* transfer_cmd;
    VklTransferFifo transfer_fifo;

    uint32_t max_buffers;
    VklBuffer* buffers;
    VkDeviceSize* allocated_sizes; // for each buffer, how much is already allocated

    uint32_t max_images;
    VklImages* images;

    uint32_t max_samplers;
    VklSampler* samplers;

    uint32_t max_textures;
    VklTexture* textures;

    uint32_t max_computes;
    VklCompute* computes;
};



struct VklTexture
{
    VklObject obj;

    VklContext* context;

    VklImages* image;
    VklSampler* sampler;
};



/*************************************************************************************************/
/*  Context                                                                                      */
/*************************************************************************************************/

VKY_EXPORT VklContext* vkl_context(VklGpu* gpu);

VKY_EXPORT void vkl_context_transfer_mode(VklContext* context, VklTransferMode mode);



/*************************************************************************************************/
/*  Buffer allocation                                                                            */
/*************************************************************************************************/

VKY_EXPORT VklBufferRegions vkl_alloc_buffers(
    VklContext* context, uint32_t buffer_idx, uint32_t buffer_count, VkDeviceSize size);



/*************************************************************************************************/
/*  Compute                                                                                      */
/*************************************************************************************************/

VKY_EXPORT VklCompute* vkl_new_compute(VklContext* context, const char* shader_path);



/*************************************************************************************************/
/*  Texture                                                                                      */
/*************************************************************************************************/

VKY_EXPORT VklTexture*
vkl_new_texture(VklContext* context, uint32_t dims, uvec3 size, VkFormat format);

VKY_EXPORT void vkl_texture_resize(VklTexture* texture, uvec3 size);

VKY_EXPORT void vkl_texture_filter(VklTexture* texture, VklFilterType type, VkFilter filter);

VKY_EXPORT void vkl_texture_address_mode(
    VklTexture* texture, VklTextureAxis axis, VkSamplerAddressMode address_mode);

VKY_EXPORT void vkl_texture_destroy(VklTexture* texture);



/*************************************************************************************************/
/*  Data transfers                                                                               */
/*************************************************************************************************/

VKY_EXPORT void vkl_texture_upload_region(
    VklTexture* texture, uvec3 offset, uvec3 shape, //
    VkDeviceSize size, const void* data);

VKY_EXPORT void vkl_texture_upload(VklTexture* texture, VkDeviceSize size, const void* data);

VKY_EXPORT void
vkl_texture_download_region(VklTexture* texture, uvec3 offset, uvec3 shape, void* data);

VKY_EXPORT void vkl_texture_download(VklTexture* texture, void* data);

VKY_EXPORT void vkl_context_transfer(VklContext* context);



#endif
