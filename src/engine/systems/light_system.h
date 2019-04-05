#pragma once
#include "interfaces/icomponent.h"
#include "interfaces/isystem.h"
#include "platform/shader_pass.h"
#include "utils/angle.h"
#include <glm/gtc/matrix_transform.hpp>
#include "platform/culling.h"

namespace lambda
{
  namespace components
  {
    class LightSystem;
    class TransformSystem;
    class MeshRenderSystem;
    class CameraSystem;

    enum class ShadowType
    {
      kNone,
      kDynamic,
      kGenerateOnce,
      kGenerated,
    };

    enum class LightType : unsigned char
    {
      kDirectional = 0,
      kPoint = 1,
      kSpot = 2,
      kCascade = 3,
      kUnknown = 255
    };

    class BaseLightComponent : public IComponent
    {
    public:
      BaseLightComponent(const entity::Entity& entity, LightSystem* system);
      BaseLightComponent(const BaseLightComponent& other);
      BaseLightComponent();

      void setShadowType(const ShadowType& shadow_type);
      ShadowType getShadowType() const;
      void setColour(const glm::vec3& colour);
      glm::vec3 getColour() const;
      void setAmbient(const glm::vec3& ambient);
      glm::vec3 getAmbient() const;
      void setIntensity(const float& intensity);
      float getIntensity() const;
	  void setShadowMapSizePx(uint32_t shadow_map_size_px);
	  uint32_t getShadowMapSizePx() const;
	  void setTexture(asset::VioletTextureHandle texture);
      asset::VioletTextureHandle getTexture() const;
      void setDepth(const float& depth);
      float getDepth() const;
      void setEnabled(const bool& enabled);
      bool getEnabled() const;
      void setDynamicFrequency(const uint8_t& frequency);
      uint8_t getDynamicFrequency() const;
      void setLightType(LightType type);
      LightType getLightType() const;
      void setRSM(bool rsm);
      bool getRSM() const;
      void setSize(const float& size);
      float getSize() const;
      void setCutOff(const utilities::Angle& cut_off);
      utilities::Angle getCutOff() const;
      void setOuterCutOff(const utilities::Angle& outer_cut_off);
      utilities::Angle getOuterCutOff() const;


    protected:
      LightSystem* system_;
    };

    class DirectionalLightComponent : public BaseLightComponent
    {
    public:
      DirectionalLightComponent(const entity::Entity& entity, LightSystem* system);
      DirectionalLightComponent(const DirectionalLightComponent& other);
      DirectionalLightComponent();
    };

    class PointLightComponent : public BaseLightComponent
    {
    public:
      PointLightComponent(const entity::Entity& entity, LightSystem* system);
      PointLightComponent(const PointLightComponent& other);
      PointLightComponent();
    };

    class SpotLightComponent : public BaseLightComponent
    {
    public:
      SpotLightComponent(const entity::Entity& entity, LightSystem* system);
      SpotLightComponent(const SpotLightComponent& other);
      SpotLightComponent();
    };

    struct LightData
    {
      LightData(const entity::Entity& entity) : entity(entity) {};
      LightData(const LightData& other);
      LightData& operator=(const LightData& other);

      LightType type = LightType::kUnknown;
      ShadowType shadow_type = ShadowType::kNone;
      glm::vec3 colour = glm::vec3(1.0f);
      glm::vec3 ambient = glm::vec3(0.0f);
      float intensity = 1.0f;
      asset::VioletTextureHandle texture;
      Vector<utilities::Culler> culler;
      entity::Entity    entity;
      utilities::Angle cut_off       = utilities::Angle::fromDeg(80.0f);
      utilities::Angle outer_cut_off = utilities::Angle::fromDeg(90.0f);
      float size  = 50.0f;
      bool enabled = true;
      bool rsm = false;
      uint8_t dynamic_frequency = 3u;
	  uint8_t dynamic_index = 254u;
	  bool valid = true;

      Vector<float>       depth;
      Vector<glm::mat4x4> projection;
      Vector<glm::mat4x4> view;
      Vector<glm::vec3>   view_position;
	  
	  // Shadow maps.
	  uint32_t shadow_map_size_px = 1024u;
	  Vector<platform::RenderTarget> render_target;
	  Vector<platform::RenderTarget> depth_target;
    };

    class LightSystem : public ISystem
    {
    public:
      ~LightSystem();

      static size_t systemId() { return (size_t)SystemIds::kLightSystem; };

    public:
      bool hasComponent(const entity::Entity& entity);
      void removeComponent(const entity::Entity& entity);

