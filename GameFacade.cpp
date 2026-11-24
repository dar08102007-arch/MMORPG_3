#include "GameFacade.h"
#include <iostream>
#include <sstream>

GameFacade::GameFacade() {
    auto base = std::make_unique<BaseHandler>();
    base->setNext(std::make_unique<InfoHandler>());
    first_.setNext(std::move(base));
}

void GameFacade::processInput(const std::string& input) {
    std::istringstream args(input);
    std::string name;
    if (args >> name) {
        std::cout << "> " << input << '\n';
        first_.handle(name, args, *this);
    }
}

void GameFacade::printHelp() const {
    std::cout
        << "create unit <тип> at <x>,<y>\n"
        << "move unit <id> to <x>,<y>\n"
        << "attack unit <id> target <id>\n"
        << "collect unit <id> from <x>,<y>\n"
        << "heal unit <id> target <id>\n"
        << "interact unit <id>\n"
        << "build barracks at <x>,<y>\n"
        << "research attack_upgrade\n"
        << "distribute <количество> to <id-казармы>\n"
        << "produce <тип> in <id-казармы>\n"
        << "stats [units|buildings|resources|research|map]\n"
        << "help, exit\n"
        << "Типы: GondorSoldier UrukHai ElfArcher OrcArcher RohanRider WargRider"
        << " Gandalf Saruman GreatEagle FellBeast\n"
        << "Координаты с нуля. Шаг и атака по соседней клетке.\n"
        << "Сундук собирает юнит на его клетке. Лечат Gandalf и Saruman.\n"
        << "Казарма стоит " << Base::barracksCost << ", улучшение стоит " << Base::researchCost
        << ". Для производства сначала выделите ресурсы казарме.\n";
}
