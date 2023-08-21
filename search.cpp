#include "progress.cpp"
#include "utils.cpp"

using namespace std;
using namespace filesystem;

template <typename T>
using hashset = std::unordered_set<T>;

template <typename T>
using linkedlist = std::list<T>;

class FileSearch {
private:
    optional<path> root;
    hashset<string> exclusive_filenames;
    hashset<string> exclusive_file_stems;
    hashset<string> exclusive_exts;
    hashset<path> exclude_dirs;
    bool quit_directory_on_match;

    void to_lower_case(string &str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    bool evaluate_entry_criteria(const path &entry) {
        bool is_exclusive_filename = this->is_exclusive_filename(entry);
        bool is_exclusive_file_stem = this->is_exclusive_file_stem(entry);
        bool is_exclusive_extension = this->is_exclusive_extension(entry);
        bool criteria = is_exclusive_filename && is_exclusive_file_stem && is_exclusive_extension;
        return criteria;
    }

    optional<path> get_canonical_path(path &entry) {
        try {
            path path_canonical = filesystem::canonical(entry);
            return path_canonical;
        } catch (filesystem_error &e) {
            return nullopt;
        }
    }

    optional<directory_iterator> get_directory_entries(path &root) {
        try {
            directory_iterator entries = directory_iterator(root);
            return entries;
        } catch (filesystem_error &e) {
            return nullopt;
        }
    }

    path get_abs_path() {
        return filesystem::current_path();
    }

    optional<path> get_root_path() {
        if (this->root.has_value()) {
            path root = this->root.value();
            return get_canonical_path(root);
        }
        path root = get_abs_path();
        return get_canonical_path(root);
    }

    bool is_same_directory(const path &entry, const path &dir) {
        if (filesystem::is_regular_file(entry)) {
            path path_parent = entry.parent_path();
            if (!path_parent.empty()) {
                return dir == path_parent;
            }
        }
        return dir == entry;
    }

    bool is_exclusive_filename(const path &entry) {
        if (this->exclusive_filenames.empty()) {
            return true;
        }

        string filename = entry.filename().string();
        this->to_lower_case(filename);
        return (this->exclusive_filenames.find(filename) != this->exclusive_filenames.end());
    }

    bool is_exclusive_file_stem(const path &entry) {
        if (this->exclusive_file_stems.empty()) {
            return true;
        }

        string file_stem = entry.stem().string();
        this->to_lower_case(file_stem);
        return (this->exclusive_file_stems.find(file_stem) != this->exclusive_file_stems.end());
    }

    bool is_exclusive_extension(const path &entry) {
        if (this->exclusive_exts.empty()) {
            return true;
        }

        string file_ext = entry.extension().string();
        this->to_lower_case(file_ext);
        return (this->exclusive_exts.find(file_ext) != this->exclusive_exts.end());
    }

    bool is_excluded_directory(const path &entry) {
        if (this->exclude_dirs.empty()) {
            return false;
        }

        for (const path &dir : this->exclude_dirs) {
            if (this->is_same_directory(entry, dir)) {
                return true;
            }
        }
        return false;
    }

    bool handle_file(const directory_entry &entry, hashset<path> &files, SearchProgress &search_progress) {
        bool entry_criteria = this->evaluate_entry_criteria(entry);

        search_progress.increment_search();
        search_progress.increment_search_bytes(entry); // causes a ~50% slowdown

        if (!(files.find(entry) != files.end()) && entry_criteria) {
            path file = entry.path();
            files.insert(file);
            search_progress.increment_match();
            return true;
        }
        return false;
    }

    void handle_entry(const directory_entry &entry, hashset<path> &files, linkedlist<path> &additional_directories, SearchProgress &search_progress) {
        search_progress.show_progress();

        if (entry.is_regular_file()) {
            bool is_match = this->handle_file(entry, files, search_progress);
            if (is_match && this->quit_directory_on_match) {
                return;
            }
        } else if (entry.is_directory()) {
            if (!entry.is_symlink()) {
                additional_directories.push_back(entry);
            }
        }
    }

    void walker(path &root, hashset<path> &files, linkedlist<path> &queue, SearchProgress &search_progress) {
        if (this->is_excluded_directory(root)) {
            return;
        }

        optional<directory_iterator> op_entries = get_directory_entries(root);
        if (!op_entries.has_value()) {
            return;
        }
        directory_iterator entries = op_entries.value();
        linkedlist<path> sub_directories;

        for (const directory_entry &entry : entries) {
            this->handle_entry(entry, files, sub_directories, search_progress);
        }

        queue.splice(queue.end(), sub_directories);
    }

public:
    void set_root(string &root) {
        this->root = path(root);
    }

    void set_exclusive_filenames(hashset<string> &filenames) {
        hashset<string> exclusive_filenames;

        for (string filename : filenames) {
            this->to_lower_case(filename);
            exclusive_filenames.insert(filename);
        }

        this->exclusive_filenames = exclusive_filenames;
    }

    void set_exclusive_file_stems(hashset<string> file_stems) {
        hashset<string> exclusive_file_stems;

        for (string file_stem : file_stems) {
            this->to_lower_case(file_stem);
            exclusive_file_stems.insert(file_stem);
        }

        this->exclusive_file_stems = exclusive_file_stems;
    }

    void set_exclusive_extensions(hashset<string> &exts) {
        hashset<string> exclusive_exts;

        for (string ext : exts) {
            Trim(ext).trim();
            this->to_lower_case(ext);
            exclusive_exts.insert(ext);
        }

        this->exclusive_exts = exclusive_exts;
    }

    void set_exclude_directories(hashset<string> &dirs) {
        hashset<path> exclude_dirs;

        for (path dir : dirs) {
            exclude_dirs.insert(path(dir));
        }
        this->exclude_dirs = exclude_dirs;
    }

    void set_quit_directory_on_match(bool state) {
        this->quit_directory_on_match = state;
    }

    hashset<path> search_files() {
        hashset<path> files;
        linkedlist<path> queue;
        SearchProgress search_progress;

        optional<path> root_path = this->get_root_path();

        if (root_path.has_value()) {
            path root = root_path.value();
            search_progress.show_root_search(root);
            queue.push_back(root);

            while (!queue.empty()) {
                path dir = queue.front();
                queue.pop_front();
                walker(dir, files, queue, search_progress);
            }
        }

        search_progress.finalize();
        return files;
    }
};
