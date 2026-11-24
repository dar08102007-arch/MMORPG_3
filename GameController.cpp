#include "GameController.h"
#include "InfantryFactory.h"
#include "ArcherFactory.h"
#include "CavalryFactory.h"
#include "MageFactory.h"
#include "FlyingFactory.h"
#include <iostream>
#include <cmath>
#include <utility>

GameController::GameController()
    : field_(12,8,20), terrain_(12,8),
      base_("Минас-Тирит",0,0,500,12,5,500) {
    terrain_.setTerrain(3,2,std::make_unique<Forest>());
    terrain_.setTerrain(4,2,std::make_unique<Forest>());
    terrain_.setTerrain(5,2,std::make_unique<Swamp>());
    terrain_.setTerrain(7,2,std::make_unique<Mountains>());
    terrain_.setTerrain(8,2,std::make_unique<Mountains>());
    terrain_.setTerrain(3,4,std::make_unique<Swamp>());
    terrain_.setNeutralObject(4,3,std::make_shared<TreasureChest>());
    terrain_.setNeutralObject(5,4,std::make_shared<Trap>());
    terrain_.setNeutralObject(2,3,std::make_shared<Altar>());
    terrain_.setNeutralObject(8,5,std::make_shared<EnergySource>());
}
GameController::~GameController() { base_.releaseObservers(field_); }
bool GameController::create(UnitKind kind, int x, int y) {
    return createAt(kind, x, y, 0);
}

bool GameController::isFreeCell(int x, int y) const {
    return field_.inBounds(x, y) && !field_.getUnit(x, y) && !base_.getBuildingAt(x, y)
        && (x != base_.x() || y != base_.y());
}

bool GameController::createAt(UnitKind kind, int x, int y, int buildingId) {
    InfantryFactory infantry;
    ArcherFactory archer;
    CavalryFactory cavalry;
    MageFactory mage;
    FlyingFactory flying;
    const UnitFactory* factory = nullptr;

    switch(kind) {
    case UnitKind::GondorSoldier: case UnitKind::UrukHai: factory=&infantry; break;
    case UnitKind::ElfArcher: case UnitKind::OrcArcher: factory=&archer; break;
    case UnitKind::RohanRider: case UnitKind::WargRider: factory=&cavalry; break;
    case UnitKind::Gandalf: case UnitKind::Saruman: factory=&mage; break;
    case UnitKind::GreatEagle: case UnitKind::FellBeast: factory=&flying; break;
    }

    if (!factory || !isFreeCell(x, y)) {
        return false;
    }

    TerrainProxy proxy(&terrain_.getTerrain(x,y));
    if (!proxy.isPassable(kind)) {
        return false;
    }

    if (!base_.createUnit(field_, *factory, kind, x, y, buildingId)) {
        return false;
    }
    field_.getUnit(x, y)->setTerrainArmor(static_cast<int>(proxy.getDefenseBonus(kind)));
    return true;
}
bool GameController::move(int x,int y,int nx,int ny) {
    Unit* u = field_.getUnit(x, y);
    if (!u || !u->isAlive() || !isFreeCell(nx, ny)) {
        return false;
    }

    if (std::abs(nx-x)+std::abs(ny-y) != 1) {
        return false;
    }

    TerrainProxy proxy(&terrain_.getTerrain(nx,ny));
    if (!proxy.isPassable(u->kind())) {
        return false;
    }

    double speed = proxy.getSpeedModifier(u->kind());
    double defense = proxy.getDefenseBonus(u->kind());
    if (!field_.moveUnit(x,y,nx,ny)) {
        return false;
    }

    u->setTerrainArmor(static_cast<int>(defense));
    std::cout << u->getType() << " переместился в (" << nx << "," << ny << "), скорость x" << speed << "\n";
    return true;
}
bool GameController::interact(int x,int y) {
    Unit* u = field_.getUnit(x, y);
    NeutralObject* obj = terrain_.getNeutralObject(x, y);

    if (!u || !obj || !u->isAlive()) {
        return false;
    }

    std::cout << u->getType() << " взаимодействует: " << obj->name() << "\n";
    if (auto chest=dynamic_cast<TreasureChest*>(obj)) {
        *u + *chest;
        base_.addResources(30);
        terrain_.setNeutralObject(x, y, nullptr);

    } else if (auto trap=dynamic_cast<Trap*>(obj)) *u - *trap;
    else if (auto altar=dynamic_cast<Altar*>(obj)) { altar->chooseStrategy(u->kind()); *u * *altar; }
    else if (auto source=dynamic_cast<EnergySource*>(obj)) {
        *u / *source;
    }
    if (!u->isAlive()) {
        field_.removeUnit(x, y);
    }
    return true;
}
bool GameController::fight(int x,int y,int tx,int ty) {
    Unit* a = field_.getUnit(x, y);
    Unit* b = field_.getUnit(tx, ty);
    if (!a || !b || !a->isAlive() || !b->isAlive() || a==b) {
        return false;
    }
    if (std::abs(x-tx)+std::abs(y-ty)>1) {
        return false;
    }
    int damage=a->attack(*b);
    std::cout << a->getType() << " атакует " << b->getType() << ": урон " << damage << "\n";
    if (!b->isAlive()) {
        field_.removeUnit(tx,ty);
    }
    return true;
}
Unit* GameController::findUnit(int id) {
    for (Unit& unit : field_) {
        if (unit.id() == id && unit.isAlive()) {
            return &unit;
        }
    }
    return nullptr;
}

