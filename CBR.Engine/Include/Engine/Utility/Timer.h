#pragma once

namespace CBR::Engine::Utility
{
	class Timer
	{
	public:
		Timer();
		~Timer();

		Timer(const Timer&) = delete;
		Timer& operator= (const Timer&) = delete;

		void Tick();

		double RawDeltaTime() const { return rawDeltaTime_; }
		double DeltaTime() const { return deltaTime_; }
		double RawTotalTime() const { return rawTotalTime_; }
		double TotalTime() const { return totalTime_; }
		uint64_t FrameIndex() const { return frameIndex_; }

		void SetTimeScale(double s) { timeScale_ = s; }
		double TimeScale() const { return timeScale_; }

	private:
		// QPC
		int64_t engineStartCounter_ = 0;
		int64_t lastFrameCounter_ = 0;
		int64_t frequency_ = 0;				// 每秒多少个QPC tick

		static constexpr double FIXED_DELTA_TIME = 1.0 / 60.0;
		double rawDeltaTime_ = 0.0;
		double deltaTime_ = 0.0;
		double rawTotalTime_ = 0.0;			// 真实物理累计时间
		double totalTime_ = 0.0;			// 游戏内累计时间
		double timeScale_ = 1.0;
		
		uint64_t frameIndex_ = 0;
		double timeLag = 0;
	};
};

