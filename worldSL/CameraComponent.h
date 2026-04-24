#pragma once
#include "object.h"

namespace Components {
	namespace Camera {
		enum class ProjectionType {
			Perspective,
			Orthographic
		};
	}

	class CameraComponent : public Components::Component
	{
	public:
		Camera::ProjectionType projectionType = Camera::ProjectionType::Orthographic;
		int size = 5;
		float nearClip = 0.3f, farClip = 1000.0f;

		float fieldOfView = 60;

		CameraComponent(World::Objects::Object* parent) : Component(parent) {}

		CameraComponent() : Component() {}

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

namespace World {
	static Components::CameraComponent* GetCurrentCamera() {
		// change to use a current cam pointer in the scene root object instead of searching for one every time
		World::Objects::Object* camObject = World::Objects::SceneRootObject::GetInstance().FindObjectWithComponent<Components::CameraComponent>(10);
		if (camObject) {
			return camObject->GetComponent<Components::CameraComponent>();
		}
		return nullptr;
	}
}

MAKE_FACTORY(CameraFactory, Components::CameraComponent)