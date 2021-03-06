﻿#pragma once 
#include <iostream>
#include <QObject>
#include <QThread>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <atomic>
#include "IRunnable.hpp"
#include <filesystem>

namespace sb {
	struct AppPack;
}

namespace sb {
	class Downloader : public IRunnable {
	public:
		using files = std::vector < AppPack >;
		Downloader();
		~Downloader() {}

		// ImElement implementation
		void Run() override;
		void Reset() override;


	private:
		enum ErrorCode : int { UNKNOWN = -200, MISSING_FILE, FILESYSTEM_ERROR };
		static size_t writeToFile(void* buffer, size_t size, size_t nmemb, void* userdata);
		static int progressCallback(void* clientp, long long dltotal, long long dlnow, long long ultotal, long long ulnow);
		void closeFile();
		bool checkDownloaded();

		int checkState();
		void emitStatus();
		void checkSpeed();

		std::string getErrorStr(int code);
	private:
		QString outfile_ = "";
		qint64 total_ = 0;
		qint64 now_ = 0;
		qint64 lastDownload_ = 0;
		qint32 downloadSpeed_ = 0;
		double speed_;

		FILE* stream_ = nullptr;
		void* curl;
		int res;

		bool cancelled = false;
	};
}
