using namespace std;
using namespace filesystem;

class SearchProgress {
private:
    size_t search_counter = 0;
    size_t match_counter = 0;
    size_t previous_length = 0;
    size_t search_bytes = 0;
    chrono::steady_clock::time_point time;

public:
    SearchProgress() {
        time = chrono::steady_clock::now();
    }

    void increment_search() {
        search_counter += 1;
    }

    void increment_match() {
        match_counter += 1;
    }

    void increment_search_bytes(const directory_entry &entry) {
        uintmax_t bytes = entry.file_size();
        this->search_bytes += bytes;
    }

    void show_progress() {
        if (search_counter % 500 == 0) {
            write_progress();
        }
    }

    void show_root_search(path &root) {
        string formatted_path = "[" + root.string() + "]";
        cout << "Searching In: " << formatted_path << std::endl;
    }

    void finalize() {
        write_progress();
        cout << endl;
    }

    void reset() {
        search_counter = 0;
        match_counter = 0;
        previous_length = 0;
    }

private:
    void write_progress() {
        string match_string = std::to_string(match_counter);
        string search_string = std::to_string(search_counter);
        string size_string = this->format_size(this->search_bytes);
        uint64_t elapsed_time = chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now() - time).count();
        string time_string = this->format_time(elapsed_time);

        cout
            << "\rMatches: " << match_string << " | "
            << "Searches: " << search_string << " | "
            << "Search Size: " << size_string << " | "
            << "Elapsed Time: " << time_string;

        size_t length = match_string.length() + search_string.length() + time_string.length() + 36;
        write_fill_string(length);
        previous_length = length;
    }

    void write_fill_string(size_t length) {
        size_t fill = get_fill(length);
        string fill_string(fill, ' ');
        cout << fill_string;
    }

    size_t get_fill(size_t length) {
        if (previous_length >= length) {
            return previous_length - length;
        }
        return 0;
    }

    string format_size(size_t bytes) {
        const double KB = static_cast<double>(1ull << 10);
        const double MB = static_cast<double>(1ull << 20);
        const double GB = static_cast<double>(1ull << 30);
        const double TB = static_cast<double>(1ull << 40);

        double bytes_dbl = static_cast<double>(bytes);
        stringstream ss;
        ss << std::fixed << std::setprecision(2);

        if (bytes_dbl <= KB)
            ss << bytes_dbl << " B";
        else if (bytes_dbl < MB)
            ss << bytes_dbl / KB << " KB";
        else if (bytes_dbl < GB)
            ss << bytes_dbl / MB << " MB";
        else if (bytes_dbl < TB)
            ss << bytes_dbl / GB << " GB";
        else
            ss << bytes_dbl / TB << " TB";

        return ss.str();
    }

    string format_time(uint64_t nanoseconds) {
        const double US = 1'000.0;
        const double MS = 1'000'000.0;
        const double S = 1'000'000'000.0;

        ostringstream time;
        if (nanoseconds < US) {
            time << std::fixed << std::setprecision(2) << nanoseconds << " ns";
        } else if (nanoseconds < MS) {
            time << std::fixed << std::setprecision(2) << nanoseconds / US << " µs";
        } else if (nanoseconds < S) {
            time << std::fixed << std::setprecision(2) << nanoseconds / MS << " ms";
        } else {
            time << std::fixed << std::setprecision(2) << nanoseconds / S << " s";
        }
        return time.str();
    }
};
