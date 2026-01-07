/// Engine.pch ///
#pragma once

#include <Windows.h>
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <unordered_set>
#include <memory>
#include <cassert>

#include <string_view>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <source_location>
#include <mutex>
#include <ranges>

// Renderer
#include <wrl/client.h> // Microsoft::WRL::ComPtr
#include <wrl.h>
#include <d3d11.h>
// Properties->Librarian->Additional Dependencies
// #pragma comment(lib, "d3d11.lib")
// #pragma comment(lib, "dxgi.lib")
// #pragma comment(lib, "d3dcompiler.lib")
#include <d3d11_1.h>