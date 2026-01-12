#pragma once
#include "Engine/Graphics/ShaderKey.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace Engine::Graphics
{
	class VertexShader
	{
	public:
	private:
		ComPtr<ID3D11VertexShader> vertexShader_;
		ComPtr<ID3DBlob> bytecode_;

		ShaderKey shaderKey_;
	};
}


