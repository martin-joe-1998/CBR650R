#include "pch.h"
#include "Engine/WindowsMain.h"
#include "Engine/GameEngine.h"
#include "Engine/Debug/Logger.h"

constexpr uint32_t DefaultScreenWidth = 800;
constexpr uint32_t DefaultScreenHeight = 600;
constexpr RECT DefaultWindowRect = { 0, 0, DefaultScreenWidth, DefaultScreenHeight }; // left, top, right, bottom

// 窗口状态
struct
{
	HINSTANCE hInstance = nullptr;
	HWND hWnd = nullptr;
	bool bMinimized = false; // 窗口是否最小化
	bool bResizing = false;  // 窗口尺寸是否变化
	bool bActivated = true;  // 窗口是否处于活跃状态
	bool bClosing = false;   // 窗口是否要关闭
	UINT dpi = 96;
	float dpiScale = 1.0;
	const wchar_t* applicationName = NULL;
	const wchar_t* className = NULL;
	std::unordered_set<CBR::Engine::WindowsMain::WinProcDelegate> eventOnWndProc;
} state;

namespace CBR::Engine
{
	void WindowsMain::RegisterWndProc(WinProcDelegate pDelegate)
	{
		state.eventOnWndProc.insert(pDelegate);
	}

	void WindowsMain::UnregisterWndProc(WinProcDelegate pDelegate)
	{
		state.eventOnWndProc.erase(pDelegate);
	}
	
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		// delegate机制来让消息管线变得更灵活。很多系统都想知道窗口消息（input，UI，游戏本身逻辑），这种可插拔系统可以让下方的switch不那么冗长难以维护
		for (const auto& delegate : state.eventOnWndProc)
		{
			LRESULT result = delegate(hWnd, uMsg, wParam, lParam);
			if (FAILED(result))
				LOG_ERROR("WindowProc delegate returned an error: 0x{:X}", result);
				DebugBreak();
		}
		
		// TODO: 增加更多处理
		switch (uMsg)
		{
		case WM_CLOSE:
		{
			state.bClosing = true;
			DestroyWindow(hWnd); /// TODO:摧毁窗口的处理放到别处，这里只设置状态
		} break;
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;
		case WM_SIZE: // 当窗口尺寸改变时
		{
			if (wParam == SIZE_MINIMIZED) // 最小化窗口
			{
				state.bMinimized = true;
			}
			else if (state.bMinimized) // 从最小化到前台或别的什么
			{
				state.bMinimized = false;
			}
			/// TODO
			//else if (Engine::IsInitialized()) // 如引擎已经初始化，则再次获取窗口大小并重绘
		} break;
		case WM_ACTIVATEAPP:
		{
			state.bActivated = (wParam != 0);
		} break;
		case WM_KILLFOCUS: // 失去键盘焦点（Alt+Tab切走，点到别的窗口，弹出系统对话框，窗口最小化）时触发。清空输入方式输入卡死
		{
			/// TODO
			// Input::ClearAll // 清空键盘/鼠标/手柄状态
		} break;
		case WM_DPICHANGED: // 屏幕DPI发生变化时触发
		{
			UINT dpi = HIWORD(wParam);

			state.dpi = dpi;
			state.dpiScale = dpi / 96.0f; // 96是Windows规定的1.0倍缩放的基准DPI

			RECT* suggested = reinterpret_cast<RECT*>(lParam);
			SetWindowPos(
				hWnd,
				nullptr,
				suggested->left,
				suggested->top,
				suggested->right - suggested->left,
				suggested->bottom - suggested->top,
				SWP_NOZORDER | SWP_NOACTIVATE
			);

			/// 未来：通知UI系统重新调整大小等UI::OnDpiChanged(scale)，重新创建swapchain，更新字体atlas(ImGUI必做)
		} break;
		case WM_INPUT: // 注册了Raw Input (RegisterRawInputDevices(...))后，Windows会在鼠标移动，鼠标按键，键盘输入时发送该消息
		{
			//UINT size = 0;
			//GetRawInputData((HRAWINPUT)lParam, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER));
			//
			//std::vector<BYTE> buffer(size);
			//if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
			//	break;
			//
			//RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(buffer.data());
			//
			//if (raw->header.dwType == RIM_TYPEMOUSE)
			//{
			//	Input::OnRawMouse(
			//		raw->data.mouse.lLastX,
			//		raw->data.mouse.lLastY
			//	);
			//}
			//else if (raw->header.dwType == RIM_TYPEKEYBOARD)
			//{
			//	Input::OnRawKey(
			//		raw->data.keyboard.VKey,
			//		raw->data.keyboard.Flags
			//	);
			//}
		} 
		break;
		default:
		{
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		}

		return 0;
	}

