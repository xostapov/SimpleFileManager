#ifndef SIMPLE_FILE_MANAGER_H
#define SIMPLE_FILE_MANAGER_H

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <functional>
#include <map>
#include <chrono>

namespace fs = std::filesystem;

// Определение для использования широких строк в Windows
#ifdef OS_WINDOWS
using tstring = std::wstring;
#define tcout std::wcout
#define tcerr std::wcerr
#define tcin std::wcin
#else
using tstring = std::string;
#define tcout std::cout
#define tcerr std::cerr
#define tcin std::cin
#endif

// Вспомогательные функции для преобразования кодировок
tstring toTString(const std::string& str);
std::string toString(const tstring& str);

class Logger {
private:
    std::ofstream logFile;
    
public:
    Logger(const std::string& logPath = "log.txt");
    ~Logger();
    
    void log(const std::string& message);
};

class FileManager {
private:
    fs::path currentPath;
    Logger logger;
    bool running;
    
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands;
    
    void registerCommands();
    std::vector<std::string> parseCommand(const std::string& input);
    
    // Команды файлового менеджера
    void listDirectory(const std::vector<std::string>& args);
    void copyFile(const std::vector<std::string>& args);
    void moveFile(const std::vector<std::string>& args);
    void removeFile(const std::vector<std::string>& args);
    void makeDirectory(const std::vector<std::string>& args);
    void displayFileContent(const std::vector<std::string>& args);
    void findFiles(const std::vector<std::string>& args);
    void showHelp(const std::vector<std::string>& args);
    void exitProgram(const std::vector<std::string>& args);
    
    // Вспомогательные функции
    void copyRecursive(const fs::path& source, const fs::path& dest);
    bool matchWildcard(const std::string& name, const std::string& pattern);
    void findRecursive(const fs::path& dir, const std::string& pattern, std::vector<fs::path>& results);
    
public:
    FileManager();
    void run();
    bool isRunning() const;
};

#endif 