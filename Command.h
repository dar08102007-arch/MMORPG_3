#pragma once
#include <functional>
#include <utility>

class GameFacade;

class Command {
public:
    Command() = default;
    explicit Command(std::function<bool(GameFacade&)> action) : action_(std::move(action)) {}

    explicit operator bool() const { return static_cast<bool>(action_); }
    bool execute(GameFacade& game) const { return action_(game); }

private:
    std::function<bool(GameFacade&)> action_;
};
