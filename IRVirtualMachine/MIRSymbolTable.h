#ifndef BUAA_COMPILER_MIRSYMBOLTABLE_H
#define BUAA_COMPILER_MIRSYMBOLTABLE_H
#include "MIRSymbolTableItem.h"
#include <vector>
#define self (*this)

class MIRSymbolTable {
private:
    std::unordered_map<std::string, MIRSymbolTableItem*>* items;

public:
    MIRSymbolTable* father;
    MIRSymbolTable* child;
    int id;

    MIRSymbolTable(MIRSymbolTable* father)
    : father(father), child(nullptr),
    items(new std::unordered_map<std::string, MIRSymbolTableItem*>()) {
        if (father != nullptr)
            self.id = father->id + 1;
        else
            self.id = 0;
    }

    MIRSymbolTable *getFather() const {
        return father;
    }

    void setFather(MIRSymbolTable *father) {
        self.father = father;
    }

    MIRSymbolTable* getChild() const {
        return child;
    }

    void setChild(MIRSymbolTable* child) {
        self.child = child;
    }

    bool contains(std::string name) {
        return self.items->find(name) != self.items->end();
    }

    void addItem(MIRSymbolTableItem* item) {
        (*(self.items))[item->name] = item;
    }

    MIRSymbolTableItem* getItem(std::string name) {
        return (*(self.items))[name];
    }
};

#endif //BUAA_COMPILER_MIRSYMBOLTABLE_H
