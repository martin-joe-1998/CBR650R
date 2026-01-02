/// Engine.pch ///
#pragma once

#include <Windows.h>
#include <cstdio>
#include <io.h>
#include <fcntl.h>
#include <iostream>
#include <unordered_set>
#include <memory>

#include <string_view>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <source_location>
#include <mutex>
#include <ranges>

// Renderer
#include <wrl/client.h> // Microsoft::WRL::ComPtr