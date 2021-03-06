﻿#include <gtest/gtest.h>
#include "AppInfoParser.hpp"
#include "UpdateManager.hpp"
#include "UpdateInfo.hpp"
#include "UpdateManagerMock.hpp"

using namespace sb;

namespace {

	struct ConfInit { // to create IConfig
		ConfInit() {
			try {
				ComponentCreator<IConfig, ConfigMock>::create();
			} catch (...) {}
		}
	} confInit_;

	UpdateManagerMock um_;
	AppInfoParser ap_;
}

TEST(versionChecking, versionChecking) {
	auto& cf = Component<IConfig>::get();
	auto& downloadDir = cf.getDownloadDir();
	ap_.init(um_);
	auto& UpdateInfo = um_.getUpdateInfo();
	UpdateInfo.setActualVersion("0");
	downloadDir = "./Jsons/1"; // first scenario with ver = 0
	ap_.Reset();
	ap_.Run();
	// when ver = 0 application is not installed = need Update/installation
	EXPECT_EQ(ap_.needUpdate(), true);
}

TEST(sample_test_case2, sample_test2) {
	EXPECT_EQ(1, 1);
}
