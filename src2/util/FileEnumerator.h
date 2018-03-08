#pragma once
#include <boost/filesystem.hpp>
#include <vector>
#include <string>

struct FileEnumerator {
	static std::vector<std::string> listAll(
		const std::string& folder, const std::string& prefix = "");
	static std::vector<std::string> listFile(
		const std::string& folder, const std::string& prefix = "");
	static std::vector<std::string> listDirectory(
		const std::string& folder, const std::string& prefix = "");
};
