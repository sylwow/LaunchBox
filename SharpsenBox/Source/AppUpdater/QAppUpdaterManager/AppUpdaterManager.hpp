﻿#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QString>
#include <QDebug>
#include <QNetworkAccessManager>
#include "IComponent.hpp"
#include "IConfig.hpp"
#include "UpdateManager.hpp"
#include "IAppUpdaterManager.hpp"
#include <filesystem>
#include <string>


namespace sb {
	class AppUpdaterManager : public IAppUpdaterManager {
		Q_OBJECT
	public:

		std::string GetName() override { return TYPENAME(AppUpdaterManager); }
		void Init() override {}
		void Update() override {}

		Q_PROPERTY(QString statusStr READ getStateStr);
		Q_PROPERTY(int progress READ getProgress NOTIFY progressChanged);
		Q_PROPERTY(int updateState READ getState NOTIFY stateChanged);

		Q_INVOKABLE QString getStateStr();
		Q_INVOKABLE int getProgress();
		Q_INVOKABLE int getState();
		Q_INVOKABLE void checkForUpdates();

		AppUpdaterManager();
		virtual ~AppUpdaterManager();
	private:
	public slots:
		void updateInstalled(const QString& version);
		void updateProgress(double progress);
		void errorCatched(const QString& what);
		void updateSt(int status);
	signals:
		void stateChanged();
		void progressChanged();

	private:
		void updateStatus(IUpdateManager::State state);
		IConfig& cf_;
		UpdateManager UpdateManager;
		QString statusStr_;
		IUpdateManager::State state_ = IUpdateManager::State::NONE;

		double progress_;
	};
}
