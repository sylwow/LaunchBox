#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QNetworkAccessManager>
#include "IQmlObject.hpp"
#include "InstalationManager.hpp"
#include <filesystem>
#include "Config.hpp"


namespace upd {
	class AppUpdater : public QObject {
		Q_OBJECT
	public:
		enum State : int {
			none = 0, downloading, installing, ended, error, noUpdateFound
		};
		Q_PROPERTY(QString statusStr READ getStateStr NOTIFY stateStrChanged);
		Q_PROPERTY(int progress READ getProgress);
		Q_PROPERTY(bool exit READ getExit NOTIFY exitChanged);

		Q_INVOKABLE QString getStateStr() const { return statusStr_; }
		Q_INVOKABLE int getProgress() const { return im.getProgress(); }
		Q_INVOKABLE bool getExit() const { return exit_; }

		AppUpdater();
		virtual ~AppUpdater() {};
	private:
		void installUpdate();
	public slots:
		void LauchBoxJsonDownloaded();
		void updateDownloaded();
		void updateInstalled();
		void checkForUpdates();
		void errorCatched();
	signals:
		void exitChanged();
		void stateStrChanged();

	private:
		bool exit_ = false;
		cf::Config& cf;
		bb::InstalationManager& im;
		QString statusStr_ = "Searching for updates";
		State state_ = State::none;
		bool installing_ = false;
		bb::InstalationManager* dm = nullptr;
		std::filesystem::path LBJsonFileName = "LaunchBoxInfo.json";
		QString UpdateFile;
		qint64 updateSize_ = 0;
	};
}