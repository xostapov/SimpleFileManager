#include "FileManager.h"
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <ctime>

#ifdef OS_WINDOWS
#include <windows.h>
#include <codecvt>
#include <locale>

// Преобразование из UTF-8 в wstring
tstring toTString(const std::string& str) {
    if (str.empty()) return L"";
    
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Преобразование из wstring в UTF-8
std::string toString(const tstring& wstr) {
    if (wstr.empty()) return "";
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
#else
// В других ОС просто возвращаем ту же строку
tstring toTString(const std::string& str) {
    return str;
}

std::string toString(const tstring& str) {
    return str;
}
#endif

// Реализация класса Logger
Logger::Logger(const std::string& logPath) {
    logFile.open(logPath, std::ios::app);
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(const std::string& message) {
    if (!logFile.is_open()) return;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::tm timeInfo;
#ifdef OS_WINDOWS
    localtime_s(&timeInfo, &time);
#else
    localtime_r(&time, &timeInfo);
#endif
    
    logFile << std::put_time(&timeInfo, "[%Y-%m-%d %H:%M:%S] ") << message << std::endl;
}

// Реализация класса FileManager
FileManager::FileManager() : running(true) {
    currentPath = fs::current_path();
    registerCommands();
}

void FileManager::registerCommands() {
    commands["ls"] = [this](const std::vector<std::string>& args) { this->listDirectory(args); };
    commands["cp"] = [this](const std::vector<std::string>& args) { this->copyFile(args); };
    commands["mv"] = [this](const std::vector<std::string>& args) { this->moveFile(args); };
    commands["rm"] = [this](const std::vector<std::string>& args) { this->removeFile(args); };
    commands["mkdir"] = [this](const std::vector<std::string>& args) { this->makeDirectory(args); };
    commands["cat"] = [this](const std::vector<std::string>& args) { this->displayFileContent(args); };
    commands["find"] = [this](const std::vector<std::string>& args) { this->findFiles(args); };
    commands["help"] = [this](const std::vector<std::string>& args) { this->showHelp(args); };
    commands["exit"] = [this](const std::vector<std::string>& args) { this->exitProgram(args); };
}

std::vector<std::string> FileManager::parseCommand(const std::string& input) {
    std::vector<std::string> args;
    std::string arg;
    bool inQuotes = false;
    
    for (char c : input) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ' ' && !inQuotes) {
            if (!arg.empty()) {
                args.push_back(arg);
                arg.clear();
            }
        } else {
            arg += c;
        }
    }
    
    if (!arg.empty()) {
        args.push_back(arg);
    }
    
    return args;
}

void FileManager::run() {
    std::string input;
    
    tcout << toTString("SimpleFileManager запущен. Введите 'help' для справки.\n");
    
    while (running) {
        tcout << toTString(currentPath.string() + "> ");
        std::getline(std::cin, input);
        
        if (input.empty()) continue;
        
        std::vector<std::string> args = parseCommand(input);
        std::string cmd = args[0];
        args.erase(args.begin());
        
        if (commands.find(cmd) != commands.end()) {
            try {
                commands[cmd](args);
                logger.log("Выполнена команда: " + input);
            } catch (const std::exception& e) {
                tcerr << toTString("Ошибка: ") << toTString(e.what()) << std::endl;
                logger.log("Ошибка при выполнении команды: " + input + " - " + e.what());
            }
        } else {
            tcout << toTString("Неизвестная команда. Введите 'help' для справки.\n");
        }
    }
}

bool FileManager::isRunning() const {
    return running;
}

// Реализация команд
void FileManager::listDirectory(const std::vector<std::string>& args) {
    fs::path path = currentPath;
    
    if (!args.empty()) {
        path = fs::path(args[0]);
        if (path.is_relative()) {
            path = currentPath / path;
        }
    }
    
    if (!fs::exists(path)) {
        tcerr << toTString("Путь не существует: " + path.string()) << std::endl;
        return;
    }
    
    if (!fs::is_directory(path)) {
        tcerr << toTString("Указанный путь не является директорией: " + path.string()) << std::endl;
        return;
    }
    
    tcout << toTString("Содержимое директории: " + path.string()) << std::endl;
    
    for (const auto& entry : fs::directory_iterator(path)) {
        std::string entryType = entry.is_directory() ? "[DIR]" : "[FILE]";
        std::string entrySize = entry.is_directory() ? "" : 
                               std::to_string(fs::file_size(entry)) + " bytes";
        
        // Исправленное форматирование вывода
        std::string output = entryType;
        while (output.length() < 10) output += " ";
        output += entry.path().filename().string();
        while (output.length() < 50) output += " ";
        output += entrySize;
        
        tcout << toTString(output) << std::endl;
    }
}

void FileManager::copyFile(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        tcerr << toTString("Использование: cp <источник> <назначение>") << std::endl;
        return;
    }
    
    fs::path source = args[0];
    fs::path dest = args[1];
    
    if (source.is_relative()) {
        source = currentPath / source;
    }
    
    if (dest.is_relative()) {
        dest = currentPath / dest;
    }
    
    if (!fs::exists(source)) {
        tcerr << toTString("Источник не существует: " + source.string()) << std::endl;
        return;
    }
    
    try {
        if (fs::is_directory(source)) {
            if (!fs::exists(dest)) {
                fs::create_directories(dest);
            } else if (!fs::is_directory(dest)) {
                tcerr << toTString("Назначение должно быть директорией: " + dest.string()) << std::endl;
                return;
            }
            
            copyRecursive(source, dest);
            tcout << toTString("Директория скопирована: " + source.string() + " -> " + dest.string()) << std::endl;
        } else {
            if (fs::is_directory(dest)) {
                dest = dest / source.filename();
            }
            
            fs::copy_file(source, dest, fs::copy_options::overwrite_existing);
            tcout << toTString("Файл скопирован: " + source.string() + " -> " + dest.string()) << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        tcerr << toTString("Ошибка копирования: ") << toTString(e.what()) << std::endl;
    }
}

void FileManager::copyRecursive(const fs::path& source, const fs::path& dest) {
    try {
        for (const auto& entry : fs::directory_iterator(source)) {
            fs::path currentDest = dest / entry.path().filename();
            
            if (fs::is_directory(entry)) {
                if (!fs::exists(currentDest)) {
                    fs::create_directories(currentDest);
                }
                copyRecursive(entry.path(), currentDest);
            } else {
                fs::copy_file(entry.path(), currentDest, fs::copy_options::overwrite_existing);
            }
        }
    } catch (const fs::filesystem_error& e) {
        tcerr << toTString("Ошибка при рекурсивном копировании: ") << toTString(e.what()) << std::endl;
        throw;
    }
}

void FileManager::moveFile(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        tcerr << toTString("Использование: mv <источник> <назначение>") << std::endl;
        return;
    }
    
    fs::path source = args[0];
    fs::path dest = args[1];
    
    if (source.is_relative()) {
        source = currentPath / source;
    }
    
    if (dest.is_relative()) {
        dest = currentPath / dest;
    }
    
    if (!fs::exists(source)) {
        tcerr << toTString("Источник не существует: " + source.string()) << std::endl;
        return;
    }
    
    try {
        if (fs::is_directory(dest) && !fs::is_directory(source)) {
            dest = dest / source.filename();
        }
        
        fs::rename(source, dest);
        tcout << toTString("Файл перемещен/переименован: " + source.string() + " -> " + dest.string()) << std::endl;
    } catch (const fs::filesystem_error& e) {
        tcerr << toTString("Ошибка перемещения: ") << toTString(e.what()) << std::endl;
    }
}

