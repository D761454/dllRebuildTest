#pragma once

#include "object.h"

namespace Components {
	class testcomponent : public Components::Component
	{
	public:
		testcomponent(World::Objects::Object* parent) : Component(parent) {}

		testcomponent() : Component() {}

		~testcomponent();

		virtual void ModuleAssignment() override;

		/// <summary>
		/// the main call used by the engine to run the component
		/// </summary>
		virtual void Update(float deltaTime) override;

		virtual bool IsOfType(const std::string type) const {
			return type == _type;
		}

		static std::string GetType() {
			return _type;
		}

		virtual const std::string GetInstanceType() override {
			return _type;
		}

		static bool InEngine() {
			return _inEngine;
		}

		virtual void Duplicate(Component* copyTo) override {
			// copy all component specific data to copyTo, may need to cast copyTo to this component type
		}
	private:
		static const std::string _type;
		static const bool _inEngine;
	};
} // Components

MAKE_FACTORY(testcomponentFactory, Components::testcomponent)
