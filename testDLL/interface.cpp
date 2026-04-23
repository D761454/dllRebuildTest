#include "interface.h"
#include <cstdio>
#include "C.h"

void TestFunc() {
	printf("base\n");
}

void ChangeALIST(B& obj) {
	delete obj.aList.back();
	obj.aList.clear();

	obj.aList.push_back(new C);
}

A* MakeC() {
	return new C;
}