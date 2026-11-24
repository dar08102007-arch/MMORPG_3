#pragma once
#include "Base.h"
#include "Terrain.h"
#include "NeutralObject.h"
class GameController {
public:
    GameController();
    ~GameController();
    bool create(UnitKind kind, int x, int y);
    bool move(int x, int y, int nx, int ny);
    bool interact(int x, int y);
    bool fight(int x, int y, int tx, int ty);
    bool moveUnit(int id, int x, int y);
    bool attack(int attackerId, int targetId);
    bool collectResource(int id, int x, int y);
    bool heal(int healerId, int targetId);
    bool interactUnit(int id);
    bool buildBuilding(int x, int y);
    bool research();
    bool distributeResources(int buildingId, int amount);
    bool produceUnit(UnitKind kind, int buildingId);
    void getStats(const std::string& section);
    void print() const;
    void runDemos();
    const GameField& field() const { return field_; }
    const Base& base() const { return base_; }
private:
    Unit* findUnit(int id);
    bool isFreeCell(int x, int y) const;
    bool createAt(UnitKind kind, int x, int y, int buildingId);

    GameField field_;
    TerrainMap terrain_;
    Base base_;
};
