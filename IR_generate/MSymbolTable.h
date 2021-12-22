#ifndef BUAA_COMPILER_SYMBOLTABLE_H
#define BUAA_COMPILER_SYMBOLTABLE_H

#include <unordered_map>
#include <vector>
#include "MSymbolTableItem.h"

#define self (*this)


class MSymbolTable {
private:
    std::unordered_map<std::string, MSymbolTableItem *> table;
    MSymbolTable *childScopeTable; //栈式结构，故只保存一个
    MSymbolTable *fatherScopeTable;
    MFunctionSymbolTableItem *fatherTableItem;
    bool isCyclic;
    int scopeLevel;

public:
    MSymbolTable(bool isCyclic)
            : childScopeTable(nullptr), fatherScopeTable(nullptr),
              fatherTableItem(nullptr), isCyclic(isCyclic), scopeLevel(0) {}

    MSymbolTable()
            : childScopeTable(nullptr), fatherScopeTable(nullptr),
              fatherTableItem(nullptr), isCyclic(false), scopeLevel(0) {}

    MSymbolTable(MSymbolTable *fatherScopeTable)
            : childScopeTable(nullptr), fatherScopeTable(fatherScopeTable),
              fatherTableItem(nullptr), isCyclic(isCyclic) {
        if (fatherScopeTable != nullptr) {
            self.scopeLevel = fatherScopeTable->scopeLevel + 1;
        }
    }

    ~MSymbolTable() {
        if (self.fatherScopeTable != nullptr)
            self.fatherScopeTable->setChildScopeTable(nullptr);
        for (auto pair : self.table) {
            delete(pair.second);
        }
    }

    void addSymbolItem(MSymbolTableItem *symbolTableItem);

    bool contains(const std::string &name) {
        return self.table.find(name) != self.table.end();
    }

    void setChildScopeTable(MSymbolTable *symbolTable) {
        self.childScopeTable = symbolTable;
    }

    MSymbolTable *getFatherScopeTable() {
        return self.fatherScopeTable;
    }

    MSymbolTable *getChildScopTable() {
        return self.childScopeTable;
    }

    MSymbolTableItem *getTableItem(const std::string& name) {
        if (self.contains(name))
            return self.table[name];
        else
            return nullptr;
    }

    void setFatherTableItem(MFunctionSymbolTableItem *item) {
        self.fatherTableItem = item;
    }

    MFunctionSymbolTableItem *getFatherTableItem() {
        return self.fatherTableItem;
    }

    bool isCyclicBlock() {
        return self.isCyclic;
    }

    int getScopeLevel() {
        return self.scopeLevel;
    }

};


#endif //BUAA_COMPILER_SYMBOLTABLE_H
