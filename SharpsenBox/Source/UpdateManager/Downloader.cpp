﻿#include <stdio.h>
#include <iostream>
#include <curl/curl.h>
#include <QElapsedTimer>
#include <QFileInfo>
#include "Downloader.hpp"
#include "IConfig.hpp"
#include "IComponent.hpp"
#include "IUpdateManager.hpp"
#include "UpdateInfo.hpp"

namespace sb {

	Downloader::Downloader() {}

	size_t Downloader::writeToFile(void* buffer, size_t size, size_t nmemb, void* userdata) {
		Downloader* out = (Downloader*)userdata;
		if (!out->stream_) {
			/* open file for writing */
			out->stream_ = fopen(out->outfile_.toStdString().c_str(), "wb");
			if (!out->stream_)
				return -1; /* failure, can't open file to write */
		}  if (out->cancelled)
			return size == 0 ? 1 : size - 1;

		return  fwrite(buffer, size, nmemb, out->stream_);
	}

	int Downloader::progressCallback(void* userData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow) {
		// It's here you will write the code for the progress message or bar
		Downloader* out = (Downloader*)userData;
		out->total_ = dltotal;
		out->now_ = dlnow - out->lastDownload_;
		out->lastDownload_ = dlnow;
		out->emitStatus();
		return out->checkState();
	}

	void Downloader::checkSpeed() {
		auto sp = Component<IConfig>::Get().GetDownloadSpeed();
		if (downloadSpeed_ != sp) {
			downloadSpeed_ = sp;
			curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, downloadSpeed_ * 1024); //KB/s -> B/s
		}
	}

	int Downloader::checkState() {
		checkSpeed();
		if (!UpdateInfo->pause.test_and_set()) {
			res = curl_easy_pause(curl, CURLPAUSE_ALL);
			if (res == CURLE_OK)
				;//emit ok 
			std::cout << "pause2\n";
			UpdateManager->paused();
		} else if (!UpdateInfo->resume.test_and_set()) {
			res = curl_easy_pause(curl, CURLPAUSE_CONT);
			if (res == CURLE_OK)
				;//emit ok 
			UpdateManager->resumed();
		} else if (!UpdateInfo->stop.test_and_set() || cancelled) {
			std::cout << "cancel\n";
			cancelled = true;
			curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1L);
			return -10;
		}
		return 0;
	}

	void Downloader::emitStatus() {
		auto x = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &speed_);
		UpdateManager->downloadStatus(now_, total_, speed_);
	}

	void Downloader::Run() {
		try {
			// clear flags etc 
			::curl_global_init(CURL_GLOBAL_DEFAULT);
			curl = curl_easy_init();
			if (curl) {
				/*
				 * You better replace the URL with one that works!
				 */
				 /* Define our callback to get called when there's data to be written */
				res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Downloader::writeToFile);
				/* Set a pointer to our struct to pass to the callback */
				res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

				res = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);

				res = curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);
				// Install the callback function
				res = curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, Downloader::progressCallback);
				/* Switch on full protocol/debug output */

				//curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 20'000L);
				//res = curl_easy_setopt(curl, CURLOPT_USERNAME, "public");
				//res = curl_easy_setopt(curl, CURLOPT_PASSWORD, "1234");
				auto& config =  Component<IConfig>::Get();
				auto& downloadDir = config.GetDownloadDir();


				curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

				//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
				auto& files = UpdateInfo->getFiles();
				auto size = files.size();
				for (size_t i = 0; !cancelled && i < size; i++) {
					auto url = files.at(i).Url;
					auto& filename = files.at(i).FileName;
					lastDownload_ = 0;
					outfile_ = config.CombinePath({downloadDir, filename});
					res = curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
					res = curl_easy_perform(curl);
					if (CURLE_OK != res) {
						break;
					}
					closeFile();
				}
			}

		} catch (std::filesystem::filesystem_error&) {
			res = -2;
		} catch (...) {
			res = -1;
		}

		closeFile();
		::curl_easy_cleanup(curl);
		::curl_global_cleanup();
		if (cancelled)
			throw AbortException();
		if (res != CURLE_OK) {
			if (res != CURLE_OPERATION_TIMEDOUT || !cancelled) { // cancel request
				Error(getErrorStr(res));
			}
		}

		cancelled = false;
		if (!checkDownloaded()) {
			res = -3;
			Error(getErrorStr(res));
		}
	}

	bool Downloader::checkDownloaded() {
		auto& config = Component<IConfig>::Get();
		auto& downloadDir = config.GetDownloadDir();
		auto& files = UpdateInfo->getFiles();
		for (auto& file : files) {
			auto finalPath = config.CombinePath({downloadDir, file.FileName});
			if (!QFileInfo::exists(finalPath))
				return false;
		}
		return true;
	}

	void Downloader::closeFile() {
		if (stream_)
			fclose(stream_); /* close the local file */
		stream_ = nullptr;
	}

	void Downloader::Reset() {
		outfile_ = "";
		total_ = 0;
		now_ = 0;
		lastDownload_ = 0;
		downloadSpeed_ = 0;
		speed_ = 0;
		stream_ = nullptr;
		res = 0;
		cancelled = false;
	}

	std::string Downloader::getErrorStr(int code) {
		switch (code) {
		case CURLE_URL_MALFORMAT:
			return "Wrong file request";
		case CURLE_REMOTE_ACCESS_DENIED:
		case CURLE_FTP_ACCEPT_FAILED:
		case CURLE_FTP_WEIRD_PASS_REPLY:
		case CURLE_FTP_ACCEPT_TIMEOUT:
		case CURLE_GOT_NOTHING:
			return "Server error ocured";
		case CURLE_RECV_ERROR:
			return "Receiving data error";
		case CURLE_COULDNT_RESOLVE_HOST:
			return "Could not reach server";
		case CURLE_COULDNT_CONNECT:
		case CURLE_FTP_CANT_GET_HOST:
			return "Could not connect to Server";
		case CURLE_OUT_OF_MEMORY:
			return "Out of memeory, could not allocate memory for downloaded files";
		case CURLE_WRITE_ERROR:
			return  "Could not save files on disk";
		case CURLE_OPERATION_TIMEDOUT:
			return  "Connection timeout";
		case MISSING_FILE:
			return  "Missing file, that should be downloaded";
		case FILESYSTEM_ERROR:
			return  "Filesystem error, check disk space";
		case UNKNOWN:
		default:
			return "Unexpected error ocured while downloading data";
		}
		return "";
	}
}
