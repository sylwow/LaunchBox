﻿#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QNetworkAccessManager>
#include "IComponent.hpp"
#include "InstalationManager.hpp"
#include <filesystem>
#include "Config.hpp"


namespace upd {
	class AppUpdater : public QObject {
		Q_OBJECT
	public:
		enum State : int {
			NONE = 0, DOWNLOADING, INSTALLING, ENDED, ERROR, NO_UPDATE_FOUND, UPDATING_GAME_PAGES
		};
		Q_PROPERTY(QString statusStr READ getStateStr);
		Q_PROPERTY(int progress READ getProgress);
		Q_PROPERTY(int updateState READ getState NOTIFY stateChanged);

		Q_INVOKABLE QString getStateStr() const { return statusStr_; }
		Q_INVOKABLE int getProgress() const { return im.getProgress(); }
		Q_INVOKABLE int getState() const { return static_cast<int>(state_); }
		Q_INVOKABLE void checkForUpdates();

		AppUpdater();
		virtual ~AppUpdater() {};
	private:
	public slots:
		void updateInstalled(const QString& version);
		void errorCatched(const QString& what);
		void updateStatus(State state);
	signals:
		void stateChanged();

	private:
		cf::Config& cf;
		im::InstalationManager& im;
		QString statusStr_;
		State state_ = State::NONE;
	};
}
