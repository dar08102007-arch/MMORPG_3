#pragma once
#include "GameController.h"
#include "Handler.h"

class GameFacade {
public:
    GameFacade();
    void processInput(const std::string& input);
    bool createUnit(UnitKind kind, int x, int y) { return mediator_.create(kind, x, y); }
    bool moveUnit(int id, int x, int y) { return mediator_.moveUnit(id, x, y); }
    bool attack(int attacker, int target) { return mediator_.attack(attacker, target); }
    bool collectResource(int id, int x, int y) { return mediator_.collectResource(id, x, y); }
    bool heal(int healer, int target) { return mediator_.heal(healer, target); }
    bool interact(int id) { return mediator_.interactUnit(id); }
    bool buildBuilding(int x, int y) { return mediator_.buildBuilding(x, y); }
    bool research() { return mediator_.research(); }
    bool distributeResources(int id, int amount) { return mediator_.distributeResources(id, amount); }
    bool produceUnit(UnitKind kind, int id) { return mediator_.produceUnit(kind, id); }
    void getStats(const std::string& section) { mediator_.getStats(section); }
    void printHelp() const;

private:
    GameController mediator_;
    UnitHandler first_;
};
