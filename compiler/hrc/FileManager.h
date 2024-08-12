#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <optional>

#include <spdlog/spdlog.h>

#include "hrc_global.h"

OPEN_HRC_NAMESPACE

class FileManager {
public:
    struct ImportMeta {
        std::string source_file; // The source file that contains the import statement
        std::string import_name; // The name of the imported file/module
        std::string found_path; // The path where the file was found on disk (empty if not found)
    };

    FileManager(const std::string &input_path,
        const std::optional<std::string> &output_path,
        const std::vector<std::string> &include_paths)
        : _input_file_path(input_path)
        , _output_file_path(output_path)
        , _include_file_paths(include_paths)
    {
    }

    void add_include_path(const std::string &path);
    void set_input_file_path(const std::string &path);
    void set_output_file_path(const std::string &path);

    std::string get_input_file_path() const;
    std::optional<std::string> get_output_file_path() const;
    std::vector<std::string> get_include_file_paths() const;

    std::string get_input_filename() const;
    std::optional<std::string> get_output_filename() const;
    std::vector<std::string> get_include_filenames() const;

    void track_imported_file(const std::string &source_file, const std::string &import_name, const std::string &found_path);
    bool find_import_file(const std::string &import_name, std::string &found_path);

    FILE *open_file(const std::string &path, const char *mode);
    FILE *open_input_file();
    FILE *open_output_file();
    void close_file(FILE *file, const std::string &path);

private:
    std::string _input_file_path;
    std::optional<std::string> _output_file_path;
    std::vector<std::string> _include_file_paths;
    std::vector<ImportMeta> _imported_files;
    std::unordered_map<std::string, FILE *> _open_files;

    std::string extract_filename(const std::string &path) const;
};

CLOSE_HRC_NAMESPACE

#endif