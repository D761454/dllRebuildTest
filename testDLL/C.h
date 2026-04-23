#pragma once
#include "../testBuilder/Benz.h"

class C : public A {
public:
	C() : A() { integer = 2; };

	virtual void Speak() override {
		std::cout << "I am class C" << std::endl;
	}
};