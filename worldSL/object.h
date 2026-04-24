#pragma once

#include <iostream>

#include "component.h"

namespace World {
	namespace Objects {
		class Object
		{
		public:
			std::string m_name;

			Object(std::string name) : m_name(name) {

			};

			~Object() {
				_components.clear();
				_children.clear();

				/*if (_gameObject) {
					if (SceneRootObject* sr = dynamic_cast<SceneRootObject*>(_gameObject))
					{
						sr->RemoveChild(*this);
					}
					else if (GameObject* go = dynamic_cast<GameObject*>(_gameObject))
					{
						go->RemoveChild(*this);
					}
				}*/

				_gameObject = nullptr;
			}

			/// <summary>
			/// Returns first component matching type
			/// </summary>
			/// <typeparam name="type">checked if is derived from component, if so, then checked if component can be converted to type, if not, it must be derived, thus enable</typeparam>
			/// <returns></returns>
			template <std::derived_from<Components::Component> type, typename = std::enable_if_t<!std::is_nothrow_convertible_v<Components::Component, type>>>
			type* GetComponent() {
				for (auto& component : _components) {
					if (component->IsOfType(type::GetType())) {
						return static_cast<type*>(component.get());
					}
				}

				return nullptr;
			}

			std::vector<std::unique_ptr<Components::Component>>* GetAllComponents() {
				return &_components;
			}

			/// <summary>
			/// Adds a component of type to the _components list.
			/// If the passed component is an engine component e.g. Rigidbody, and the object allready has a component of the same type, it will not be added.
			/// </summary>
			/// <typeparam name="type"></typeparam>
			template <std::derived_from<Components::Component> type, typename = std::enable_if_t<!std::is_nothrow_convertible_v<Components::Component, type>>>
			type* AddComponent() {
				if (type::InEngine()) {
					for (auto& component : _components) {
						if (component->IsOfType(type::GetType())) {
							std::cout << "Cant add already has component" << std::endl;
							return nullptr;
						}
					}
				}

				_components.emplace_back(std::make_unique<type>(this));
				return static_cast<type*>(_components.back().get());
			}

			/// <summary>
			/// removes the first component of type from the _components list
			/// </summary>
			/// <typeparam name="type"></typeparam>
			template <std::derived_from<Components::Component> type, typename = std::enable_if_t<!std::is_nothrow_convertible_v<Components::Component, type>>>
			void RemoveComponent() {
				if (_components.empty()) {
					return;
				}

				auto typeFound = [cType = type::GetType()](auto& component) {
					return component->IsOfType(cType);
					};

				auto index = std::find_if(_components.begin(), _components.end(), typeFound);

				if (index != _components.end()) {
					_components.erase(index);
				}
			}

			void RemoveComponent(std::string type) {
				if (_components.empty()) {
					return;
				}

				auto typeFound = [type](auto& component) {
					return component->IsOfType(type);
					};

				auto index = std::find_if(_components.begin(), _components.end(), typeFound);

				if (index != _components.end()) {
					_components.erase(index);
				}
			}

			// HELPERS

			uint32_t GetID() {
				return _objectID;
			}

			void SetID(uint32_t id) {
				_objectID = id;
			}

			virtual Object* GetParent() = 0;

			virtual void SetParent(Object* parent) = 0;

			// CHILDREN

			std::vector<std::unique_ptr<Object>>* GetChildren() {
				return &_children;
			}

			Object* GetChild(int index) {
				if (_children.size() == 0 || index > (_children.size() - 1)) {
					return nullptr;
				}
				return _children.at(index).get();
			}

			/// <summary>
			/// Creates a new game object and attaches it as a child object
			/// </summary>
			/// <param name="name"></param>
			virtual Object* CreateChild(std::string name = "GameObject") = 0;
			virtual Object* CreateChild(std::string name, int id) = 0;

			/// <summary>
			/// takes a game object and sets its new parent to this
			/// </summary>
			/// <param name="child"></param>
			virtual void AttachChild(Object* child) = 0;

