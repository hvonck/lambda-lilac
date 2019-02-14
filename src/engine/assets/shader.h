#pragma once
#include "assets/asset_handle.h"
#include <containers/containers.h>
#include <assets/shader_manager.h>
#include <platform/shader_variable.h>

namespace lambda
{
	namespace asset
	{
		///////////////////////////////////////////////////////////////////////////
		class Shader
		{
		public:
			Shader();
			Shader(const Shader& Shader);
			Shader(VioletShader Shader);
			~Shader();

			String getFilePath() const;
			Vector<uint32_t> getByteCode(ShaderStages stage, int type) const;

			void setShaderVariable(const platform::ShaderVariable& variable);
			Vector<platform::ShaderVariable> getQueuedShaderVariables();

		private:
			VioletShader shader_;
			Vector<platform::ShaderVariable> queued_shader_variables_;
		};
		typedef VioletHandle<Shader> VioletShaderHandle;

		///////////////////////////////////////////////////////////////////////////
		class ShaderManager
		{
		public:
			VioletShaderHandle create(Name name);
			VioletShaderHandle create(Name name, Shader Shader);
			VioletShaderHandle create(Name name, VioletShader Shader);
			VioletShaderHandle get(Name name);
			VioletShaderHandle get(uint64_t hash);
			void destroy(VioletShaderHandle Shader);

		public:
			static ShaderManager* getInstance();

		protected:
			VioletShaderManager& getManager();
			const VioletShaderManager& getManager() const;

		private:
			VioletShaderManager manager_;
		};
	}
}