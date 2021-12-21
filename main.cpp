#include "boost/algorithm/string.hpp"
#include "boost/locale.hpp"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>

[[nodiscard]]
std::string GetCurrentPath()
{
	return std::filesystem::current_path().string();
}

[[nodiscard]]
std::list<std::filesystem::path> GetFilesRecursively(const std::string& current_path)
{
	std::list<std::filesystem::path> files;

	std::filesystem::recursive_directory_iterator itr(current_path);
	for (const auto& file : itr)
	{
		files.emplace_back(file.path());
	}

	return files;
}

[[nodiscard]]
std::vector<std::string> GetExtensions(int argc, char* argv[])
{
	std::vector<std::string> extensions;
	extensions.reserve(argc - 2);

	for (int i = 2; i < argc; ++i)
	{
		extensions.emplace_back(argv[i]);
	}

	return extensions;
}

[[nodiscard]]
bool IsExtensionNeedToBeConverted(const std::string& input, const std::vector<std::string>& extensions)
{
	for (const auto& extension : extensions)
	{
		if (boost::iequals(input, extension))
			return true;
	}

	return false;
}

void ConvertFileToUTF8(const std::filesystem::path& file, const std::string& encoding_from)
{
	auto path(file.string());

	std::ifstream inf(path);
	if (!inf.is_open())
		return;

	std::stringstream ss;
	ss << inf.rdbuf();

	inf.close();

	std::ofstream outf(path);
	if (!outf.is_open())
		return;

	auto str(ss.str());
	auto u8str(boost::locale::conv::to_utf<char>(str, encoding_from));

	outf << u8str;

	outf.close();
}

void ConvertFileWithInputExtensionToUTF8(const std::filesystem::path& file, const std::string& encoding_from, const std::vector<std::string>& extensions)
{
	auto extension(file.extension().string());

	if (!IsExtensionNeedToBeConverted(extension, extensions))
		return;

	ConvertFileToUTF8(file, encoding_from);
}

void ConvertFilesWithInputExtensionToUTF8(const std::list<std::filesystem::path>& files, const std::string& encoding_from, const std::vector<std::string>& extensions)
{
	std::size_t num_files(files.size());

	for (const auto& file : files)
	{
		ConvertFileWithInputExtensionToUTF8(file, encoding_from, extensions);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << "Need encoding and extension." << std::endl;
		std::cout << "ex) filetoutf8 EUC-KR .cpp .h" << std::endl;
		return EXIT_SUCCESS;
	}

	auto current_path(GetCurrentPath());

	auto extensions(GetExtensions(argc, argv));

	auto files(GetFilesRecursively(current_path));

	ConvertFilesWithInputExtensionToUTF8(files, argv[1], extensions);

	std::cout << "DONE!" << std::endl;

	return EXIT_SUCCESS;
}