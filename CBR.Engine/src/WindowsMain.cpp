#include "pch.h"
#include "Engine/WindowsMain.h"
#include "Engine/GameEngine.h"
#include "Engine/Debug/Logger.h"

struct
{
	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
	bool bMinimized = false; // 窗口是否最小化
	bool bResizing = false;  // 窗口尺寸是否变化
	bool bActivated = true;  // 游戏是否处于活跃状态
	bool bClosing = false;   // 游戏是否要关闭
	const wchar_t* applicationName = NULL;
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

		if (FAILED(InitWindow(hInstance, nCmdShow)))
		{
			return 0;
		}

		bool running = true;
		MSG msg = {};
		while (msg.message != WM_QUIT)
		{
			// 当有消息时优先处理消息
			if (PeekMessage(
				&msg, 	   /* lpMsg         */
				nullptr,   /* hWnd          */
				0u, 	   /* wMsgFilterMin */
				0u, 	   /* wMsgFilterMax */
				PM_REMOVE  /* wRemoveMsg    */
			))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// 只有消息列表为空时才执行游戏主循环
			else if (running) {
				if (!GameEngine::Iteration())
				{
					break;
				}
			}
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
			state.applicationName ? state.applicationName : L"Title",
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
		while (msg.message != WM_QUIT)
		{
			// 当有消息时优先处理消息
			if(PeekMessage(
				&msg, 	   /* lpMsg         */
				nullptr,   /* hWnd          */
				0u, 	   /* wMsgFilterMin */
				0u, 	   /* wMsgFilterMax */
				PM_REMOVE  /* wRemoveMsg    */
			))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		return true;
	}
};