#pragma once
#include <vector>
#include <memory>
#include <iostream>

#ifdef TESTDLL_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" DLL_API void TestFunc();