#pragma once 
#include <iostream>
#include <QObject>
#include <QThread>
#include <QQmlApplicationEngine>
#include <QDebug>
#include <atomic>
#include "IQmlObject.hpp"
#include <filesystem>
#include <fstream>
#define BLOCK_SIZE 512000
struct archive;
namespace bb {
	class ArchieveInstaller : public  QThread {
		Q_OBJECT
	public:
		using files = std::vector < std::filesystem::path>;
		ArchieveInstaller() {};
		~ArchieveInstaller() {}
		void setUnpackFiles(std::vector<std::filesystem::path> files);

		void run() override;
	signals:
		void statusSignal(qint64 progress);
		void ended();
		void error(int);
	public slots:

	private:
		void emitStatus();
		static int64_t myread(archive* a, void* client_data, const void** buff);
		static int ArchieveInstaller::myclose(archive* a, void* client_data);
		files filesToUnpack;
		std::ifstream file;
		std::string actualUnpacking;
		char buff[BLOCK_SIZE];
		qint64 alreadyRead = 0;
		size_t size;
	};
}