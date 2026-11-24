#pragma once
#include <istream>
#include <memory>
#include <string>
#include "Command.h"

class Handler {
public:
    virtual ~Handler() = default;
    void setNext(std::unique_ptr<Handler> next);
    void handle(const std::string& name, std::istream& args, GameFacade& game) const;

protected:
    virtual bool accepts(const std::string& name) const = 0;
    virtual Command parse(const std::string& name, std::istream& args) const = 0;

private:
    std::unique_ptr<Handler> next_;
};

class UnitHandler : public Handler {
    bool accepts(const std::string& name) const override;
    Command parse(const std::string& name, std::istream& args) const override;
};

class BaseHandler : public Handler {
    bool accepts(const std::string& name) const override;
    Command parse(const std::string& name, std::istream& args) const override;
};

class InfoHandler : public Handler {
    bool accepts(const std::string& name) const override;
    Command parse(const std::string& name, std::istream& args) const override;
};
