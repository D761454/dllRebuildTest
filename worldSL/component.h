#pragma once

#define COMPONENT_SETUP(T, N, IE) \
	const std::string T::_type = N;\
	const bool T::_inEngine = IE;\

#include <string>
#include <unordered_map>
#include <memory>

namespace World {
	namespace Objects {
		/// <summary>
		/// Class used by all modifiable objects within a scene
		/// </summary>
		class Object;
	} // Objects
} // World

namespace Components {
	struct Handle {
		uint32_t index = UINT32_MAX;
		uint32_t generation = 0;
	};

	struct Slot {
		uint32_t generation = 0;
		bool active = false;
	};

	/// <summary>
	/// base class for all components that can be attached to game objects to modify / add functionality to an object
	/// </summary>
	class Component
	{
	public:
		World::Objects::Object* m_gameObject = nullptr;
		int m_ID;
		bool m_active = true;

		Component(World::Objects::Object* parent) : m_gameObject(parent) {
			m_ID = reinterpret_cast<uint64_t>(this);
			ModuleAssignment();
		}

		Component() {
			m_ID = reinterpret_cast<uint64_t>(this);
		}

		virtual ~Component() {
			m_gameObject = nullptr;
		}

		virtual void ModuleAssignment() {}

		virtual void Update(float deltaTime) {}

		/// <summary>
		/// method to compare the _type values of 2 components,
		/// used to verify type on both get and add components - ensuring component is present and correct
		/// </summary>
		/// <param name="type"></param>
		/// <returns></returns>
		virtual bool IsOfType(const std::string type) const {
			return type == _type;
		}

		/// <summary>
		/// returns the type via a static call - used within adding / getting / removing components
		/// </summary>
		/// <returns></returns>
		static std::string GetType() {
			return _type;
		}

		/// <summary>
		/// returns the type from a component instance, may be removed as save is handled within components
		/// </summary>
		/// <returns></returns>
		virtual const std::string GetInstanceType() {
			return _type;
		}

		/// <summary>
		/// returns if a component is an in engine type,
		/// preventing multiples per object
		/// </summary>
		/// <returns></returns>
		static bool InEngine() {
			return _inEngine;
		}

		//void SetHandle(Handle* h) { _handle = h; }

		//Handle* GetHandle() { return _handle; }

		/// <summary>
		/// takes in a pointer to a component and should copy all local variables to that game object
		/// </summary>
		/// <param name="copyTo"></param>
		virtual void Duplicate(Component* copyTo) {
			// copy all component specific data to copyTo, may need to cast copyTo to this component type
		}

	private:
		static const std::string _type;
		static const bool _inEngine;
		//Handle* _handle = nullptr;
	};

	/// <summary>
	/// registered to a factory holder, takes in an object and will attach a component to it during loading a scene
	/// </summary>
	class ComponentFactory {
	public:
		virtual ~ComponentFactory() {}
		virtual void Create(World::Objects::Object* parent) = 0;
	};

	/// <summary>
	/// holds factories for all available components in engine to then call create on load
	/// </summary>
	class ComponentTypeFactory {
	public:
		// receive a key and factory to add to the map, allowing the load function to load appropariate components for objects without needing to include other modules
		void RegisterFactory(std::string componentType, std::unique_ptr<ComponentFactory> componentFactory) {
			_factories.insert(std::pair<std::string, std::unique_ptr<ComponentFactory>>(componentType, std::move(componentFactory)));
		}

		void Create(std::string componentType, World::Objects::Object* parent) {
			_factories.at(componentType)->Create(parent);
		}

		std::vector<std::string> GetTypes() {
			std::vector<std::string> result;

			for (auto& item : _factories) {
				result.push_back(item.first);
			}
			return result;
		}

	private:
		std::unordered_map<std::string, std::unique_ptr<ComponentFactory>> _factories;
	};
} // Components

#define MAKE_FACTORY(CN, T) class CN : public Components::ComponentFactory { public: virtual void Create(World::Objects::Object* parent) override { parent->AddComponent<T>(); } };

#define REGISTER_FACTORY(CN, T) CN t; World::Objects::SceneRootObject::GetInstance().m_componentFactories.RegisterFactory(T::GetType(), std::make_unique<CN>(t));