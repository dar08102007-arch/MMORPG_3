#include <iostream>
#include <sstream>
#include "GameFacade.h"
#ifdef _WIN32
#include <windows.h>
#endif
int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    if (argc == 2 && std::string(argv[1]) == "--demo") {
        GameController game;
        game.runDemos();
        return 0;
    }
    if (argc != 1) {
        std::cerr << "Запуск: mmo [--demo]\n";
        return 1;
    }

    std::cout << "СРЕДИЗЕМЬЕ. Список команд: help. Выход: exit.\n";
    GameFacade game;
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream input(line);
        std::string name, extra;
        if ((input >> name) && name == "exit" && !(input >> extra)) {
            std::cout << "> " << line << "\nИгра завершена.\n";
            break;
        }
        game.processInput(line);
    }
    return 0;
}
