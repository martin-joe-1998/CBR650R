#include "pch.h"
#include "Engine/Utility/Timer.h"

namespace CBR::Engine::Utility
{
	Timer::Timer()
	{
		LARGE_INTEGER freq;
		QueryPerformanceFrequency(&freq);
		frequency_ = freq.QuadPart;

		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		engineStartCounter_ = now.QuadPart;
		lastFrameCounter_ = engineStartCounter_; // 避免第一帧dt过大
	}

	Timer::~Timer()
	{

	}

	void Timer::Tick()
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		int64_t deltaCounts = now.QuadPart - lastFrameCounter_;
		lastFrameCounter_ = now.QuadPart;

		rawDeltaTime_ = double(deltaCounts) / double(frequency_);
		rawTotalTime_ += rawDeltaTime_;
		deltaTime_ = rawDeltaTime_ * timeScale_;
		totalTime_ += deltaTime_;
		++frameIndex_;
	}
}
