#include "boost/algorithm/string.hpp"
#include "boost/locale.hpp"

#include <iostream>
#include <filesystem>
#include <string>
#include <vector>
#include <list>
#include <utility>
#include <fstream>
#include <sstream>
#include <future>

[[nodiscard]] std::string GetCurrentPath()
{
	return std::filesystem::current_path().string();
}

[[nodiscard]] std::list<std::filesystem::path> GetFilesRecursively(const std::string& current_path)
{
	std::list<std::filesystem::path> files;

	std::filesystem::recursive_directory_iterator itr(current_path);
	for (const auto& file : itr)
	{
		files.emplace_back(file.path());
	}

	return std::move(files);
}

[[nodiscard]] std::vector<std::string> GetExtensions(int argc, char* argv[])
{
	std::vector<std::string> extensions;
	extensions.reserve(argc - 2);

	for (int i = 2; i < argc; ++i)
	{
		extensions.emplace_back(argv[i]);
	}

	return std::move(extensions);
}

[[nodiscard]] bool IsExtensionNeedToBeConverted(const std::string& input, const std::vector<std::string>& extensions)
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
	auto path = file.string();

	std::ifstream inf(path);
	if (!inf.is_open())
		return;

	std::stringstream ss;
	ss << inf.rdbuf();

	inf.close();

	std::ofstream outf(path);
	if (!outf.is_open())
		return;

	auto str = std::move(ss.str());
	auto u8str = std::move(boost::locale::conv::to_utf<char>(str, encoding_from));

	outf << u8str;

	outf.close();
}

void ConvertFileWithInputExtensionToUTF8(const std::filesystem::path& file, const std::string& encoding_from, const std::vector<std::string>& extensions)
{
	auto extension = file.extension().string();

	if (!IsExtensionNeedToBeConverted(extension, extensions))
		return;

	ConvertFileToUTF8(file, encoding_from);
}

void ConvertFilesWithInputExtensionToUTF8(const std::list<std::filesystem::path>& files, const std::string& encoding_from, const std::vector<std::string>& extensions)
{
	std::size_t num_files = files.size();

	std::vector<std::future<void>> converts;
	converts.reserve(num_files);

	for (const auto& file : files)
	{
		converts.emplace_back(std::async(ConvertFileWithInputExtensionToUTF8, file, encoding_from, extensions));
	}

	for (std::size_t i = 0; i < num_files; ++i)
	{
		converts[i].wait();
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		std::cout << R"(	Need encoding and extension. 
	ex) %PROGRAM_NAME% EUC-KR .cpp .h)" << std::endl;
		return EXIT_SUCCESS;
	}

	auto current_path = GetCurrentPath();

	auto extensions = GetExtensions(argc, argv);

	auto files = GetFilesRecursively(current_path);

	ConvertFilesWithInputExtensionToUTF8(files, argv[1], extensions);

	return EXIT_SUCCESS;
}
