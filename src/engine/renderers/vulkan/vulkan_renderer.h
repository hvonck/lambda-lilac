#pragma once
#include "interfaces/irenderer.h"
#include "vulkan.h"
#include "vulkan_state_manager.h"

namespace lambda
{
  namespace linux
  {
	  class VulkanTexture;
	  class VulkanShader;
	  class VulkanRenderer;
	  class VulkanMesh;

	  ///////////////////////////////////////////////////////////////////////////
	  class VulkanRenderBuffer : public platform::IRenderBuffer
	  {
	  public:
		  VulkanRenderBuffer(
			  uint32_t size,
			  uint32_t flags,
			  VkBuffer buffer,
			  VulkanRenderer* renderer
		  );
		  virtual void*    lock()   override;
		  virtual void     unlock() override;
		  virtual uint32_t getFlags()  const override;
		  virtual uint32_t getSize()   const override;
		  VkBuffer         getBuffer() const;

	  private:
		  void* data_;
		  VulkanRenderer* renderer_;
		  VkBuffer buffer_;
		  uint32_t flags_;
		  uint32_t size_;
	  };

	  ///////////////////////////////////////////////////////////////////////////
	  class VulkanRenderTexture : public platform::IRenderTexture
	  {
	  public:
		  VulkanRenderTexture(
			  uint32_t width,
			  uint32_t height,
			  uint32_t depth,
			  uint32_t mip_count,
			  TextureFormat format,
			  uint32_t flags,
			  VulkanTexture* texture,
			  VulkanRenderer* renderer
		  );
		  virtual void*    lock(uint32_t level)   override;
		  virtual void     unlock(uint32_t level) override;
		  virtual uint32_t getWidth()    const override;
		  virtual uint32_t getHeight()   const override;
		  virtual uint32_t getDepth()    const override;
		  virtual uint32_t getMipCount() const override;
		  virtual uint32_t getFlags()    const override;
		  virtual TextureFormat getFormat() const override;
		  VulkanTexture* getTexture() const;

	  private:
		  void* data_;
		  VulkanRenderer* renderer_;
		  VulkanTexture* texture_;
		  uint32_t flags_;
		  uint32_t width_;
		  uint32_t height_;
		  uint32_t depth_;
		  uint32_t mip_count_;
		  TextureFormat format_;
	  };

	  struct VulkanCommandBuffer
	  {
	  public:
		  VulkanCommandBuffer();
		  void create(VkDevice device, VkQueue graphics_queue);
		  void destroy(VkDevice device);
		  void begin();
		  void end();
		  void tryBegin();
		  void tryEnd();
		  VkCommandBuffer getCommandBuffer() const;

	  private:
		  VkCommandBuffer command_buffer;
		  bool recording;
	  };

    ///////////////////////////////////////////////////////////////////////////
    class VulkanRenderer : public platform::IRenderer
    {
      struct MemoryStats
      {
        uint32_t vertex;
        uint32_t index;
        uint32_t constant;
        uint32_t texture;
        uint32_t render_target;
      } memory_stats_;
#define TO_MB(bytes) (std::round(bytes / 10000) / 100)

    public:
		platform::IRenderBuffer* allocRenderBuffer(
			uint32_t size,
			uint32_t flags,
			void* data = nullptr
		);
		void freeRenderBuffer(platform::IRenderBuffer* buffer);
		platform::IRenderTexture* allocRenderTexture(
			asset::VioletTextureHandle texture
		);
		void freeRenderTexture(platform::IRenderTexture* texture);

      VulkanRenderer();
      virtual ~VulkanRenderer();
      virtual void setWindow(
        foundation::SharedPointer<platform::IWindow> window
      ) override;
      virtual void initialize(world::IWorld* world) override;
      virtual void deinitialize() override;
      virtual void resize() override;
      virtual void update(const double& delta_time) override;
      virtual void startFrame() override;
      virtual void endFrame(bool display) override;

      /////////////////////////////////////////////////////////////////////////
      ///// Deferred Calls ////////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////
      virtual void draw() override;
      virtual void drawInstanced(const Vector<glm::mat4>& matrices) override;

      virtual void setRasterizerState(
        const platform::RasterizerState& rasterizer_state
      ) override;
      virtual void setBlendState(
        const platform::BlendState& blend_state
      ) override;
      virtual void setDepthStencilState(
        const platform::DepthStencilState& depth_stencil_state
      ) override;
      virtual void setSamplerState(
        const platform::SamplerState& sampler_state, 
        unsigned char slot
      ) override;

      virtual void generateMipMaps(
        const asset::VioletTextureHandle& texture
      ) override;
      virtual void copyToScreen(
        const asset::VioletTextureHandle& texture
      ) override;
      virtual void copyToTexture(
        const asset::VioletTextureHandle& src,
        const asset::VioletTextureHandle& dst
      ) override;
      virtual void bindShaderPass(
        const platform::ShaderPass& shader_pass
      ) override;
      virtual void clearRenderTarget(
        asset::VioletTextureHandle texture, 
        const glm::vec4& colour
      ) override;