bool GameController::moveUnit(int id, int x, int y) {
    Unit* unit = findUnit(id);
    return unit && move(unit->getX(), unit->getY(), x, y);
}

bool GameController::attack(int attackerId, int targetId) {
    Unit* attacker = findUnit(attackerId);
    Unit* target = findUnit(targetId);
    return attacker && target
        && fight(attacker->getX(), attacker->getY(), target->getX(), target->getY());
}

bool GameController::collectResource(int id, int x, int y) {
    Unit* unit = findUnit(id);
    if (!unit || unit->getX() != x || unit->getY() != y
        || !dynamic_cast<TreasureChest*>(terrain_.getNeutralObject(x, y))) {
        return false;
    }
    return interact(x, y);
}

bool GameController::heal(int healerId, int targetId) {
    Unit* healer = findUnit(healerId);
    Unit* target = findUnit(targetId);
    if (!healer || !target || (healer->kind() != UnitKind::Gandalf && healer->kind() != UnitKind::Saruman)) {
        return false;
    }
    if (std::abs(healer->getX() - target->getX()) + std::abs(healer->getY() - target->getY()) > 1) {
        return false;
    }
    int before = target->getHealth().getValue();
    HealthRestoreStrategy().apply(*target);
    std::cout << "Юнит #" << targetId << " восстановил " << target->getHealth().getValue() - before << " HP\n";
    return true;
}

bool GameController::interactUnit(int id) {
    Unit* unit = findUnit(id);
    return unit && interact(unit->getX(), unit->getY());
}

bool GameController::buildBuilding(int x, int y) {
    if (!isFreeCell(x, y) || terrain_.getNeutralObject(x, y) || terrain_.getTerrain(x, y).symbol() != '.') {
        return false;
    }
    if (!base_.buildBarracks(x, y)) {
        return false;
    }
    std::cout << "Казарма #" << base_.buildings().back().id << " построена в (" << x << ',' << y << ")\n";
    return true;
}

bool GameController::research() {
    if (!base_.researchAttack()) {
        return false;
    }
    for (Unit& unit : field_) {
        if (unit.isAlive()) {
            unit.boostAttack(Base::attackBonus);
        }
    }
    std::cout << "Улучшение attack_upgrade: +" << Base::attackBonus << " к атаке\n";
    return true;
}

bool GameController::distributeResources(int buildingId, int amount) {
    return base_.distributeResources(buildingId, amount);
}

