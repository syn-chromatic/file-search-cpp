using namespace std;
using namespace filesystem;

void print() {
    std::cout << std::endl;
}

template <typename T, typename... Args>
void print(T first, Args... args) {
    std::cout << first << " ";
    print(args...);
}

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