	  virtual void setScissorRects(const Vector<glm::vec4>& rects) override;
	  virtual void setViewports(const Vector<glm::vec4>& rects) override;

      virtual void setMesh(asset::MeshHandle mesh) override;
      virtual void setSubMesh(const uint32_t& sub_mesh_idx) override;
      virtual void setShader(asset::VioletShaderHandle shader) override;
      virtual void setTexture(
        asset::VioletTextureHandle texture, 
        uint8_t slot = 0
      ) override;
	  virtual void setRenderTargets(
        Vector<asset::VioletTextureHandle> render_targets,
        asset::VioletTextureHandle depth_buffer
	  ) override;
	  void setRenderTargets(
		  Vector<VkImageView> render_targets,
		  VkImageView depth_buffer,
		  uint32_t width,
		  uint32_t height
	  );

      virtual void pushMarker(const String& name) override;
      virtual void setMarker(const String& name) override;
      virtual void popMarker() override;

      virtual void  beginTimer(const String& name) override;
      virtual void  endTimer(const String& name) override;
      virtual uint64_t getTimerMicroSeconds(const String& name) override;
      /////////////////////////////////////////////////////////////////////////
      ///// Deferred Calls End ////////////////////////////////////////////////
      /////////////////////////////////////////////////////////////////////////

      virtual void setRenderScale(const float& render_scale) override;
      virtual float getRenderScale() override;

      virtual void setVSync(bool vsync) override;
      virtual bool getVSync() const override;

      void setShaderVariable(
        const platform::ShaderVariable& variable
      ) override;

	  VkDevice getDevice() const;
	  VkCommandBuffer getCommandBuffer() const;

    protected:
      virtual void destroyAsset(
        foundation::SharedPointer<asset::IAsset> asset
      ) override;

	private:
		void createCommandBuffer();
		void createTest();

		void updateFramebuffer();
		Vector<VezAttachmentReference> getAttachmentReferences();

	private:
	  bool vsync_;
	  world::IWorld* world_;

	  VkInstance instance_;
	  VkPhysicalDevice physical_device_;
	  VkSurfaceKHR surface_;
	  VkDevice device_;
	  VezSwapchain swapchain_;

	  VkQueue graphics_queue_;
	  VulkanCommandBuffer command_buffer_;

	  struct State
	  {
		  uint16_t dirty_textures;
		  VulkanRenderTexture* textures[16];
		  bool dirty_shader;
		  VulkanShader* shader;
		  
		  bool dirty_framebuffer;
		  VkImageView render_targets[8];
		  VkImageView depth_target;
		  uint32_t rt_width;
		  uint32_t rt_height;
		  VezFramebuffer framebuffer;

		  VulkanMesh* mesh;
		  uint32_t sub_mesh_idx;
		  
		  glm::mat4x4 model;
		  glm::vec2 screen_size;
		  glm::vec2 metallic_roughness;
	  } state_;

	  double delta_time_;
	  double total_time_;
	  float  render_scale_;

	  struct Memory
	  {
		  template<typename T>
		  struct Entry
		  {
			  T* ptr = nullptr;
			  uint32_t hit_count = 0u;
			  bool keep_in_memory = false;
		  };

		  VulkanShader* getShader(asset::VioletShaderHandle handle);
		  VulkanRenderTexture* getTexture(asset::VioletTextureHandle handle);
		  void removeShader(asset::VioletShaderHandle handle);
		  void removeTexture(asset::VioletTextureHandle handle);
		  void removeShader(size_t handle);
		  void removeTexture(size_t handle);

		  void removeUnusedEntries();
		  void removeAllEntries();
		  UnorderedMap<size_t, Entry<VulkanShader>> shaders;
		  UnorderedMap<size_t, Entry<VulkanRenderTexture>> textures;
		  VulkanRenderer* renderer;
	  } memory_;

	  VulkanStateManager state_manager_;
	  struct VulkanFullScreenQuad
	  {
		  asset::MeshHandle mesh;
		  asset::VioletShaderHandle shader;
	  } full_screen_quad_;

	  struct Test
	  {
		  struct Pipeline
		  {
			  VezPipeline pipeline = VK_NULL_HANDLE;
			  std::vector<VkShaderModule> shader_modules;
		  } pipeline;

		  VkBuffer vertex_buffer = VK_NULL_HANDLE;
		  VkBuffer index_buffer = VK_NULL_HANDLE;
		  VkImage image = VK_NULL_HANDLE;
		  VkImageView image_view = VK_NULL_HANDLE;
		  VkSampler sampler = VK_NULL_HANDLE;
		  VkBuffer uniform_buffer = VK_NULL_HANDLE;
	  } test_;
    };
  }
}