bool GameController::produceUnit(UnitKind kind, int buildingId) {
    const Building* building = base_.getBuilding(buildingId);
    if (!building) {
        return false;
    }
    const int offsets[][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    for (const auto& offset : offsets) {
        if (createAt(kind, building->x + offset[0], building->y + offset[1], buildingId)) {
            return true;
        }
    }
    return false;
}

void GameController::getStats(const std::string& section) {
    if (section == "map") {
        print();
        return;
    }
    const bool all = section == "all";
    if (all || section == "units") {
        std::cout << "Юниты: " << base_.count() << '/' << base_.maxUnits() << '\n';
        for (const Unit& unit : field_) {
            TerrainProxy proxy(&terrain_.getTerrain(unit.getX(), unit.getY()));
            double speed = proxy.getSpeedModifier(unit.kind());
            std::cout << "  #" << unit.id() << ' ' << unit.getType() << " (" << unit.getX() << ',' << unit.getY()
                      << ") HP=" << unit.getHealth().getValue() << '/' << unit.getInfo().maxHealth
                      << " атака=" << unit.getAttack().getValue() << " броня=" << unit.getEffectiveArmor().getValue()
                      << " скорость=" << unit.getInfo().baseSpeed * speed << " (x" << speed << ")\n";
        }
    }
    if (all || section == "buildings") {
        std::cout << "База " << base_.name() << " (" << base_.x() << ',' << base_.y()
                  << ") HP=" << base_.health() << " защита=" << base_.defense() << '\n';
        std::cout << "Казармы: " << base_.buildings().size() << '\n';
        for (const Building& building : base_.buildings()) {
            std::cout << "  #" << building.id << " (" << building.x << ',' << building.y
                      << ") выделено=" << building.resources << '\n';
        }
    }
    if (all || section == "resources") {
        int allocated = 0;
        for (const Building& building : base_.buildings()) {
            allocated += building.resources;
        }
        std::cout << "Ресурсы: свободно " << base_.resources() << ", выделено " << allocated
                  << ", всего " << base_.resources() + allocated << '\n';
        for (int y = 0; y < terrain_.height(); ++y) {
            for (int x = 0; x < terrain_.width(); ++x) {
                if (dynamic_cast<TreasureChest*>(terrain_.getNeutralObject(x, y))) {
                    std::cout << "  Сундук (" << x << ',' << y << ")\n";
                }
            }
        }
    }
    if (all || section == "research") {
        std::cout << "Исследование attack_upgrade: "
                  << (base_.hasAttackUpgrade() ? "завершено" : "доступно") << '\n';
    }
}

void GameController::print() const {
    std::cout << "\n   "; for(int x=0;x<terrain_.width();++x) std::cout << x%10 << ' '; std::cout << '\n';
    for(int y=0;y<terrain_.height();++y) {
        std::cout << y << "  ";
        for(int x=0;x<terrain_.width();++x) {
            char ch=terrain_.getTerrain(x,y).symbol();
            if (auto obj=terrain_.getNeutralObject(x,y)) {
                ch=obj->symbol();
            }
            if (auto unit=field_.getUnit(x,y)) {
                ch=unit->symbol();
            }
            if (x==base_.x() && y==base_.y()) {
                ch='B';
            }
            if (base_.getBuildingAt(x, y)) {
                ch='K';
            }
            std::cout << ch << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "Легенда: B база, K казарма, . равнина, * лес, ~ болото, ^ горы, $ сундук, ! ловушка, A алтарь, + источник\n";
    std::cout << "База " << base_.name() << ": " << base_.count() << " юнитов, " << base_.resources() << " ресурсов\n";
}
void GameController::runDemos() {
    std::cout << "\n СЦЕНАРИЙ 1: БАЗА И СОЗДАНИЕ ЮНИТОВ ЧЕРЕЗ ФАБРИКИ \n";
    create(UnitKind::GondorSoldier,1,1);
    create(UnitKind::UrukHai,10,6);
    create(UnitKind::ElfArcher,2,2);
    create(UnitKind::OrcArcher,9,6);
    create(UnitKind::RohanRider,6,2);
    create(UnitKind::WargRider,9,5);
    create(UnitKind::Gandalf,1,3);
    create(UnitKind::Saruman,10,5);
    create(UnitKind::GreatEagle,7,1);
    create(UnitKind::FellBeast,10,4);
    print();

    std::cout << "\n СЦЕНАРИЙ 2: ПЕРЕМЕЩЕНИЕ И ПРОКСИ ЛАНДШАФТА \n";
    std::cout << "Всадник в горах: " << (move(6,2,7,2)?"прошёл":"проход закрыт") << '\n';
    std::cout << "Орёл в горах: " << (move(7,1,7,2)?"прошёл":"проход закрыт") << '\n';
    move(2,2,3,2);
    print();

    std::cout << "\n СЦЕНАРИЙ 3: ОБЪЕКТЫ, СТРАТЕГИЯ И ОПЕРАТОРЫ \n";
    move(1,3,2,3); interact(2,3); // Алтарь: operator*
    move(3,2,3,3); move(3,3,4,3); interact(4,3); // Сундук: operator+
    create(UnitKind::OrcArcher,5,3); move(5,3,5,4); interact(5,4); // Ловушка: operator-
    create(UnitKind::Gandalf,8,4); move(8,4,8,5); interact(8,5); // Источник: operator/
    print();

    std::cout << "\n СЦЕНАРИЙ 4: НАБЛЮДАТЕЛЬ, ГИБЕЛЬ И КОПИРОВАНИЕ \n";
    Unit* soldier=field_.getUnit(1,1);
    if (soldier) {
        soldier->takeDamage(500);
        field_.removeUnit(1, 1);
    }
    std::cout << "Юнитов на базе после гибели: " << base_.count() << '\n';
    GameField copy=field_;
    GameField moved=std::move(copy);
    std::cout << "Юнитов в перемещённой копии: " << moved.size() << '\n';
    std::cout << "Обход итератором:\n";
    for (Unit& u : moved) {
        std::cout << "  " << u.getType() << " #" << u.id() << " (" << u.getX() << ',' << u.getY() << ")\n";
    }
    print();
}
