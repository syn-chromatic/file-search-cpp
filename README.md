# File Search C++

## `➢` Information
A C++ utility to search files with various filters such as:
* Exclusive Filenames
* Exclusive Extensions
* Exclude Directories

The algorithm recursively searches through all of the directories from the specified root.

## `➢` Example Usage
```cpp
FileSearch file_search = FileSearch();

// Set the root directory for the file search
string root = "./";
file_search.set_root(root);

// Below examples are optional

// Specify filenames to exclusively search for
vector<string> exclusive_filenames = {"README"};
file_search.set_exclusive_filenames(exclusive_filenames);

// Specify extensions to exclusively search for
vector<string> exclusive_exts = {".md"};
file_search.set_exclusive_extensions(exclusive_exts);


// Specify directories to exclude from the search
// This excludes the path and not the directory name
vector<string> exclude_dirs = {"./excluded_dir"};
file_search.set_exclude_directories(exclude_dirs);

// Perform the file search and get the result
vector<path> files = file_search.search_files();
```
