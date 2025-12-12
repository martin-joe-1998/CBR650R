#include "pch.h"
#include "Engine/WindowsMain.h"
#include "Game.h"

using namespace CBR;
using namespace CBR::Engine;

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    Application::RegisterInstance(new Game::CBRGame());
    
    return WindowsMain::Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}