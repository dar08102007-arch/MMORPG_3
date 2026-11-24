#pragma once
#include <string>
#include <vector>
#include "GameField.h"
#include "UnitEvent.h"

struct Building {
    int id;
    int x;
    int y;
    int resources = 0;
};

class Base : public IObserver {
public:
    static constexpr int barracksCost = 60;
    static constexpr int researchCost = 50;
    static constexpr int attackBonus = 5;

    Base(std::string name, int x, int y, int health, int maxUnits, int defense, int resources);
    Base(const Base&) = delete;
    Base& operator=(const Base&) = delete;
    bool createUnit(GameField& field, const UnitFactory& factory, UnitKind kind, int x, int y,
                    int buildingId = 0);
    bool removeUnit(GameField& field, int unitId);
    void onUnitEvent(const UnitEvent& event) override;
    void releaseObservers(GameField& field);
    int count() const { return static_cast<int>(unitIds_.size()); }
    int maxUnits() const { return maxUnits_; }
    int resources() const { return resources_; }
    int health() const { return health_; }
    int defense() const { return defense_; }
    int x() const { return x_; }
    int y() const { return y_; }
    const std::string& name() const { return name_; }
    void addResources(int amount) { if (amount > 0) resources_ += amount; }
    void takeDamage(int damage);
    bool buildBarracks(int x, int y);
    bool distributeResources(int buildingId, int amount);
    bool researchAttack();
    bool hasAttackUpgrade() const { return attackUpgrade_; }
    const Building* getBuilding(int id) const;
    const Building* getBuildingAt(int x, int y) const;
    const std::vector<Building>& buildings() const { return buildings_; }
private:
    Building* findBuilding(int id);

    std::string name_;
    int x_, y_, health_, maxUnits_, defense_, resources_;
    std::vector<int> unitIds_;
    std::vector<Building> buildings_;
    bool attackUpgrade_ = false;
};
