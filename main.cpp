#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace std;

class Trim {
private:
    string &str;
    const char *spaces = " \t\n\r\f\v";

public:
    Trim(string &str) : str(str) {
        this->str = str;
    }

    void right_trim() {
        string str = this->str;
        const char *spaces = this->spaces;
        char whitespace_chars = str.find_last_not_of(spaces) + 1;
        this->str = str.erase(whitespace_chars);
    }

    void left_trim() {
        string str = this->str;
        const char *spaces = this->spaces;
        char whitespace_chars = str.find_first_not_of(spaces);
        this->str = str.erase(0, whitespace_chars);
    }

    void trim() {
        this->right_trim();
        this->left_trim();
    }
};

class FileSearch {
private:
    optional<filesystem::path> root;
    vector<string> include_filenames;
    vector<string> include_exts;
    vector<filesystem::path> exclude_dirs;

public:
    FileSearch() {
    }

    void set_root(string &root) {
        this->root = filesystem::path(root);
    }

    void set_include_filenames(vector<string> &filenames) {
        vector<string> include_filenames;
        include_filenames.reserve(filenames.size());

        for (string filename : filenames) {
            include_filenames.push_back(filename);
        }
        this->include_filenames = include_filenames;
    }

    void set_include_extensions(vector<string> &exts) {
        vector<string> include_exts;
        include_exts.reserve(exts.size());

        for (string ext : exts) {
            include_exts.push_back(ext);
        }
        this->include_exts = include_exts;
    }

    void set_exclude_directories(vector<string> &dirs) {
        vector<filesystem::path> exclude_dirs;
        for (filesystem::path dir : dirs) {
            exclude_dirs.push_back(filesystem::path(dir));
        }
        this->exclude_dirs = exclude_dirs;
    }

    vector<filesystem::path> search_files() {
        vector<filesystem::path> roots;
        vector<filesystem::path> files;

        filesystem::path root = this->get_root_path();
        this->recursive_search(root, roots, files);
        return files;
    }

    string format_extension(string &ext) {
        Trim(ext).trim();
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (!ext.empty() && ext[0] != '.') {
            ext.insert(0, ".");
        }
        return ext;
    }

    bool is_same_directory(filesystem::path &file, filesystem::path &dir) {
        if (filesystem::exists(dir)) {
            filesystem::path dir_canon = filesystem::canonical(dir);
            filesystem::path file_canon = filesystem::canonical(file);

            while (file_canon.has_parent_path()) {
                file_canon = file_canon.parent_path();
                if (file_canon == dir_canon) {
                    return true;
                }
            }
        }
        return false;
    }

    bool is_included_filename(filesystem::path &file_path) {
        if (this->include_filenames.empty()) {
            return true;
        }

        string file_stem = file_path.stem().string();
        for (string file_name : this->include_filenames) {
            if (file_name == file_stem) {
                return true;
            }
        }
        return false;
    }

    bool is_included_extension(filesystem::path &file_path) {
        if (this->include_exts.empty()) {
            return true;
        }

        for (string ext : this->include_exts) {
            ext = this->format_extension(ext);
            string file_ext = file_path.extension().string();
            file_ext = this->format_extension(file_ext);

            if (file_ext == ext) {
                return true;
            }
        }
        return false;
    }

    bool is_excluded_directory(filesystem::path &file_path) {
        if (this->exclude_dirs.empty()) {
            return false;
        }

        for (filesystem::path dir : this->exclude_dirs) {
            bool is_same_directory = this->is_same_directory(file_path, dir);
            if (is_same_directory) {
                return true;
            }
        }
        return false;
    }

    bool filter_pass(filesystem::path &file_path) {
        bool is_included_filename = this->is_included_filename(file_path);
        bool is_included_extension = this->is_included_extension(file_path);
        bool is_excluded_directory = this->is_excluded_directory(file_path);

        bool filters = is_included_filename && is_included_extension && !is_excluded_directory;
        return filters;
    }

    void handle_file(filesystem::path &path, vector<filesystem::path> &files) {
        bool filter_pass = this->filter_pass(path);
        bool contains_path = find(files.begin(), files.end(), path) != files.end();

        if (!contains_path && filter_pass) {
            files.push_back(path);
        }
    }

    void handle_folder(filesystem::path &path, vector<filesystem::path> &roots, vector<filesystem::path> &files) {
        bool contains_path = find(roots.begin(), roots.end(), path) != roots.end();

        if (!contains_path) {
            roots.push_back(path);
            this->recursive_search(path, roots, files);
        }
    }

    void recursive_search(filesystem::path &root, vector<filesystem::path> &roots, vector<filesystem::path> &files) {
        try {
            filesystem::directory_iterator entries = filesystem::directory_iterator(root);

            for (filesystem::path entry : entries) {
                filesystem::path path = filesystem::canonical(entry);

                if (filesystem::is_regular_file(path)) {
                    this->handle_file(path, files);

                } else if (filesystem::is_directory(path)) {
                    this->handle_folder(path, roots, files);
                }
            }

        } catch (filesystem::filesystem_error &e) {
            printf("Directory Inaccessible: [%s]\n", root.string().c_str());
        }
    }

    filesystem::path get_abs_path() {
        return filesystem::current_path();
    }

    filesystem::path get_root_path() {
        if (this->root.has_value()) {
            return this->root.value();
        }
        return get_abs_path();
    }
};

int main() {
    FileSearch file_search = FileSearch();

    string root = "./";
    vector<string> include_filenames = {};
    vector<string> include_exts = {};
    vector<string> exclude_dirs = {};

    file_search.set_root(root);
    file_search.set_include_filenames(include_filenames);
    file_search.set_include_extensions(include_exts);
    file_search.set_exclude_directories(exclude_dirs);

    vector<filesystem::path> files = file_search.search_files();

    for (filesystem::path file : files) {
        printf("[%s]\n", file.string().c_str());
    }

    return 0;
}