#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <unordered_map>
#include "ComponentRegister.hpp"

namespace sb {
	class Backend {
	public:
		Backend(QQmlApplicationEngine* eng);
		~Backend() {}
		
		void Init();
		void RegisterComponents();
	private:
		ComponentRegister _Register;
	};
}