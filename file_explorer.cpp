#include <iostream>
#include <string>
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <ctime>
#include <map>
#include <limits>

using namespace std;

class ActivityLogger {
private:
    string logFile;
public:
    ActivityLogger() {
        logFile = "file_explorer_activity.log";
    }

    void logActivity(const string& action) {
        ofstream log(logFile, ios::app);
        if (!log.is_open()) return;
        time_t now = time(0);
        char* dt = ctime(&now);
        if (dt) {
            dt[strlen(dt)-1] = '\0'; // remove newline
            log << "[" << dt << "] " << action << "\n";
        } else {
            log << "[unknown time] " << action << "\n";
        }
        log.close();
    }

    void viewHistory() {
        cout << "\nACTIVITY HISTORY\n";
        cout << string(70, '=') << "\n";
        ifstream log(logFile);
        if (!log.is_open()) {
            cout << "No history found.\n";
            cout << string(70, '=') << "\n";
            return;
        }
        vector<string> lines;
        string line;
        while (getline(log, line)) lines.push_back(line);
        int start = max(0, (int)lines.size() - 20);
        for (int i = start; i < lines.size(); ++i) cout << lines[i] << "\n";
        cout << string(70, '=') << "\n";
        log.close();
    }
};

class FileStatistics {
public:
    int totalFiles = 0;
    int totalDirs = 0;
    long long totalSize = 0;
    map<string, int> extensionCount;

    void analyze(const string& path) {
        totalFiles = 0;
        totalDirs = 0;
        totalSize = 0;
        extensionCount.clear();
        analyzeDirectory(path);
    }

    void analyzeDirectory(const string& path) {
        DIR* dir = opendir(path.c_str());
        if (!dir) return;
        struct dirent* entry;
        struct stat fileStat;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            string fullPath = path + "/" + entry->d_name;
            if (stat(fullPath.c_str(), &fileStat) == 0) {
                if (S_ISDIR(fileStat.st_mode)) {
                    totalDirs++;
                    analyzeDirectory(fullPath);
                } else if (S_ISREG(fileStat.st_mode)) {
                    totalFiles++;
                    totalSize += fileStat.st_size;
                    string name = entry->d_name;
                    size_t dotPos = name.find_last_of('.');
                    if (dotPos != string::npos) {
                        string ext = name.substr(dotPos);
                        extensionCount[ext]++;
                    } else {
                        extensionCount["(no_ext)"]++;
                    }
                }
            }
        }
        closedir(dir);
    }

    void display() {
        cout << "\nDIRECTORY STATISTICS DASHBOARD\n";
        cout << string(70, '=') << "\n";
        cout << "Total Directories: " << totalDirs << "\n";
        cout << "Total Files:       " << totalFiles << "\n";
        cout << "Total Size:        " << formatSize(totalSize) << "\n";
        if (!extensionCount.empty()) {
            cout << "\nFile Types Breakdown:\n";
            cout << string(40, '-') << "\n";
            for (auto& p : extensionCount) {
                cout << setw(15) << left << p.first << ": " << p.second << " files\n";
            }
        }
        cout << string(70, '=') << "\n";
    }

    string formatSize(long long bytes) {
        const char* units[] = {"B", "KB", "MB", "GB", "TB"};
        int unitIndex = 0;
        double size = (double)bytes;
        while (size >= 1024.0 && unitIndex < 4) {
            size /= 1024.0;
            unitIndex++;
        }
        ostringstream oss;
        oss << fixed << setprecision(2) << size << " " << units[unitIndex];
        return oss.str();
    }
};

class FileExplorer {
private:
    string currentPath;
    ActivityLogger logger;
    FileStatistics stats;

    void displayHeader() {
        cout << "\n========================\n";
        cout << "File Explorer Application\n";
        cout << "========================\n";
    }