void FileManager::removeFile(const std::vector<std::string>& args) {
    if (args.empty()) {
        tcerr << toTString("Использование: rm <путь>") << std::endl;
        return;
    }
    
    fs::path path = args[0];
    
    if (path.is_relative()) {
        path = currentPath / path;
    }
    
    if (!fs::exists(path)) {
        tcerr << toTString("Файл не существует: " + path.string()) << std::endl;
        return;
    }
    
    try {
        if (fs::is_directory(path)) {
            std::uintmax_t count = fs::remove_all(path);
            tcout << toTString("Удалена директория и " + std::to_string(count) + " элементов.") << std::endl;
        } else {
            fs::remove(path);
            tcout << toTString("Файл удален: " + path.string()) << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        tcerr << toTString("Ошибка удаления: ") << toTString(e.what()) << std::endl;
    }
}

void FileManager::makeDirectory(const std::vector<std::string>& args) {
    if (args.empty()) {
        tcerr << toTString("Использование: mkdir <путь>") << std::endl;
        return;
    }
    
    fs::path path = args[0];
    
    if (path.is_relative()) {
        path = currentPath / path;
    }
    
    try {
        if (fs::create_directories(path)) {
            tcout << toTString("Директория создана: " + path.string()) << std::endl;
        } else {
            tcout << toTString("Директория уже существует: " + path.string()) << std::endl;
        }
    } catch (const fs::filesystem_error& e) {
        tcerr << toTString("Ошибка создания директории: ") << toTString(e.what()) << std::endl;
    }
}

void FileManager::displayFileContent(const std::vector<std::string>& args) {
    if (args.empty()) {
        tcerr << toTString("Использование: cat <файл>") << std::endl;
        return;
    }
    
    fs::path path = args[0];
    
    if (path.is_relative()) {
        path = currentPath / path;
    }
    
    if (!fs::exists(path)) {
        tcerr << toTString("Файл не существует: " + path.string()) << std::endl;
        return;
    }
    
    if (fs::is_directory(path)) {
        tcerr << toTString("Путь указывает на директорию, а не файл: " + path.string()) << std::endl;
        return;
    }
    
    try {
        std::ifstream file(path);
        if (!file) {
            tcerr << toTString("Не удалось открыть файл: " + path.string()) << std::endl;
            return;
        }
        
        tcout << toTString("Содержимое файла: " + path.string()) << std::endl;
        tcout << toTString("----------------------------------------") << std::endl;
        
        std::string line;
        while (std::getline(file, line)) {
            tcout << toTString(line) << std::endl;
        }
        
        tcout << toTString("----------------------------------------") << std::endl;
        
    } catch (const std::exception& e) {
        tcerr << toTString("Ошибка чтения файла: ") << toTString(e.what()) << std::endl;
    }
}

bool FileManager::matchWildcard(const std::string& name, const std::string& pattern) {
    if (pattern.empty()) return name.empty();
    
    std::size_t nameIndex = 0;
    std::size_t patternIndex = 0;
    std::size_t starIndex = std::string::npos;
    std::size_t matchIndex = 0;
    
    while (nameIndex < name.size()) {
        if (patternIndex < pattern.size() && 
            (pattern[patternIndex] == '?' || pattern[patternIndex] == name[nameIndex])) {
            ++nameIndex;
            ++patternIndex;
        } else if (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
            starIndex = patternIndex;
            matchIndex = nameIndex;
            ++patternIndex;
        } else if (starIndex != std::string::npos) {
            patternIndex = starIndex + 1;
            nameIndex = ++matchIndex;
        } else {
            return false;
        }
    }
    
    while (patternIndex < pattern.size() && pattern[patternIndex] == '*') {
        ++patternIndex;
    }
    
    return patternIndex == pattern.size();
}

void FileManager::findRecursive(const fs::path& dir, const std::string& pattern, std::vector<fs::path>& results) {
    try {
        for (const auto& entry : fs::directory_iterator(dir)) {
            std::string filename = entry.path().filename().string();
            
            if (matchWildcard(filename, pattern)) {
                results.push_back(entry.path());
            }
            
            if (fs::is_directory(entry)) {
                findRecursive(entry.path(), pattern, results);
            }
        }
    } catch (const fs::filesystem_error& e) {
        tcerr << toTString("Ошибка поиска в директории " + dir.string() + ": ") << toTString(e.what()) << std::endl;
    }
}

void FileManager::findFiles(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        tcerr << toTString("Использование: find <директория> <шаблон>") << std::endl;
        return;
    }
    
    fs::path dir = args[0];
    std::string pattern = args[1];
    
    if (dir.is_relative()) {
        dir = currentPath / dir;
    }
    
    if (!fs::exists(dir)) {
        tcerr << toTString("Директория не существует: " + dir.string()) << std::endl;
        return;
    }
    
    if (!fs::is_directory(dir)) {
        tcerr << toTString("Указанный путь не является директорией: " + dir.string()) << std::endl;
        return;
    }
    
    std::vector<fs::path> results;
    
    try {
        findRecursive(dir, pattern, results);
        
        if (results.empty()) {
            tcout << toTString("Файлы, соответствующие шаблону '" + pattern + "', не найдены.") << std::endl;
        } else {
            tcout << toTString("Найдено " + std::to_string(results.size()) + " файлов:") << std::endl;
            for (const auto& path : results) {
                tcout << toTString(path.string()) << std::endl;
            }
        }
    } catch (const std::exception& e) {
        tcerr << toTString("Ошибка поиска: ") << toTString(e.what()) << std::endl;
    }
}

void FileManager::showHelp(const std::vector<std::string>& args) {
    tcout << toTString("SimpleFileManager - Консольный файловый менеджер\n"
               "Доступные команды:\n"
               "  ls [path]               - Вывод списка файлов и папок\n"
               "  cp <source> <dest>      - Копирование файла/папки\n"
               "  mv <source> <dest>      - Перемещение/переименование\n"
               "  rm <path>               - Удаление файла/папки\n"
               "  mkdir <path>            - Создание директории\n"
               "  cat <file>              - Вывод содержимого файла\n"
               "  find <dir> <name>       - Поиск файла по имени (поддерживает *, ?)\n"
               "  help                    - Вывод списка команд\n"
               "  exit                    - Выход из программы\n") << std::endl;
}

void FileManager::exitProgram(const std::vector<std::string>& args) {
    running = false;
    tcout << toTString("Выход из программы...\n") << std::endl;
} 