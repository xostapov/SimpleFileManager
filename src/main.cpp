#include "FileManager.h"
#include <iostream>
#include <exception>
#include <locale>

#ifdef OS_WINDOWS
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

int main() {
    try {
#ifdef OS_WINDOWS
        // Настройка кодовой страницы для корректного отображения кириллицы
        SetConsoleOutputCP(1251);
        SetConsoleCP(1251);
        
        // Настройка стандартных потоков для работы с юникодом
        _setmode(_fileno(stdout), _O_U16TEXT);
        _setmode(_fileno(stdin), _O_TEXT);
        _setmode(_fileno(stderr), _O_U16TEXT);
#else
        // Настройка локали с поддержкой UTF-8
        std::locale::global(std::locale("en_US.UTF-8"));
#endif
        
        FileManager fileManager;
        fileManager.run();
    } catch (const std::exception& e) {
        tcerr << toTString("Критическая ошибка: ") << toTString(e.what()) << std::endl;
        return 1;
    }
    
    return 0;
} 