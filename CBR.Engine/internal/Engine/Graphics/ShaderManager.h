#pragma once
#include "Engine/Graphics/ShaderKey.h"
#include "Engine/Graphics/VertexShader.h"
#include "Engine/Graphics/PixelShader.h"

namespace Engine::Graphics
{
	class ShaderManager
	{
	public:
		static ShaderManager& Instance()
		{
			static ShaderManager instance;
			return instance;
		}

	private:
		ShaderManager() {}
		~ShaderManager() {}

		ShaderManager(const ShaderManager&) = delete;
		ShaderManager& operator=(const ShaderManager&) = delete;

		void InitializeImpl();
		void ShutdownImpl();

		std::unordered_map<ShaderKey, std::shared_ptr<VertexShader>> vsCache;
		std::unordered_map<ShaderKey, std::shared_ptr<PixelShader>> psCache;

		std::unordered_map<ShaderKey, std::shared_ptr<VertexShader>, ShaderKeyHash> vsCache;
		std::unordered_map<ShaderKey, std::shared_ptr<PixelShader>, ShaderKeyHash> psCache;
	};
}

