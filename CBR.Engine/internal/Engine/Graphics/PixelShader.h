#pragma once
#include "Engine/Graphics/ShaderKey.h"

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace Engine::Graphics
{
	class PixelShader
	{
	public:
	private:
		ComPtr<ID3D11PixelShader> pixelShader_;
		ComPtr<ID3DBlob> bytecode_;

		ShaderKey shaderKey_;
	};
}


