#include "pch.h"
#include "Application.h"

using namespace CBR;

static Application* instance = nullptr;

Application* Application::GetInstance()
{
	return instance;
}

void Application::DestroyInstance()
{
    delete instance;
    instance = nullptr;
}

void Application::RegisterInstance(Application* pApplication)
{
    instance = pApplication;
}