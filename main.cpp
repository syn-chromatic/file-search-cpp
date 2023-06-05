#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

using namespace std;
using namespace filesystem;

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
        size_t whitespace_chars = str.find_last_not_of(spaces) + 1;
        this->str = str.erase(whitespace_chars);
    }

    void left_trim() {
        string str = this->str;
        const char *spaces = this->spaces;
        size_t whitespace_chars = str.find_first_not_of(spaces);
        this->str = str.erase(0, whitespace_chars);
    }

    void trim() {
        this->right_trim();
        this->left_trim();
    }
};

class FileSearch {
private:
    optional<path> root;
    vector<string> exclusive_filenames;
    vector<string> exclusive_exts;
    vector<path> exclude_dirs;

    string format_extension(string &ext) {
        Trim(ext).trim();
        transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (!ext.empty() && ext[0] != '.') {
            ext.insert(0, ".");
        }
        return ext;
    }

    bool get_filter_validation(path &fpath) {
        bool is_exclusive_filename = this->is_exclusive_filename(fpath);
        bool is_exclusive_extension = this->is_exclusive_extension(fpath);
        bool filter_validation = is_exclusive_filename && is_exclusive_extension;
        return filter_validation;
    }

    optional<path> get_canonical_path(path &fpath) {
        try {
            path path_canonical = filesystem::canonical(fpath);
            return path_canonical;
        } catch (filesystem_error &e) {
            printf("Path Inaccessible: [%s]\n\n", fpath.string().c_str());
            return nullopt;
        }
    }

    optional<directory_iterator> get_directory_entries(path &root) {
        try {
            directory_iterator entries = directory_iterator(root);
            return entries;
        } catch (filesystem_error &e) {
            printf("Path Inaccessible: [%s]\n\n", root.string().c_str());
            return nullopt;
        }
    }

    path get_abs_path() {
        return filesystem::current_path();
    }

    path get_root_path() {
        if (this->root.has_value()) {
            return this->root.value();
        }
        return get_abs_path();
    }

    bool is_same_directory(path &fpath, path &dir) {
        if (exists(dir)) {
            while (fpath != fpath.root_path()) {
                if (fpath == dir) {
                    return true;
                }
                fpath = fpath.parent_path();
            }
        }
        return false;
    }

    bool is_exclusive_filename(path &fpath) {
        if (this->exclusive_filenames.empty()) {
            return true;
        }

        string file_stem = fpath.stem().string();
        for (string file_name : this->exclusive_filenames) {
            if (file_name == file_stem) {
                return true;
            }
        }
        return false;
    }

    bool is_exclusive_extension(path &fpath) {
        if (this->exclusive_exts.empty()) {
            return true;
        }

        for (string ext : this->exclusive_exts) {
            ext = this->format_extension(ext);
            string file_ext = fpath.extension().string();
            file_ext = this->format_extension(file_ext);

            if (file_ext == ext) {
                return true;
            }
        }
        return false;
    }

    bool is_excluded_directory(path &fpath) {
        if (this->exclude_dirs.empty()) {
            return false;
        }

        path fpath_c = fpath;
        for (path dir : this->exclude_dirs) {
            bool is_same_directory = this->is_same_directory(fpath_c, dir);
            if (is_same_directory) {
                return true;
            }
        }
        return false;
    }

    void handle_file(path &fpath, vector<path> &files) {
        bool filter_validation = this->get_filter_validation(fpath);
        bool contains_path = find(files.begin(), files.end(), fpath) != files.end();

        if (!contains_path && filter_validation) {
            files.push_back(fpath);
        }
    }

    void handle_folder(path &fpath, vector<path> &roots, vector<path> &files) {
        bool contains_path = find(roots.begin(), roots.end(), fpath) != roots.end();

        if (!contains_path) {
            roots.push_back(fpath);
            this->search(fpath, roots, files);
        }
    }

    void walker(directory_iterator &entries, vector<path> &roots, vector<path> &files) {
        for (path entry : entries) {
            optional<path> op_fpath = this->get_canonical_path(entry);
            if (op_fpath.has_value()) {
                path fpath = op_fpath.value();

                if (filesystem::is_regular_file(fpath)) {
                    this->handle_file(fpath, files);

                } else if (filesystem::is_directory(fpath)) {
                    this->handle_folder(fpath, roots, files);
                }
            }
        }
    }

    void search(path &root, vector<path> &roots, vector<path> &files) {
        optional<path> root_op = this->get_canonical_path(root);
        if (root_op.has_value()) {
            path root = root_op.value();
            if (this->is_excluded_directory(root)) {
                return;
            }

            optional<directory_iterator> op_entries = get_directory_entries(root);
            if (op_entries.has_value()) {
                directory_iterator entries = op_entries.value();
                this->walker(entries, roots, files);
            }
        }
    }

public:
    void set_root(string &root) {
        this->root = path(root);
    }

    void set_exclusive_filenames(vector<string> &filenames) {
        vector<string> exclusive_filenames;
        exclusive_filenames.reserve(filenames.size());

        for (string filename : filenames) {
            exclusive_filenames.push_back(filename);
        }
        this->exclusive_filenames = exclusive_filenames;
    }

    void set_exclusive_extensions(vector<string> &exts) {
        vector<string> exclusive_exts;
        exclusive_exts.reserve(exts.size());

        for (string ext : exts) {
            exclusive_exts.push_back(ext);
        }
        this->exclusive_exts = exclusive_exts;
    }

    void set_exclude_directories(vector<string> &dirs) {
        vector<path> exclude_dirs;
        for (path dir : dirs) {
            exclude_dirs.push_back(path(dir));
        }
        this->exclude_dirs = exclude_dirs;
    }

    vector<path> search_files() {
        vector<path> roots;
        vector<path> files;

        path root = this->get_root_path();
        this->search(root, roots, files);
        return files;
    }
};

int main() {
    FileSearch file_search = FileSearch();

    string root = "./";
    vector<string> exclusive_filenames = {};
    vector<string> exclusive_exts = {};
    vector<string> exclude_dirs = {};

    file_search.set_root(root);
    file_search.set_exclusive_filenames(exclusive_filenames);
    file_search.set_exclusive_extensions(exclusive_exts);
    file_search.set_exclude_directories(exclude_dirs);

    vector<path> files = file_search.search_files();

    for (path file : files) {
        printf("[%s]\n", file.string().c_str());
    }

    return 0;
}
