#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_set>

#include "search.cpp"

int main() {
    FileSearch file_search = FileSearch();

    string root = "";
    hashset<string> exclusive_filenames = {};
    hashset<string> exclusive_file_stems = {};
    hashset<string> exclusive_exts = {};
    hashset<string> exclude_dirs = {};

    file_search.set_root(root);
    file_search.set_exclusive_filenames(exclusive_filenames);
    file_search.set_exclusive_file_stems(exclusive_file_stems);
    file_search.set_exclusive_extensions(exclusive_exts);
    file_search.set_exclude_directories(exclude_dirs);

    hashset<path> files = file_search.search_files();

    for (path file : files) {
        printf("[%s]\n", file.string().c_str());
    }

    return 0;
}
