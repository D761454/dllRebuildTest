#include "object.h"

World::Objects::Object* World::Objects::SceneRootObject::GetParent() {
	return nullptr;
}

void World::Objects::SceneRootObject::SetParent(Object* parent) {
}

/// <summary>
/// Creates a new game object and attaches it as a child object
/// </summary>
/// <param name="name"></param>
World::Objects::Object* World::Objects::SceneRootObject::CreateChild(std::string name) {
	_children.emplace_back(std::make_unique<GameObject>(name, this, true));
	return _children.back().get();
}

World::Objects::Object* World::Objects::SceneRootObject::CreateChild(std::string name, int id) {
	_children.emplace_back(std::make_unique<GameObject>(name, id, this, true));
	return _children.back().get();
}

/// <summary>
/// takes a game object and sets its new parent to this
/// </summary>
/// <param name="child"></param>
void World::Objects::SceneRootObject::AttachChild(Object* child) {
	if (!child) {
		return;
	}

	std::unique_ptr<Object> ptr;

	Object* p = child->GetParent();

	if (p) {
		if (p == this)
		{
			return;
		}
		// remove child from its parents list of children
		std::vector<std::unique_ptr<Object>>* siblings = p->GetChildren();

		for (int i = siblings->size(); i--;) {
			if (siblings->at(i)->GetID() == child->GetID()) {
				ptr = std::move(siblings->at(i));

				siblings->erase(std::next(siblings->begin(), i));
				break;
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

void World::Objects::SceneRootObject::AttachChild(std::unique_ptr<Object> child) {
	child->SetParent(this);
	_children.emplace_back(std::move(child));
}

World::Objects::Object* World::Objects::SceneRootObject::Duplicate() {
	return nullptr;
}