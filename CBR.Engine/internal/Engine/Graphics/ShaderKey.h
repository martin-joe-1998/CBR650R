#pragma once
#include <string>

namespace Engine::Graphics
{
	enum class ShaderStage
	{
		VS,
		PS,
	};
	
	struct ShaderKey
	{
		std::string fileName;
		std::string entryPoint;
		ShaderStage shaderStage;
		std::string target;
		uint32_t compileFlags = 0;

		bool operator==(const ShaderKey& rhs) const
		{
			return fileName == rhs.fileName &&
				   entryPoint == rhs.entryPoint &&
				   target == rhs.target &&
				   compileFlags == rhs.compileFlags &&
				   shaderStage == rhs.shaderStage;
		}
	};

	struct ShaderKeyHash
	{
		size_t operator()(const ShaderKey& k) const
		{
			size_t h = 0;

			auto HashCombine = [&](size_t v)
			{
				h ^= v + 0x9e3779b9 + (h << 6) + (h >> 2);
			};

			HashCombine(std::hash<std::string>{}(k.fileName));
			HashCombine(std::hash<std::string>{}(k.entryPoint));
			HashCombine(std::hash<std::string>{}(k.target));
			HashCombine(std::hash<uint32_t>{}(k.compileFlags));
			HashCombine(std::hash<ShaderStage>{}(k.shaderStage));

			return h;
		}
	};
}