			// should only be used with paste method in tooling
			virtual void AttachChild(std::unique_ptr<Object> child) = 0;

			/// <summary>
			/// deletes a child object
			/// </summary>
			/// <param name="obj"></param>
			void RemoveChild(Object& obj) {
				if (_children.empty()) {
					return;
				}

				for (int i = _children.size(); i--;) {
					if (_children.at(i)->GetID() == obj.GetID()) {
						_children.erase(std::next(_children.begin(), i));
						return;
					}
				}
			}

			/// <summary>
			/// deletes a child object by index
			/// </summary>
			/// <param name="obj"></param>
			void RemoveChild(int i, bool id = false) {
				if (_children.empty()) {
					return;
				}

				if (id) {
					for (int j = _children.size(); j--;) {
						if (_children.at(j)->GetID() == i) {
							_children.erase(std::next(_children.begin(), j));
							return;
						}
					}
				}
				else {
					_children.erase(std::next(_children.begin(), i));
				}
			}

			Object* FindObject(int id) {
				if (_children.size() == 0) {
					return nullptr;
				}

				for (int i = 0; i < _children.size(); i++) {
					if (_children.at(i).get()->GetID() == id) {
						return _children.at(i).get();
					}
					return _children.at(i).get()->FindObject(id);
				}
			}

			Object* FindObject(std::string name) {
				if (_children.size() == 0) {
					return nullptr;
				}

				for (int i = 0; i < _children.size(); i++) {
					if (_children.at(i).get()->m_name == name) {
						return _children.at(i).get();
					}
					return _children.at(i).get()->FindObject(name);
				}
			}

			/// <summary>
			/// searches for an object with a component of type, returns the first object found with that component, searches children recursively until depth is reached, -1 for infinite depth
			/// </summary>
			/// <typeparam name="type"></typeparam>
			/// <param name="depth"></param>
			/// <returns></returns>
			template <std::derived_from<Components::Component> type>
			Object* FindObjectWithComponent(int depth) {
				if (_children.size() == 0) {
					return nullptr;
				}

				for (int i = 0; i < _children.size(); i++) {
					std::vector<std::unique_ptr<Components::Component>>* tcl = _children.at(i).get()->GetAllComponents();

					if (tcl->empty()) {
						continue;
					}

					auto typeFound = [cType = type::GetType()](auto& component) {
						return component->IsOfType(cType);
						};

					auto index = std::find_if(tcl->begin(), tcl->end(), typeFound);

					if (index != tcl->end()) {
						return _children.at(i).get();
					}
					else {
						if (depth > 0 || depth == -1) {
							Object* result = _children.at(i).get()->FindObjectWithComponent<type>(depth - 1);
							if (result) {
								return result;
							}
							else {
								continue;
							}
						}
					}
				}

				return nullptr;
			}


			virtual Object* Duplicate() = 0;

		protected:
			uint32_t _objectID = 0;
			Object* _gameObject = nullptr;
			std::vector<std::unique_ptr<Object>> _children;
			std::vector<std::unique_ptr<Components::Component>> _components;
		};

		class GameObject;

		/// <summary>
		/// Acts as a container for all game objects within a scene,
		/// Can only act as a parent
		/// </summary>
		class SceneRootObject : public Object {
		public:
			Components::ComponentTypeFactory m_componentFactories;

			static SceneRootObject& GetInstance()
			{
				static SceneRootObject instance;
				return instance;
			}

			SceneRootObject(SceneRootObject const&) = delete;
			SceneRootObject(SceneRootObject&&) = delete;

			SceneRootObject& operator=(const SceneRootObject&) = delete;
			SceneRootObject& operator=(SceneRootObject&&) = delete;

			void Reset() {
				_children.clear();
			}

			virtual Object* GetParent() override;

			virtual void SetParent(Object* parent) override;

			virtual Object* CreateChild(std::string name = "GameObject") override;
			virtual Object* CreateChild(std::string name, int id) override;

			virtual void AttachChild(Object* child) override;

			// should only be used with paste method in tooling
			virtual void AttachChild(std::unique_ptr<Object> child) override;

