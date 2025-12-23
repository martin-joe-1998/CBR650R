#pragma once
#include "Engine/Application.h"

using namespace CBR::Engine;

namespace CBR::Game
{
	class CBRGame : public Application
	{
		
	protected:
		bool Initialize() override;
		void Shotdowm() override;
	};
};

