

#ifndef BUAA_COMPILER_SYNTAX_EXCEPTIONS_H
#define BUAA_COMPILER_SYNTAX_EXCEPTIONS_H

#include <bits/exception.h>
#include <string>
#include "syntax_nodes.h"
#include <utility>

#define self (*this)

class ComponentNotFoundException : std::exception {
private:
    std::string father;
    std::string component;
public:
    ComponentNotFoundException() : exception(), father(""), component(""){}

    ComponentNotFoundException(std::string component, std::string father)
            : exception(), father(std::move(father)), component(std::move(component)) {}

    const char *what() const noexcept override {
        return (self.component + " in " + self.father + " not found.").c_str();
    }
};

class LogException : std::exception {
private:
    std::string log;
public:
    LogException() : exception(), log(""){}

    explicit LogException(std::string log)
            : exception(), log(std::move(log)) {}

    const char *what() const noexcept override {
        return self.log.c_str();
    }
};

#endif //BUAA_COMPILER_SYNTAX_EXCEPTIONS_H
