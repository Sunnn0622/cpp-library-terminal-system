#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

class LibraryItem {
protected:
    int id;
    string title;
    int year;
    bool borrowed;

public:
    LibraryItem(int id, string title, int year, bool borrowed)
        : id(id), title(std::move(title)), year(year), borrowed(borrowed) {}

    virtual ~LibraryItem() = default;

    int getId() const { return id; }
    string getTitle() const { return title; }
    int getYear() const { return year; }
    bool isBorrowed() const { return borrowed; }

    void borrowItem() { borrowed = true; }
    void returnItem() { borrowed = false; }

    virtual string type() const = 0;
    virtual string detail() const = 0;
    virtual unique_ptr<LibraryItem> clone() const = 0;

    virtual string toCsv() const {
        return type() + "," + to_string(id) + "," + escapeCsv(title) + "," +
               to_string(year) + "," + (borrowed ? "1" : "0") + "," + detail();
    }

    static string escapeCsv(const string &value) {
        bool needsQuotes = value.find(',') != string::npos || value.find('"') != string::npos;
        if (!needsQuotes) {
            return value;
        }

        string result = "\"";
        for (char ch : value) {
            if (ch == '"') {
                result += "\"\"";
            } else {
                result += ch;
            }
        }
        result += "\"";
        return result;
    }
};

class Book : public LibraryItem {
    string author;

public:
    Book(int id, string title, int year, bool borrowed, string author)
        : LibraryItem(id, std::move(title), year, borrowed), author(std::move(author)) {}

    string type() const override { return "Book"; }
    string detail() const override { return LibraryItem::escapeCsv(author); }
    unique_ptr<LibraryItem> clone() const override { return make_unique<Book>(*this); }
};

class Magazine : public LibraryItem {
    string issue;

public:
    Magazine(int id, string title, int year, bool borrowed, string issue)
        : LibraryItem(id, std::move(title), year, borrowed), issue(std::move(issue)) {}

    string type() const override { return "Magazine"; }
    string detail() const override { return LibraryItem::escapeCsv(issue); }
    unique_ptr<LibraryItem> clone() const override { return make_unique<Magazine>(*this); }
};

class MediaItem : public LibraryItem {
    string format;

public:
    MediaItem(int id, string title, int year, bool borrowed, string format)
        : LibraryItem(id, std::move(title), year, borrowed), format(std::move(format)) {}

    string type() const override { return "Media"; }
    string detail() const override { return LibraryItem::escapeCsv(format); }
    unique_ptr<LibraryItem> clone() const override { return make_unique<MediaItem>(*this); }
};

vector<string> parseCsvLine(const string &line) {
    vector<string> fields;
    string current;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i) {
        char ch = line[i];
        if (ch == '"') {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                current += '"';
                ++i;
            } else {
                inQuotes = !inQuotes;
            }
        } else if (ch == ',' && !inQuotes) {
            fields.push_back(current);
            current.clear();
        } else {
            current += ch;
        }
    }
    fields.push_back(current);
    return fields;
}

class LibrarySystem {
    vector<unique_ptr<LibraryItem>> items;
    string dataPath;

public:
    explicit LibrarySystem(string dataPath) : dataPath(std::move(dataPath)) {}

    void load() {
        items.clear();
        ifstream file(dataPath);
        if (!file) {
            seedData();
            save();
            return;
        }

        string line;
        while (getline(file, line)) {
            if (line.empty()) {
                continue;
            }
            vector<string> f = parseCsvLine(line);
            if (f.size() < 6) {
                continue;
            }

            string itemType = f[0];
            int id = stoi(f[1]);
            string title = f[2];
            int year = stoi(f[3]);
            bool borrowed = f[4] == "1";
            string extra = f[5];

            if (itemType == "Book") {
                items.push_back(make_unique<Book>(id, title, year, borrowed, extra));
            } else if (itemType == "Magazine") {
                items.push_back(make_unique<Magazine>(id, title, year, borrowed, extra));
            } else if (itemType == "Media") {
                items.push_back(make_unique<MediaItem>(id, title, year, borrowed, extra));
            }
        }
    }

    void save() const {
        ofstream file(dataPath);
        if (!file) {
            throw runtime_error("Cannot write data file: " + dataPath);
        }
        for (const auto &item : items) {
            file << item->toCsv() << '\n';
        }
    }

    void seedData() {
        items.push_back(make_unique<Book>(1001, "Clean Code", 2008, false, "Robert C. Martin"));
        items.push_back(make_unique<Book>(1002, "Effective C++", 2005, false, "Scott Meyers"));
        items.push_back(make_unique<Magazine>(2001, "National Geographic", 2024, true, "2024-08"));
        items.push_back(make_unique<MediaItem>(3001, "C++ STL Tutorial", 2023, false, "Video"));
    }

    int nextId() const {
        int maxId = 1000;
        for (const auto &item : items) {
            maxId = max(maxId, item->getId());
        }
        return maxId + 1;
    }

    void addItem(unique_ptr<LibraryItem> item) {
        items.push_back(std::move(item));
        sortById();
    }

    LibraryItem *findById(int id) {
        auto it = find_if(items.begin(), items.end(), [id](const auto &item) {
            return item->getId() == id;
        });
        return it == items.end() ? nullptr : it->get();
    }

