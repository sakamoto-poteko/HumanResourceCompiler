#include "FileManager.h"
#include "hrl_global.h"

OPEN_HRC_NAMESPACE

void FileManager::add_include_path(const std::string &path)
{
    _include_file_paths.push_back(path);
    spdlog::info("Added include path: {}", path);
}

void FileManager::set_input_file_path(const std::string &path)
{
    _input_file_path = path;
    spdlog::info("Set input file path: {}", path);
}

void FileManager::set_output_file_path(const std::string &path)
{
    _output_file_path = path;
    spdlog::info("Set output file path: {}", path);
}

void FileManager::track_imported_file(const std::string &source_file, const std::string &import_name, const std::string &found_path)
{
    ImportMeta meta = { source_file, import_name, found_path };
    _imported_files.push_back(meta);
    if (!found_path.empty()) {
        spdlog::info("Tracked imported file '{}' from '{}' found at '{}'", import_name, source_file, found_path);
    } else {
        spdlog::warn("Tracked imported file '{}' from '{}' but file not found", import_name, source_file);
    }
}

bool FileManager::find_import_file(const std::string &import_name, std::string &found_path)
{
    UNUSED(found_path);
    // This function should search include paths and tracked imports to find the file
    // Implementation needed
    spdlog::info("Searching for import file: {}", import_name);
    return false; // Stub return value
}

FILE *FileManager::open_file(const std::string &path, const char *mode)
{
    FILE *file = fopen(path.c_str(), mode);
    if (file == nullptr) {
        spdlog::error("Failed to open file '{}': {}", path, std::strerror(errno));
        return nullptr;
    }

    _open_files[path] = file;
    spdlog::debug("Successfully opened file: {}", path);
    return file;
}

// Close a file
void FileManager::close_file(FILE *file, const std::string &path)
{
    if (file) {
        fclose(file);
        spdlog::info("Closed file: {}", path);
        _open_files.erase(path);
    } else {
        spdlog::warn("Attempted to close a null file pointer for path: {}", path);
    }
}

FILE *FileManager::open_input_file()
{
    return open_file(_input_file_path, "r");
}

FILE *FileManager::open_output_file()
{
    if (_output_file_path) {
        return open_file(*_output_file_path, "w");
    } else {
        spdlog::info("Output file path is not set.");
        return nullptr;
    }
}

std::string FileManager::get_input_file_path() const
{
    return _input_file_path;
}

std::optional<std::string> FileManager::get_output_file_path() const
{
    return _output_file_path;
}

std::vector<std::string> FileManager::get_include_file_paths() const
{
    return _include_file_paths;
}

std::string FileManager::get_input_filename() const
{
    return extract_filename(_input_file_path);
}

std::optional<std::string> FileManager::get_output_filename() const
{
    if (_output_file_path) {
        return extract_filename(*_output_file_path);
    }
    return std::nullopt;
}

std::vector<std::string> FileManager::get_include_filenames() const
{
    std::vector<std::string> filenames;
    for (const auto &path : _include_file_paths) {
        filenames.push_back(extract_filename(path));
    }
    return filenames;
}

std::string FileManager::extract_filename(const std::string &path) const
{
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) {
        return path; // No directory part, return the path as is
    }
    return path.substr(pos + 1);
}

CLOSE_HRC_NAMESPACE