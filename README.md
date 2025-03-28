# SimpleFileManager

Консольный файловый менеджер, реализованный на C++ с использованием стандартной библиотеки.

## Описание

SimpleFileManager - это кроссплатформенная консольная утилита для выполнения базовых операций с файлами и директориями. Программа поддерживает основные команды для работы с файловой системой, такие как просмотр содержимого директорий, копирование, перемещение и удаление файлов, создание директорий и поиск файлов.

## Особенности

- Кроссплатформенность (Windows/Linux/macOS)
- Поддержка базовых файловых операций
- Рекурсивное копирование и удаление директорий
- Поиск файлов с поддержкой шаблонов (* и ?)
- Логирование операций в файл

## Инструкция по сборке

### Требования

- C++17 компилятор (GCC, Clang, MSVC)
- CMake 3.10 или выше

### Сборка

```bash
git clone https://github.com/xostapov/SimpleFileManager.git
cd SimpleFileManager

mkdir build
cd build

cmake ..
cmake --build .
```

## Использование

После сборки вы получите исполняемый файл `FileManager`. Запустите его из консоли:

```bash
./FileManager
```

После запуска вы увидите приглашение командной строки с текущей директорией:

```
/path/to/current/directory> 
```

### Доступные команды

| Команда | Описание | Пример использования |
|---------|----------|----------------------|
| ls [path] | Вывод списка файлов и папок | ls ./docs |
| cp <source> <dest> | Копирование файла/папки | cp file.txt backup/file.txt |
| mv <source> <dest> | Перемещение/переименование | mv old.txt new.txt |
| rm <path> | Удаление файла/папки | rm temp.txt |
| mkdir <path> | Создание директории | mkdir new_folder |
| cat <file> | Вывод содержимого файла | cat readme.txt |
| find <dir> <name> | Поиск файла по имени | find ./project "*.cpp" |
| help | Вывод списка команд | help |
| exit | Выход из программы | exit |

## Примеры использования

```
/home/user> ls
[DIR]     Documents                               
[DIR]     Downloads                               
[FILE]    file.txt                                1024 bytes

/home/user> mkdir test_folder
Директория создана: /home/user/test_folder

/home/user> cp file.txt test_folder/file_copy.txt
Файл скопирован: /home/user/file.txt -> /home/user/test_folder/file_copy.txt

/home/user> find . "*.txt"
Найдено 2 файлов:
/home/user/file.txt
/home/user/test_folder/file_copy.txt
```

## Логирование

Все операции логируются в файл `log.txt` в текущей директории. Лог содержит дату, время и описание выполненной операции. 
