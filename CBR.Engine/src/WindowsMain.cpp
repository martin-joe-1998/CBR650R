#include "pch.h"
#include "Engine/WindowsMain.h"
#include "Engine/GameEngine.h"
#include "Engine/Debug/Logger.h"

struct
{
	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
} state;

namespace CBR::Engine
{
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	int WindowsMain::Run(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
	{
		UNREFERENCED_PARAMETER(hPrevInstance);
		UNREFERENCED_PARAMETER(lpCmdLine);

		// 由于目前的管理关系是GameEngine->DebugManager->Logger，所以为了能尽早使用LOG，必须先初始化引擎...
		if (!GameEngine::Initialize())
		{
			GameEngine::Shutdown();
			return 0;
		}
		LOG_INFO("日本語出力テスト");
		if (FAILED(InitWindow(hInstance, nCmdShow)))
		{
			return 0;
		}

		// TODO: 放到Iteration
		bool running = true;
		while (running)
		{
			if (!ProcessMessages())
			{
				LOG_INFO("Close Window.");
				running = false;
			}

			if (!GameEngine::Iteration())
			{
				LOG_INFO("Engine iteration ends.");
				running = false;
			}
			// Render

			Sleep(10);
		}

		GameEngine::Shutdown();

		const wchar_t* CLASS_NAME = L"Window Test";

		UnregisterClass(CLASS_NAME, state.hInstance);

		// TODO
		return 0;
	}

	HRESULT WindowsMain::InitWindow(HINSTANCE hInstance, int nCmdShow)
	{
		// TODO: 保存这个名字
		const wchar_t* CLASS_NAME = L"Window Test";

		WNDCLASS wndClass = {};
		wndClass.lpszClassName = CLASS_NAME;
		wndClass.hInstance = hInstance;
		wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.lpfnWndProc = WindowProc;

		if (!RegisterClass(&wndClass))
		{
			return E_FAIL;
		}

		state.hInstance = hInstance;

		// 目前是无法自由改变大小但可以拖拽窗口
		DWORD style = /*WS_OVERLAPPEDWINDOW*/WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		int width = 640;
		int height = 480;

		RECT rect;
		rect.left = 250;
		rect.top = 250;
		rect.right = rect.left + width;
		rect.bottom = rect.top + height;

		AdjustWindowRect(&rect, style, false);

		state.hWnd = CreateWindowEx(
			0,
			CLASS_NAME,
			L"Title",
			style,
			rect.left,
			rect.top,
			rect.right - rect.left,
			rect.bottom - rect.top,
			NULL,
			NULL,
			hInstance,
			NULL
		);

		if (!state.hWnd)
		{
			return E_FAIL;
		}

		ShowWindow(state.hWnd, nCmdShow);
		LOG_INFO("Successfully Created Window.");

		return S_OK;
	}

	bool WindowsMain::ProcessMessages()
	{
		MSG msg = {};

		while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				return false;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return true;
	}
};