    vector<LibraryItem *> searchByTitle(const string &keyword) {
        vector<LibraryItem *> result;
        for (auto &item : items) {
            string title = item->getTitle();
            string key = keyword;
            transform(title.begin(), title.end(), title.begin(), ::tolower);
            transform(key.begin(), key.end(), key.begin(), ::tolower);
            if (title.find(key) != string::npos) {
                result.push_back(item.get());
            }
        }
        return result;
    }

    void sortById() {
        sort(items.begin(), items.end(), [](const auto &a, const auto &b) {
            return a->getId() < b->getId();
        });
    }

    map<string, int> countByType() const {
        map<string, int> counts;
        for (const auto &item : items) {
            counts[item->type()]++;
        }
        return counts;
    }

    const vector<unique_ptr<LibraryItem>> &allItems() const { return items; }
};

int readInt(const string &prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "輸入格式錯誤，請輸入數字。\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

string readLine(const string &prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

void printItemTable(const vector<LibraryItem *> &items) {
    cout << left << setw(8) << "ID" << setw(12) << "類型" << setw(28) << "名稱"
         << setw(8) << "年份" << setw(10) << "狀態" << "作者/期別/格式\n";
    cout << string(82, '-') << '\n';
    for (const auto *item : items) {
        cout << left << setw(8) << item->getId()
             << setw(12) << item->type()
             << setw(28) << item->getTitle().substr(0, 26)
             << setw(8) << item->getYear()
             << setw(10) << (item->isBorrowed() ? "借出" : "可借")
             << item->detail() << '\n';
    }
}

void listItems(const LibrarySystem &system) {
    vector<LibraryItem *> rows;
    for (const auto &item : system.allItems()) {
        rows.push_back(item.get());
    }
    printItemTable(rows);
}

void addItemFlow(LibrarySystem &system) {
    cout << "\n新增館藏類型：1. 書籍  2. 雜誌  3. 影音媒體\n";
    int type = readInt("請選擇：");
    string title = readLine("名稱：");
    int year = readInt("年份：");
    string extra;
    int id = system.nextId();

    if (type == 1) {
        extra = readLine("作者：");
        system.addItem(make_unique<Book>(id, title, year, false, extra));
    } else if (type == 2) {
        extra = readLine("期別：");
        system.addItem(make_unique<Magazine>(id, title, year, false, extra));
    } else if (type == 3) {
        extra = readLine("格式：");
        system.addItem(make_unique<MediaItem>(id, title, year, false, extra));
    } else {
        cout << "沒有這個類型。\n";
        return;
    }

    system.save();
    cout << "新增成功，館藏 ID：" << id << "\n";
}

void borrowReturnFlow(LibrarySystem &system, bool borrow) {
    int id = readInt(borrow ? "請輸入要借出的 ID：" : "請輸入要歸還的 ID：");
    LibraryItem *item = system.findById(id);
    if (!item) {
        cout << "找不到此館藏。\n";
        return;
    }
    if (borrow && item->isBorrowed()) {
        cout << "此館藏已被借出。\n";
        return;
    }
    if (!borrow && !item->isBorrowed()) {
        cout << "此館藏目前沒有被借出。\n";
        return;
    }

    borrow ? item->borrowItem() : item->returnItem();
    system.save();
    cout << (borrow ? "借出完成：" : "歸還完成：") << item->getTitle() << "\n";
}

void searchFlow(LibrarySystem &system) {
    string keyword = readLine("請輸入查詢關鍵字：");
    vector<LibraryItem *> result = system.searchByTitle(keyword);
    if (result.empty()) {
        cout << "沒有找到符合的館藏。\n";
        return;
    }
    printItemTable(result);
}

void statsFlow(const LibrarySystem &system) {
    map<string, int> counts = system.countByType();
    cout << "\n館藏統計\n";
    cout << "--------------------\n";
    for (const auto &[type, count] : counts) {
        cout << setw(12) << left << type << count << " 筆\n";
    }
}

void printMenu() {
    cout << "\n========================================\n";
    cout << "  智慧圖書館借閱管理系統\n";
    cout << "========================================\n";
    cout << "1. 顯示全部館藏\n";
    cout << "2. 新增館藏\n";
    cout << "3. 借出館藏\n";
    cout << "4. 歸還館藏\n";
    cout << "5. 依名稱搜尋\n";
    cout << "6. 館藏統計\n";
    cout << "0. 儲存並離開\n";
    cout << "========================================\n";
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    LibrarySystem system("data/library_items.csv");
    try {
        system.load();
    } catch (const exception &ex) {
        cerr << "讀取資料失敗：" << ex.what() << '\n';
        return 1;
    }

    while (true) {
        printMenu();
        int choice = readInt("請輸入功能：");
        try {
            switch (choice) {
            case 1:
                listItems(system);
                break;
            case 2:
                addItemFlow(system);
                break;
            case 3:
                borrowReturnFlow(system, true);
                break;
            case 4:
                borrowReturnFlow(system, false);
                break;
            case 5:
                searchFlow(system);
                break;
            case 6:
                statsFlow(system);
                break;
            case 0:
                system.save();
                cout << "資料已儲存，謝謝使用。\n";
                return 0;
            default:
                cout << "沒有這個功能，請重新選擇。\n";
            }
        } catch (const exception &ex) {
            cout << "操作失敗：" << ex.what() << '\n';
        }
    }
}
