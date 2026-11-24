#include "Handler.h"
#include "GameFacade.h"
#include <iostream>
#include <sstream>

static bool word(std::istream& args, const std::string& expected) {
    std::string value;
    return (args >> value) && value == expected;
}

static bool number(const std::string& value, int& result) {
    std::istringstream input(value);
    return (input >> result) && input.eof();
}

static bool positiveNumber(std::istream& args, int& result) {
    std::string value;
    return (args >> value) && number(value, result) && result > 0;
}

static bool position(std::istream& args, int& x, int& y) {
    std::string value;
    if (!(args >> value)) {
        return false;
    }
    auto comma = value.find(',');
    return comma != std::string::npos
        && number(value.substr(0, comma), x) && number(value.substr(comma + 1), y);
}

static bool unitKind(std::istream& args, UnitKind& result) {
    std::string value;
    if (!(args >> value)) {
        return false;
    }
    const UnitKind kinds[] = {
        UnitKind::GondorSoldier, UnitKind::UrukHai, UnitKind::ElfArcher, UnitKind::OrcArcher,
        UnitKind::RohanRider, UnitKind::WargRider, UnitKind::Gandalf, UnitKind::Saruman,
        UnitKind::GreatEagle, UnitKind::FellBeast
    };
    for (UnitKind kind : kinds) {
        if (UnitTypeInfo::getInfo(kind).name == value) {
            result = kind;
            return true;
        }
    }
    return false;
}

void Handler::setNext(std::unique_ptr<Handler> next) {
    next_ = std::move(next);
}

void Handler::handle(const std::string& name, std::istream& args, GameFacade& game) const {
    if (!accepts(name)) {
        if (next_) {
            next_->handle(name, args, game);
        } else {
            std::cout << "Неизвестная команда \"" << name << "\". Список команд: help.\n";
        }
        return;
    }
    Command command = parse(name, args);
    std::string extra;
    if (!command || (args >> extra)) {
        std::cout << "Неверные аргументы команды \"" << name << "\". Формат команды: help.\n";
        return;
    }
    if (command.execute(game)) {
        std::cout << "Выполнено.\n";
    } else {
        std::cout << "Действие \"" << name << "\" невозможно. Проверьте условия и состояние игры через help и stats.\n";
    }
}

bool UnitHandler::accepts(const std::string& name) const {
    return name == "create" || name == "move" || name == "attack" || name == "collect"
        || name == "heal" || name == "interact";
}

Command UnitHandler::parse(const std::string& name, std::istream& args) const {
    if (!word(args, "unit")) {
        return {};
    }
    int x, y;
    if (name == "create") {
        UnitKind kind;
        if (unitKind(args, kind) && word(args, "at") && position(args, x, y)) {
            return Command([kind, x, y](GameFacade& game) { return game.createUnit(kind, x, y); });
        }
        return {};
    }
    int id;
    if (!positiveNumber(args, id)) {
        return {};
    }
    if (name == "move") {
        if (word(args, "to") && position(args, x, y)) {
            return Command([id, x, y](GameFacade& game) { return game.moveUnit(id, x, y); });
        }
        return {};
    }
    if (name == "collect") {
        if (word(args, "from") && position(args, x, y)) {
            return Command([id, x, y](GameFacade& game) { return game.collectResource(id, x, y); });
        }
        return {};
    }
    if (name == "interact") {
        return Command([id](GameFacade& game) { return game.interact(id); });
    }
    int target;
    if (!word(args, "target") || !positiveNumber(args, target)) {
        return {};
    }
    if (name == "attack") {
        return Command([id, target](GameFacade& game) { return game.attack(id, target); });
    }
    return Command([id, target](GameFacade& game) { return game.heal(id, target); });
}

bool BaseHandler::accepts(const std::string& name) const {
    return name == "build" || name == "research" || name == "distribute" || name == "produce";
}

Command BaseHandler::parse(const std::string& name, std::istream& args) const {
    if (name == "build") {
        int x, y;
        if (word(args, "barracks") && word(args, "at") && position(args, x, y)) {
            return Command([x, y](GameFacade& game) { return game.buildBuilding(x, y); });
        }
        return {};
    }
    if (name == "research") {
        if (word(args, "attack_upgrade")) {
            return Command([](GameFacade& game) { return game.research(); });
        }
        return {};
    }
    int id;
    if (name == "distribute") {
        int amount;
        if (positiveNumber(args, amount) && word(args, "to") && positiveNumber(args, id)) {
            return Command([id, amount](GameFacade& game) { return game.distributeResources(id, amount); });
        }
        return {};
    }
    UnitKind kind;
    if (unitKind(args, kind) && word(args, "in") && positiveNumber(args, id)) {
        return Command([kind, id](GameFacade& game) { return game.produceUnit(kind, id); });
    }
    return {};
}

bool InfoHandler::accepts(const std::string& name) const {
    return name == "stats" || name == "help";
}

Command InfoHandler::parse(const std::string& name, std::istream& args) const {
    if (name == "help") {
        return Command([](GameFacade& game) {
            game.printHelp();
            return true;
        });
    }
    std::string section;
    if (!(args >> section)) {
        section = "all";
    }
    if (section != "all" && section != "units" && section != "buildings" && section != "resources"
        && section != "research" && section != "map") {
        return {};
    }
    return Command([section](GameFacade& game) {
        game.getStats(section);
        return true;
    });
}
