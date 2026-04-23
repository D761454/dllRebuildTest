#pragma once
#include <vector>
#include <memory>
#include <iostream>

#ifdef TESTDLL_EXPORTS 
// windows exclusive, dont need for linux when building engine dll for game as linux exports all - idk abt apple tho
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" DLL_API void TestFunc();