    void displayMenu() {
        cout << "\nBASIC OPERATIONS\n";
        cout << "1.  List Directory Contents\n";
        cout << "2.  Change Directory\n";
        cout << "3.  Create Directory\n";
        cout << "4.  Create File\n";
        cout << "5.  Delete File\n";
        cout << "6.  Delete Directory\n";
        cout << "7.  Copy File\n";
        cout << "8.  Move File\n";
        cout << "9.  Search File (simple)\n";
        cout << "10. View File Permissions\n";
        cout << "11. Change File Permissions\n";
        cout << "12. View File Content\n";
        cout << "13. Show Current Directory\n";
        cout << "\nADVANCED FEATURES\n";
        cout << "14. Directory Statistics Dashboard\n";
        cout << "15. View Activity History\n";
        cout << "16. Advanced Search (with filters)\n";
        cout << "17. Compare Two Files\n";
        cout << "0.  Exit\n";
    }

    void clearInput() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

public:
    FileExplorer() {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            currentPath = string(cwd);
        } else {
            currentPath = ".";
        }
    }

    void listDirectory() {
        cout << "\nCurrent Directory: " << currentPath << "\n";
        cout << string(70, '-') << "\n";
        DIR* dir = opendir(currentPath.c_str());
        if (dir == NULL) {
            cout << "Error opening directory!\n";
            return;
        }
        struct dirent* entry;
        struct stat fileStat;
        cout << left << setw(35) << "Name" << setw(12) << "Type" << setw(15) << "Size" << "Modified\n";
        cout << string(70, '-') << "\n";
        while ((entry = readdir(dir)) != NULL) {
            string fullPath = currentPath + "/" + entry->d_name;
            if (stat(fullPath.c_str(), &fileStat) == 0) {
                string type = S_ISDIR(fileStat.st_mode) ? "DIR" : "FILE";
                string size = S_ISDIR(fileStat.st_mode) ? "-" : stats.formatSize(fileStat.st_size);
                char timeStr[20];
                strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M", localtime(&fileStat.st_mtime));
                cout << left << setw(35) << entry->d_name << setw(12) << type << setw(15) << size << timeStr << "\n";
            }
        }
        closedir(dir);
        cout << string(70, '-') << "\n";
        logger.logActivity("Listed directory: " + currentPath);
    }

    void changeDirectory() {
        cout << "\nEnter directory path (use .. for parent): ";
        clearInput();
        string newPath;
        getline(cin, newPath);
        string oldPath = currentPath;

        if (newPath == "..") {
            size_t pos = currentPath.find_last_of('/');
            if (pos != string::npos && pos > 0) {
                currentPath = currentPath.substr(0, pos);
            } else {
                currentPath = "/";
            }
        } else if (!newPath.empty() && newPath[0] == '/') {
            currentPath = newPath;
        } else if (!newPath.empty()) {
            if (currentPath == "/")
                currentPath = currentPath + newPath;
            else
                currentPath = currentPath + "/" + newPath;
        }

        if (chdir(currentPath.c_str()) == 0) {
            cout << "Changed to: " << currentPath << "\n";
            logger.logActivity("Changed directory from " + oldPath + " to " + currentPath);
        } else {
            cout << "Invalid directory!\n";
            currentPath = oldPath;
        }
    }

    void createDirectory() {
        cout << "\nEnter directory name: ";
        clearInput();
        string dirName;
        getline(cin, dirName);
        if (dirName.empty()) {
            cout << "No name provided.\n";
            return;
        }
        string fullPath = currentPath + "/" + dirName;
        if (mkdir(fullPath.c_str(), 0755) == 0) {
            cout << "Directory created successfully.\n";
            logger.logActivity("Created directory: " + fullPath);
        } else {
            cout << "Error creating directory.\n";
        }
    }

    void createFile() {
        cout << "\nEnter file name: ";
        clearInput();
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "No filename provided.\n";
            return;
        }
        string fullPath = currentPath + "/" + fileName;
        ofstream file(fullPath);
        if (file.is_open()) {
            cout << "Enter content (type a single line with END to finish):\n";
            string line;
            while (true) {
                getline(cin, line);
                if (line == "END") break;
                file << line << "\n";
            }
            file.close();
            cout << "File created successfully.\n";
            logger.logActivity("Created file: " + fullPath);
        } else {
            cout << "Error creating file.\n";
        }
    }

    void deleteFile() {
        cout << "\nEnter file name to delete: ";
        clearInput();
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "No filename provided.\n";
            return;
        }
        string fullPath = currentPath + "/" + fileName;
        if (remove(fullPath.c_str()) == 0) {
            cout << "File deleted successfully.\n";
            logger.logActivity("Deleted file: " + fullPath);
        } else {
            cout << "Error deleting file.\n";
        }
    }

    void deleteDirectory() {
        cout << "\nEnter directory name to delete: ";
        clearInput();
        string dirName;
        getline(cin, dirName);
        if (dirName.empty()) {
            cout << "No name provided.\n";
            return;
        }
        string fullPath = currentPath + "/" + dirName;
        if (rmdir(fullPath.c_str()) == 0) {
            cout << "Directory deleted successfully.\n";
            logger.logActivity("Deleted directory: " + fullPath);
        } else {
            cout << "Error: directory may not be empty or doesn't exist.\n";
        }
    }

    void copyFile() {
        cout << "\nEnter source file name: ";
        clearInput();
        string source;
        getline(cin, source);
        cout << "Enter destination file name: ";
        string destination;
        getline(cin, destination);
        if (source.empty() || destination.empty()) {
            cout << "Source or destination missing.\n";
            return;
        }
        string srcPath = currentPath + "/" + source;
        string destPath = currentPath + "/" + destination;
        ifstream src(srcPath, ios::binary);
        ofstream dest(destPath, ios::binary);
        if (src.is_open() && dest.is_open()) {
            dest << src.rdbuf();
            src.close();
            dest.close();
            cout << "File copied successfully.\n";
            logger.logActivity("Copied: " + srcPath + " to " + destPath);
        } else {
            cout << "Error copying file.\n";
        }
    }

    void moveFile() {
        cout << "\nEnter source file name: ";
        clearInput();
        string source;
        getline(cin, source);
        cout << "Enter destination file name: ";
        string destination;
        getline(cin, destination);
        if (source.empty() || destination.empty()) {
            cout << "Source or destination missing.\n";
            return;
        }
        string srcPath = currentPath + "/" + source;
        string destPath = currentPath + "/" + destination;
        if (rename(srcPath.c_str(), destPath.c_str()) == 0) {
            cout << "File moved successfully.\n";
            logger.logActivity("Moved: " + srcPath + " to " + destPath);
        } else {
            cout << "Error moving file.\n";
        }
    }

    void searchFile() {
        cout << "\nEnter file name to search: ";
        clearInput();
        string searchName;
        getline(cin, searchName);
        if (searchName.empty()) {
            cout << "No search term provided.\n";
            return;
        }
        cout << "\nSearching in: " << currentPath << "\n";
        cout << string(70, '-') << "\n";
        bool found = false;
        searchInDirectory(currentPath, searchName, found);
        if (!found) cout << "No matches found.\n";
        cout << string(70, '-') << "\n";
        logger.logActivity("Searched for: " + searchName);
    }

    void searchInDirectory(const string& path, const string& searchName, bool& found) {
        DIR* dir = opendir(path.c_str());
        if (!dir) return;
        struct dirent* entry;
        struct stat fileStat;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            string fullPath = path + "/" + entry->d_name;
            if (string(entry->d_name).find(searchName) != string::npos) {
                cout << "Found: " << fullPath << "\n";
                found = true;
            }
            if (stat(fullPath.c_str(), &fileStat) == 0 && S_ISDIR(fileStat.st_mode)) {
                searchInDirectory(fullPath, searchName, found);
            }
        }
        closedir(dir);
    }

    void viewPermissions() {
        cout << "\nEnter file name: ";
        clearInput();
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "No filename provided.\n";
            return;
        }
        string fullPath = currentPath + "/" + fileName;
        struct stat fileStat;
        if (stat(fullPath.c_str(), &fileStat) == 0) {
            cout << "\nPermissions for: " << fileName << "\n";
            cout << string(40, '-') << "\n";
            cout << "Owner:  " << ((fileStat.st_mode & S_IRUSR) ? "r" : "-")
                 << ((fileStat.st_mode & S_IWUSR) ? "w" : "-")
                 << ((fileStat.st_mode & S_IXUSR) ? "x" : "-") << "\n";
            cout << "Group:  " << ((fileStat.st_mode & S_IRGRP) ? "r" : "-")
                 << ((fileStat.st_mode & S_IWGRP) ? "w" : "-")
                 << ((fileStat.st_mode & S_IXGRP) ? "x" : "-") << "\n";
            cout << "Others: " << ((fileStat.st_mode & S_IROTH) ? "r" : "-")
                 << ((fileStat.st_mode & S_IWOTH) ? "w" : "-")
                 << ((fileStat.st_mode & S_IXOTH) ? "x" : "-") << "\n";
            cout << "Octal:  " << oct << (fileStat.st_mode & 0777) << dec << "\n";
        } else {
            cout << "Error reading file permissions.\n";
        }
    }

    void changePermissions() {
        cout << "\nEnter file name: ";
        clearInput();
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "No filename provided.\n";
            return;
        }
        cout << "Enter permissions in octal (e.g., 755): ";
        int mode;
        if (!(cin >> oct >> mode)) {
            cout << "Invalid input.\n";
            cin.clear();
            clearInput();
            return;
        }
        clearInput();
        string fullPath = currentPath + "/" + fileName;
        if (chmod(fullPath.c_str(), mode) == 0) {
            cout << "Permissions changed successfully.\n";
            logger.logActivity("Changed permissions of: " + fullPath);
        } else {
            cout << "Error changing permissions.\n";
        }
    }

    void viewFileContent() {
        cout << "\nEnter file name: ";
        clearInput();
        string fileName;
        getline(cin, fileName);
        if (fileName.empty()) {
            cout << "No filename provided.\n";
            return;
        }
        string fullPath = currentPath + "/" + fileName;
        ifstream file(fullPath);
        if (!file.is_open()) {
            cout << "Error opening file.\n";
            return;
        }
        cout << "\n" << string(70, '=') << "\n";
        cout << "Content of: " << fileName << "\n";
        cout << string(70, '=') << "\n";
        string line;
        int lineNum = 1;
        while (getline(file, line)) {
            cout << setw(4) << lineNum++ << " | " << line << "\n";
        }
        cout << string(70, '=') << "\n";
        file.close();
        logger.logActivity("Viewed file: " + fullPath);
    }

    void showCurrentDirectory() {
        cout << "\nCurrent Directory: " << currentPath << "\n";
    }

    // Advanced features kept
    void showStatistics() {
        cout << "\nAnalyzing directory tree...\n";
        stats.analyze(currentPath);
        stats.display();
        logger.logActivity("Generated statistics for: " + currentPath);
    }

    void viewActivityHistory() {
        logger.viewHistory();
    }

    void advancedSearch() {
        cout << "\nADVANCED SEARCH\n";
        clearInput();
        cout << "Enter filename pattern (or * for all): ";
        string pattern;
        getline(cin, pattern);
        cout << "Filter by extension (e.g., .txt) or press Enter to skip: ";
        string extension;
        getline(cin, extension);
        cout << "Minimum size in bytes (0 for no limit): ";
        long long minSize = 0, maxSize = 0;
        if (!(cin >> minSize)) minSize = 0;
        cout << "Maximum size in bytes (0 for no limit): ";
        if (!(cin >> maxSize)) maxSize = 0;
        clearInput();
        if (maxSize == 0) maxSize = numeric_limits<long long>::max();

        cout << "\nSearching with filters...\n";
        cout << string(70, '-') << "\n";
        bool found = false;
        advancedSearchInDirectory(currentPath, pattern.empty() ? "*" : pattern, extension, minSize, maxSize, found);
        if (!found) cout << "No files found matching criteria.\n";
        cout << string(70, '-') << "\n";
        logger.logActivity("Advanced search performed");
    }

    void advancedSearchInDirectory(const string& path, const string& pattern,
                                   const string& ext, long long minSize, long long maxSize, bool& found) {
        DIR* dir = opendir(path.c_str());
        if (!dir) return;
        struct dirent* entry;
        struct stat fileStat;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            string fullPath = path + "/" + entry->d_name;
            string name = entry->d_name;
            if (stat(fullPath.c_str(), &fileStat) == 0) {
                if (S_ISREG(fileStat.st_mode)) {
                    bool matches = true;
                    if (pattern != "*" && !pattern.empty() && name.find(pattern) == string::npos) matches = false;
                    if (!ext.empty() && name.find(ext) == string::npos) matches = false;
                    if (fileStat.st_size < minSize || fileStat.st_size > maxSize) matches = false;
                    if (matches) {
                        cout << fullPath << " (" << stats.formatSize(fileStat.st_size) << ")\n";
                        found = true;
                    }
                } else if (S_ISDIR(fileStat.st_mode)) {
                    advancedSearchInDirectory(fullPath, pattern, ext, minSize, maxSize, found);
                }
            }
        }
        closedir(dir);
    }

    void compareFiles() {
        cout << "\nFILE COMPARISON TOOL\n";
        clearInput();
        cout << "Enter first file name: ";
        string file1;
        getline(cin, file1);
        cout << "Enter second file name: ";
        string file2;
        getline(cin, file2);
        if (file1.empty() || file2.empty()) {
            cout << "File names missing.\n";
            return;
        }
        string path1 = currentPath + "/" + file1;
        string path2 = currentPath + "/" + file2;
        ifstream f1(path1);
        ifstream f2(path2);
        if (!f1.is_open() || !f2.is_open()) {
            cout << "Error opening one or both files.\n";
            return;
        }

        f1.seekg(0, ios::end);
        f2.seekg(0, ios::end);
        long size1 = f1.tellg();
        long size2 = f2.tellg();
        cout << "\nCOMPARISON RESULTS\n";
        cout << string(70, '=') << "\n";
        cout << "File 1 size: " << stats.formatSize(size1) << "\n";
        cout << "File 2 size: " << stats.formatSize(size2) << "\n";
        f1.seekg(0, ios::beg);
        f2.seekg(0, ios::beg);

        string line1, line2;
        int lineNum = 1;
        int differences = 0;
        bool identical = true;
        while (getline(f1, line1) && getline(f2, line2)) {
            if (line1 != line2) {
                if (differences < 10) {
                    cout << "\nDifference at line " << lineNum << ":\n";
                    cout << "  File 1: " << line1 << "\n";
                    cout << "  File 2: " << line2 << "\n";
                }
                differences++;
                identical = false;
            }
            lineNum++;
        }

        // If one file has extra lines
        bool extra = false;
        if (getline(f1, line1) || getline(f2, line2)) extra = true;
        if (extra) {
            identical = false;
            cout << "\nFiles have different number of lines.\n";
        }

        cout << "\n" << string(70, '-') << "\n";
        if (identical) cout << "Files are IDENTICAL.\n";
        else cout << "Files are DIFFERENT (" << differences << " differences shown up to first 10).\n";
        cout << string(70, '=') << "\n";

        f1.close();
        f2.close();
        logger.logActivity("Compared files: " + file1 + " and " + file2);
    }

    void run() {
        displayHeader();
        int choice = -1;
        do {
            displayMenu();
            cout << "\nEnter your choice: ";
            if (!(cin >> choice)) {
                cin.clear();
                clearInput();
                cout << "Invalid input. Please enter a number.\n";
                continue;
            }
            switch (choice) {
                case 1: listDirectory(); break;
                case 2: changeDirectory(); break;
                case 3: createDirectory(); break;
                case 4: createFile(); break;
                case 5: deleteFile(); break;
                case 6: deleteDirectory(); break;
                case 7: copyFile(); break;
                case 8: moveFile(); break;
                case 9: searchFile(); break;
                case 10: viewPermissions(); break;
                case 11: changePermissions(); break;
                case 12: viewFileContent(); break;
                case 13: showCurrentDirectory(); break;
                case 14: showStatistics(); break;
                case 15: viewActivityHistory(); break;
                case 16: advancedSearch(); break;
                case 17: compareFiles(); break;
                case 0:
                    cout << "\nThank you for using File Explorer Application.\n";
                    logger.logActivity("Application closed");
                    break;
                default:
                    cout << "Invalid choice. Please try again.\n";
            }
        } while (choice != 0);
    }
};

int main() {
    FileExplorer explorer;
    explorer.run();
    return 0;
}
