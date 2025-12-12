#include "pch.h"
#include "Engine/Application.h"

using namespace CBR::Engine;

static Application* Instance_ = nullptr;

Application* Application::GetInstance()
{
	return Instance_;
}

void Application::DestroyInstance()
{
    delete Instance_;
    Instance_ = nullptr;
}

void Application::RegisterInstance(Application* pApplication)
{
    Instance_ = pApplication;
}