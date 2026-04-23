#pragma once
#include <vector>
#include <string>
#include <iostream>

class A {
public:
	A() {};
	~A() {};

	int integer = 7;

	virtual void Speak() {
		std::cout << "I am class A" << std::endl;
	}
};

class B {
public:
	B() {};
	~B() {
		for (int i = 0; i < aList.size(); i++) {
			delete aList.at(i);
		}

		aList.clear();
	};

	std::vector<A*> aList;
	std::string stringy = "i am a string";
};