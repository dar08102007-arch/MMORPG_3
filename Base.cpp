#include "Base.h"
#include <algorithm>
#include <iostream>
#include <ctime>

Base::Base(std::string name, int x, int y, int hp, int limit, int defense, int resources)
    : name_(name), x_(x), y_(y), health_(hp), maxUnits_(limit), defense_(defense), resources_(resources) {
    if (maxUnits_ > 0) {
        unitIds_.reserve(maxUnits_);
    }
}

bool Base::createUnit(GameField& field, const UnitFactory& factory, UnitKind kind, int x, int y,
                      int buildingId) {
    Building* building = findBuilding(buildingId);
    if (buildingId != 0 && !building) {
        return false;
    }
    int& budget = building ? building->resources : resources_;
    int cost = UnitTypeInfo::getInfo(kind).cost;

    if (health_ <= 0 || count() >= maxUnits_ || budget < cost || !field.inBounds(x,y) || field.getUnit(x,y)) {
        return false;
    }

    if (!field.spawn(factory, kind, x, y)) {
        return false;
    }

    Unit* unit = field.getUnit(x,y);
    unit->subscribe(this);
    unitIds_.push_back(unit->id());
    budget -= cost;
    if (attackUpgrade_) {
        unit->boostAttack(attackBonus);
    }

    unit->notify({EventType::Created, unit->id(), 0, unit->getType(), static_cast<int>(std::time(nullptr))});
    return true;
}

bool Base::removeUnit(GameField& field, int unitId) {
    for (Unit& unit : field) if (unit.id() == unitId) {
        unit.unsubscribe(this);
        unitIds_.erase(std::remove(unitIds_.begin(), unitIds_.end(), unitId), unitIds_.end());
        return field.removeUnit(unit.getX(), unit.getY());
    }
    return false;
}
void Base::onUnitEvent(const UnitEvent& e) {
    auto owned = std::find(unitIds_.begin(), unitIds_.end(), e.unitId);
    if (owned == unitIds_.end()) {
        return;
    }
    std::cout << "[База " << name_ << "] " << e.unitName << " #" << e.unitId << ": ";
    if (e.type == EventType::Created) {
        std::cout << "создан";
    }
    if (e.type == EventType::Damaged) {
        std::cout << "получил урон " << e.damage;
    }
    if (e.type == EventType::Destroyed) {
        std::cout << "уничтожен";
        unitIds_.erase(owned);
    }
    std::cout << "\n";
}
void Base::takeDamage(int damage) {
    if (damage > defense_) {
        health_ = std::max(0, health_ - (damage - defense_));
    }
    if (health_ == 0) {
        std::cout << "База " << name_ << " разрушена!\n";
    }
}
void Base::releaseObservers(GameField& field) { for (Unit& unit : field) unit.unsubscribe(this); }

bool Base::buildBarracks(int x, int y) {
    if (health_ <= 0 || resources_ < barracksCost || getBuildingAt(x, y)) {
        return false;
    }
    int id = static_cast<int>(buildings_.size()) + 1;
    buildings_.push_back({id, x, y});
    resources_ -= barracksCost;
    return true;
}

bool Base::distributeResources(int buildingId, int amount) {
    Building* building = findBuilding(buildingId);
    if (health_ <= 0 || !building || amount <= 0 || amount > resources_) {
        return false;
    }
    resources_ -= amount;
    building->resources += amount;
    return true;
}

bool Base::researchAttack() {
    if (health_ <= 0 || attackUpgrade_ || resources_ < researchCost) {
        return false;
    }
    resources_ -= researchCost;
    attackUpgrade_ = true;
    return true;
}

const Building* Base::getBuilding(int id) const {
    if (id <= 0 || id > static_cast<int>(buildings_.size())) {
        return nullptr;
    }
    return &buildings_[id - 1];
}

Building* Base::findBuilding(int id) {
    if (!getBuilding(id)) {
        return nullptr;
    }
    return &buildings_[id - 1];
}

const Building* Base::getBuildingAt(int x, int y) const {
    for (const Building& building : buildings_) {
        if (building.x == x && building.y == y) {
            return &building;
        }
    }
    return nullptr;
}
