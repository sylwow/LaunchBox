﻿#include "AppUpdater.hpp"
#include "Config.hpp"
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <string>


namespace upd {

	AppUpdater::AppUpdater() : cf(cf::Config::getObject()), im(bb::InstalationManager::getObject()) {
		cf.init();
		im.init();
		auto& downloadDir = cf.getDownloadDir();
		if (!std::filesystem::exists(downloadDir))
			std::filesystem::create_directory(downloadDir);
		;//problem
	}

	void AppUpdater::checkForUpdates() {
		auto& im = bb::InstalationManager::getObject();
		connect(&im, &bb::InstalationManager::updateStatus, this, &AppUpdater::updateStatus);
		connect(&im, &bb::InstalationManager::readGameInfo, this, &AppUpdater::readGameInfo);
		connect(&im, &bb::InstalationManager::errorEmit, this, &AppUpdater::errorCatched);
		connect(&im, &bb::InstalationManager::updateEnded, this, &AppUpdater::updateInstalled);
		state_ = State::DOWNLOADING;
		im.updateMainApp(cf.getVer(), cf.getLauncherAppInfoUrl(), cf.getGameInfoRepository(), cf.getVer() == "0");
	}

	void AppUpdater::updateStatus(bool downloading) {
		//download new Laucher
		state_ = downloading ? State::DOWNLOADING : State::INSTALLING;
		stateChanged();
	}

	void AppUpdater::readGameInfo() {
		state_ = State::UPDATING_GAME_PAGES;
		stateChanged();
		connect( &gameParser_, &GameParser::parseEnded, this, &AppUpdater::parseEnded);
		gameParser_.parse();
	}

	void AppUpdater::parseEnded() {
		im.updateGamePages(gameParser_.getFiles());
	}

	void AppUpdater::updateInstalled(QString version) {
		state_ = State::ENDED;
		disconnect(&im, &bb::InstalationManager::updateStatus, this, &AppUpdater::updateStatus);
		disconnect(&im, &bb::InstalationManager::errorEmit, this, &AppUpdater::errorCatched);
		disconnect(&im, &bb::InstalationManager::updateEnded, this, &AppUpdater::updateInstalled);
		cf.setVer(version);
		gameParser_.updateGamesInfo();
		stateChanged();
	}

	void AppUpdater::errorCatched() {
		//auto msg = bb::InstalationManager::getObject().getErrorString();
		state_ = State::ERROR;
		statusStr_ = im.getError();
		stateChanged();
	}
}
