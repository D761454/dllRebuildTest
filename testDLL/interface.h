#pragma once
#include <vector>
#include <memory>
#include <iostream>

#include "object.h"

#ifdef TESTDLL_EXPORTS
// windows exclusive, dont need for linux when building engine dll for game as linux exports all - idk abt apple tho
// with linux - can load .so at runtime same as dll - yay
// can compile a so by using -shared
// https://www.akkadia.org/drepper/dsohowto.pdf
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#define REG(comp, factory, list) if (std::find(list.begin(), list.end(), comp::GetType()) == list.end()) { factory t; sro->m_componentFactories.RegisterFactory(comp::GetType(), std::make_unique<factory>(t)); }

extern "C" DLL_API void TestFunc();

extern "C" DLL_API void Register(World::Objects::SceneRootObject* sro);