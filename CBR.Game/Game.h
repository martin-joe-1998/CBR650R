#pragma once
#include "Engine/Application.h"

using namespace CBR::Engine;

namespace CBR::Game
{
	class CBRGame : public Application
	{
		bool Initialize() { return true; }
		void Shotdowm() {}
	};
};

