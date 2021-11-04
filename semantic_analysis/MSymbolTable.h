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

public:
    MSymbolTable(bool isCyclic)
            : childScopeTable(nullptr), fatherScopeTable(nullptr),
              fatherTableItem(nullptr), isCyclic(isCyclic) {}

    MSymbolTable(MSymbolTable *fatherScopeTable, bool isCyclic)
            : childScopeTable(nullptr), fatherScopeTable(fatherScopeTable),
              fatherTableItem(nullptr), isCyclic(isCyclic) {}

    ~MSymbolTable() {
        if (self.fatherScopeTable != nullptr)
            self.fatherScopeTable->setChildScopeTable(nullptr);

        for (auto pair : self.table) {
            delete(pair.second);
        }
    }

    void addSymbolItem(MSymbolTableItem *symbolTableItem);

    bool contains(std::string &name) {
        return self.table[name] != nullptr;
    }

    void setChildScopeTable(MSymbolTable *symbolTable) {
        if (self.childScopeTable != nullptr) {
            delete (self.childScopeTable);
        }
        self.childScopeTable = symbolTable;
    }

    MSymbolTable *getFatherScopeTable() {
        return self.fatherScopeTable;
    }

    MSymbolTable *getChildScopTable() {
        return self.childScopeTable;
    }

    MSymbolTableItem *getTableItem(std::string name) {
        return self.table[name];
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

};


#endif //BUAA_COMPILER_SYMBOLTABLE_H