	int WindowsMain::Run(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
	{
		UNREFERENCED_PARAMETER(hPrevInstance);
		UNREFERENCED_PARAMETER(lpCmdLine);

		if (FAILED(InitWindow(hInstance, nCmdShow)))
		{
			return 0;
		}

		// 由于目前的管理关系是GameEngine->DebugManager->Logger，所以为了能尽早使用LOG，必须先初始化引擎...
		if (!GameEngine::Initialize())
		{
			GameEngine::Shutdown();
			return 0;
		}

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
			else if (!state.bResizing && !state.bMinimized && /*state.bActivated*/IsWindowVisible(state.hWnd)) 
			{
				if (!GameEngine::Iteration())
				{
					break;
				}
			}
		}

		GameEngine::Shutdown();

		UnregisterClass(state.className, state.hInstance);

		// 让程序的退出码由PostQuitMessage决定，便于外部调试
		return static_cast<int>(msg.wParam);
	}

	HRESULT WindowsMain::InitWindow(HINSTANCE hInstance, int nCmdShow)
	{
		state.className = L"Window Test";

		WNDCLASSEX wcex
		{
			.cbSize = sizeof(WNDCLASSEX),
			.style = CS_HREDRAW | CS_VREDRAW,
			.lpfnWndProc = WindowProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = hInstance,
			.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION)),
			.hCursor = LoadCursor(nullptr, IDC_ARROW),
			.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1),
			.lpszMenuName = nullptr,
			.lpszClassName = state.className,
			.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION)),
		};

		if (!RegisterClassEx(&wcex))
		{
			return E_FAIL;
		}

		state.hInstance = hInstance;

		// 目前是无法自由改变大小但可以拖拽窗口
		DWORD style = /*WS_OVERLAPPEDWINDOW*/WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;
		RECT rect = DefaultWindowRect;

		AdjustWindowRect(&rect, style, FALSE);

		int windowWidth = rect.right - rect.left;
		int windowHeight = rect.bottom - rect.top;

		// 获取屏幕尺寸
		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		// 计算窗口中心位置
		int x = (screenWidth - windowWidth) / 2;
		int y = (screenHeight - windowHeight) / 2;

		state.hWnd = CreateWindowEx(
			0,
			state.className,
			state.applicationName ? state.applicationName : L"Title",
			style,
			x,
			y,
			windowWidth,
			windowHeight,
			nullptr,
			nullptr,
			hInstance,
			nullptr
		);

		if (!state.hWnd)
		{
			return E_FAIL;
		}

		ShowWindow(state.hWnd, nCmdShow);
		//LOG_INFO("Successfully Created Window.");

		return S_OK;
	}

	HWND WindowsMain::GetMainWindowHandle()
	{
		return state.hWnd;
	}

	RECT WindowsMain::GetDefaultWindowRect()
	{
		return DefaultWindowRect;
	}

	uint32_t WindowsMain::GetDefaultScreenWidth()
	{
		return DefaultScreenWidth;
	}

	uint32_t WindowsMain::GetDefaultScreenHeight()
	{
		return DefaultScreenHeight;
	}
};