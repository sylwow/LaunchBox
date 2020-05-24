#include <iostream>
#include <QDebug>
#include <QQmlContext>
#include "AppBackend.hpp"
#include "Button.hpp"

#define NAME(TYPE) #TYPE
#define REGISTER(CLASS) qmlRegisterType<CLASS>("_"#CLASS, 1, 0, "_"#CLASS); registerObject(ZIP(CLASS))

namespace bc {
	QQmlApplicationEngine* Backend::engine = nullptr;

	void Backend::init() {
		registerObjects();
	}

	void Backend::registerObjects() {
		registerObject<Button>();
		//registerObject(ZIP(Button));

	}

	template<class T>
	void Backend::registerObject() {
		objects_.push_back(new T);
		std::string name(NAME(T));
		name.insert(0, "_");
		auto cStr = name.c_str();
		qmlRegisterType<T>(cStr, 1, 0, cStr);
		engine->rootContext()->setContextProperty(cStr, objects_.back());
	}

	Backend::~Backend() {
		for (auto ptr : objects_)
			delete ptr;
	}
}