      BaseLightComponent addComponent(const entity::Entity& entity);
      DirectionalLightComponent addDirectionalLight(const entity::Entity& entity);
      PointLightComponent addPointLight(const entity::Entity& entity);
      SpotLightComponent addSpotLight(const entity::Entity& entity);
      DirectionalLightComponent addCascadedLight(const entity::Entity& entity);

      BaseLightComponent getComponent(const entity::Entity& entity);
      DirectionalLightComponent getDirectionalLight(const entity::Entity& entity);
      PointLightComponent getPointLight(const entity::Entity& entity);
      SpotLightComponent getSpotLight(const entity::Entity& entity);

      /*
      * Generate: [Input] Nothing [Output] Shadow map.
      * Modify:   [Input] Shadow map [Output] Shadow map.
	  * Modify Count: The amount of time the modify shader needs to be applied.
      * Publish:  [Input] Shadow map | Position | Normal | Metallic_Roughness [Output] Light map.
	  * Shadow Type: The type of shadows that should be used.
      */
	  void setShaders(String generate, String modify, uint32_t modify_count, String publish, String shadow_type);

      virtual void initialize(world::IWorld& world) override;
      virtual void deinitialize() override;
      virtual void onRender() override;
      virtual void collectGarbage() override;

      void setColour(const entity::Entity& entity, const glm::vec3& colour);
      glm::vec3 getColour(const entity::Entity& entity) const;
      void setAmbient(const entity::Entity& entity, const glm::vec3& ambient);
      glm::vec3 getAmbient(const entity::Entity& entity) const;
      void setIntensity(const entity::Entity& entity, const float& intensity);
      float getIntensity(const entity::Entity& entity) const;
      void setShadowType(const entity::Entity& entity, const ShadowType& shadow_type);
      ShadowType getShadowType(const entity::Entity& entity) const;
      void setShadowMapSizePx(const entity::Entity& entity, uint32_t shadow_map_size_px);
	  uint32_t getShadowMapSizePx(const entity::Entity& entity) const;
      void setCutOff(const entity::Entity& entity, const utilities::Angle& cut_off);
      utilities::Angle getCutOff(const entity::Entity& entity) const;
      void setOuterCutOff(const entity::Entity& entity, const utilities::Angle& outer_cut_off);
      utilities::Angle getOuterCutOff(const entity::Entity& entity) const;
      void setDepth(const entity::Entity& entity, const float& depth);
      float getDepth(const entity::Entity& entity) const;
      void setSize(const entity::Entity& entity, const float& size);
      float getSize(const entity::Entity& entity) const;
      void setTexture(const entity::Entity& entity, asset::VioletTextureHandle texture);
      asset::VioletTextureHandle getTexture(const entity::Entity& entity) const;
      void setEnabled(const entity::Entity& entity, const bool& enabled);
      bool getEnabled(const entity::Entity& entity) const;
      void setDynamicFrequency(const entity::Entity& entity, const uint8_t& frequency);
      uint8_t getDynamicFrequency(const entity::Entity& entity) const;
      void setRSM(const entity::Entity& entity, bool rsm);
      bool getRSM(const entity::Entity& entity) const;
      void setLightType(const entity::Entity& entity, LightType type);
      LightType getLightType(const entity::Entity& entity) const;

    private:
      LightData& lookUpData(const entity::Entity& entity);
      const LightData& lookUpData(const entity::Entity& entity) const;
	  void createShadowMaps(const entity::Entity& entity);

    private:
      void renderDirectional(const entity::Entity& entity);
      void renderSpot(const entity::Entity& entity);
      void renderPoint(const entity::Entity& entity);
      void renderCascade(const entity::Entity& entity);

    private:
      Vector<LightData> data_;
	  Map<entity::Entity, uint32_t> entity_to_data_;
	  Map<uint32_t, entity::Entity> data_to_entity_;
	  Set<entity::Entity> marked_for_delete_;
	  Queue<uint32_t> unused_data_entries_;

      asset::MeshHandle full_screen_mesh_;

	  String shader_generate_;
	  String shader_modify_;
	  uint32_t shader_modify_count_;
	  String shader_publish_;
	  String shader_shadow_type_;

      platform::RenderTarget default_shadow_map_;
      asset::VioletTextureHandle default_texture_;

      foundation::SharedPointer<TransformSystem> transform_system_;
      foundation::SharedPointer<MeshRenderSystem> mesh_render_system_;
      foundation::SharedPointer<CameraSystem> camera_system_;
      world::IWorld* world_;
    };
  }
}
