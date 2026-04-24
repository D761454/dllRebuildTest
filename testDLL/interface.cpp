#include "interface.h"
#include <cstdio>

#include "testcomponent.h"

void TestFunc() {
	printf("base\n");
}

void Register(World::Objects::SceneRootObject* sro) {
	std::vector<std::string> faclist = sro->m_componentFactories.GetTypes();

	REG(Components::testcomponent, testcomponentFactory, faclist)
}