			virtual Object* Duplicate() override;

		private:
			SceneRootObject() : Object("SceneRoot") {};
		};

		/// <summary>
		/// Represents all objects within a scene, can act as a parent or child to other game objects but cannot be a parent to a SceneRootNode
		/// </summary>
		class GameObject : public Object
		{
		public:
			GameObject() : Object("GameObject") {
				_objectID = reinterpret_cast<uint64_t>(this) + (1 + (rand() % 100));
			}

			GameObject(std::string name) : Object(name) {
				_objectID = reinterpret_cast<uint64_t>(this) + (1 + (rand() % 100));
			}

			GameObject(std::string name, int id) : Object(name) {
				_objectID = id;
			}

			GameObject(std::string name, Object* parent, bool fromCreate = false) : Object(name) {
				_objectID = reinterpret_cast<uint64_t>(this) + (1 + (rand() % 100));

				if (!fromCreate) {
					parent->AttachChild(this);

					/*if (SceneRootObject* sr = static_cast<SceneRootObject*>(parent))
					{
						sr->AttachChild(this);
					}
					else if (GameObject* go = static_cast<GameObject*>(parent))
					{
						go->AttachChild(this);
					}*/
				}
				else {
					_gameObject = parent;
				}
			}

			GameObject(std::string name, int id, Object* parent, bool fromCreate = false) : Object(name) {
				_objectID = id;

				if (!fromCreate) {
					parent->AttachChild(this);

					/*if (SceneRootObject* sr = static_cast<SceneRootObject*>(parent))
					{
						sr->AttachChild(this);
					}
					else if (GameObject* go = static_cast<GameObject*>(parent))
					{
						go->AttachChild(this);
					}*/
				}
				else {
					_gameObject = parent;
				}
			}

			virtual Object* GetParent() override {
				return _gameObject;
			}

			virtual void SetParent(Object* parent) override {
				_gameObject = parent;
			}

			/// <summary>
			/// Creates a new game object and attaches it as a child object
			/// </summary>
			/// <param name="name"></param>
			virtual Object* CreateChild(std::string name = "GameObject") override {
				_children.emplace_back(std::make_unique<GameObject>(name, this, true));
				return _children.back().get();
			}

			virtual Object* CreateChild(std::string name, int id) override {
				_children.emplace_back(std::make_unique<GameObject>(name, id, this, true));
				return _children.back().get();
			}

			/// <summary>
			/// takes a game object and sets its new parent to this
			/// </summary>
			/// <param name="child"></param>
			virtual void AttachChild(Object* child) override {
				if (!child) {
					return;
				}

				if (child->GetParent() == this) {
					return;
				}

				std::unique_ptr<Object> ptr;

				Object* p = child->GetParent();

				if (p) {
					std::vector<std::unique_ptr<Object>>* siblings = p->GetChildren();

					if (siblings) {
						for (int i = siblings->size(); i--;) {
							if (siblings->at(i)->GetID() == child->GetID()) {
								ptr = std::move(siblings->at(i));

								siblings->erase(std::next(siblings->begin(), i));
								break;
							}
						}
					}
					if (!ptr) {
						return;
					}
				}
				else {
					ptr = std::unique_ptr<Object>(child);
				}

				ptr->SetParent(this);
				_children.emplace_back(std::move(ptr));
			}

			// should only be used with paste method in tooling
			virtual void AttachChild(std::unique_ptr<Object> child) override {
				child->SetParent(this);
				_children.emplace_back(std::move(child));
			}

			virtual Object* Duplicate() override {
				GameObject* t = new GameObject();

				t->m_name = m_name;

				for(auto& child : _children) {
					t->AttachChild(child.get()->Duplicate());
				}

				for (int i = 0; i < _components.size(); i++) {
					// call attach components on new object - ensures added handles / slots n stuff
					SceneRootObject::GetInstance().m_componentFactories.Create(_components.at(i).get()->GetInstanceType(), t);
					_components.back().get()->Duplicate(t->_components.back().get());
				}

				return t;
			}
		};
	} // Objects
} // World