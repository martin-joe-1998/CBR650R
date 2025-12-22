#include "pch.h"
#include "Engine/Debug/DebugManager.h"
#include "Engine/Debug/MemoryLT.h"

namespace CBR::Engine::Debug
{
    DebugManager::DebugManager()
        : logger_(Logger())
    {

    }

    DebugManager::~DebugManager()
    {
        
    }

    // Logger
    Logger& GetLogger() noexcept
    {
        return DebugManager::Instance().GetLogger();
    }

    // static
    bool DebugManager::Initialize()
	{
        static bool Initialized = false;
        if (Initialized)
            return false;

        Instance().InitializeImpl();

        // ²âÊÔÒ»ÏÂ
        LOG_INFO("INFO.");
        LOG_WARN("WARN.");
        LOG_ERROR("ERROR.");
        LOG_DEBUG("DEBUG.");

        Initialized = true;
        return Initialized;
	}

    // static
    void DebugManager::Shutdown()
    {
        Instance().ShutdownImpl();
    }


    void DebugManager::InitializeImpl()
    {
#if defined(_DEBUG) || defined(DEBUG)
        if (!memoryTrackingEnabled_)
        {
            // ¿ªÆôÄÚ´æÐ¹Â©¼ì²â
            mlt::Init(true, 256);
            memoryTrackingEnabled_ = true;
        }
#endif
    }

    void DebugManager::ShutdownImpl()
    {
        // ¹Ø±ÕÄÚ´æÐ¹Â©¼ì²â
        if (memoryTrackingEnabled_)
        {
            mlt::Close();

            memoryTrackingEnabled_ = false;
        }
    }
}