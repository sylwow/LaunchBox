﻿#pragma once
#include <QObject>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <QThread>
#include <QNetworkAccessManager>
#include "IComponent.hpp"
#include <filesystem>
#include "ILoadingBar.hpp"
#include "IUpdateManager.hpp"

namespace lb {
	class LoadingBar final : public ILoadingBar {
		Q_OBJECT
	public:


		virtual ~LoadingBar();
		LoadingBar();

		// IQmlObject implementation
		void update() final;
		std::string getName() final;
		void init() final;

		void setTotal(double tot) final;
		void setActual(double act) final;
		void setProgress(double prog) final;
		void setSpeed(double sp) final;
		void setError(const QString& str) final;
		void setState(im::IUpdateManager::State st) final;
		void setVisibleState(im::IUpdateManager::VisibleState st) final;
		void setUninstallMode(bool un) final;
		void reset() final;
		void paused() final;
		void resumed() final;
		//QML Propetries
		Q_PROPERTY(double speed READ getSpeed); //download speed B/s
		Q_PROPERTY(double actual READ getActual); //actual downloaded MB
		Q_PROPERTY(double total READ getTotal); // total MB to download
		Q_PROPERTY(double progress READ getProgress); // total MB to download

		Q_PROPERTY(int state READ getState NOTIFY stateChanged);
		Q_PROPERTY(int visibleState READ getVisibleState NOTIFY visibleStateChanged);

		Q_PROPERTY(int uninstall READ getUninstall);
		Q_PROPERTY(bool pauseResume READ getPauseResume NOTIFY pauseResumeChanged);

		//QMl invoklabes
		Q_INVOKABLE double getSpeed() ;
		Q_INVOKABLE double getActual() ;
		Q_INVOKABLE double getTotal() ;
		Q_INVOKABLE double getProgress() ;

		Q_INVOKABLE int getState() ;
		Q_INVOKABLE int getVisibleState() ;

		Q_INVOKABLE bool getUninstall() ;

		Q_INVOKABLE void pause() ;
		Q_INVOKABLE void resume() ;
		Q_INVOKABLE void stop() ;
		Q_INVOKABLE bool getPauseResume() { return pauseResume_; }


	signals:
		void stateChanged();
		void visibleStateChanged();

		void notifyEnded();
		void getProgress(qint64 actual);
		void pauseResumeChanged();
	private:
		im::IUpdateManager::State state_ = im::IUpdateManager::State::NONE;
		im::IUpdateManager::VisibleState visibleState_ = im::IUpdateManager::VisibleState::HIDDEN;

		im::IUpdateManager::State lastState_ = im::IUpdateManager::State::NONE; // when pausing hold last state in this variable for resume

		//qml properties
		double progress_ = 0;
		double total_ = 0;
		double speed_ = 0;
		double actual_ = 0;

		bool pauseResume_ = false;
		bool uninstall_ = false;
	};
}