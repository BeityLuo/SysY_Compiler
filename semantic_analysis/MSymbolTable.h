#ifndef BUAA_COMPILER_SYMBOLTABLE_H
#define BUAA_COMPILER_SYMBOLTABLE_H

#include <unordered_map>
#include <vector>
#include "MSymbol.h"

#define self (*this)

class MSymbolTableItem;
class MSymbolTable {
private:
    std::unordered_map<std::string, MSymbolTableItem *> table;
    MSymbolTable *childScopeTable; //栈式结构，故只保存一个
    MSymbolTable *fatherScopeTable;
public:
    MSymbolTable() : childScopeTable(nullptr), fatherScopeTable(nullptr) {}
    MSymbolTable(MSymbolTable* fatherScopeTable) : childScopeTable(nullptr), fatherScopeTable(fatherScopeTable) {}

    void addSymbolItem(MSymbolTableItem *symbolTableItem);

    bool contains(MSymbol *symbol) {
        return false;
    }

    void setChildScopeTable(MSymbolTable *symbolTable) {
        if (self.childScopeTable != nullptr) {
            delete(self.childScopeTable);
        }
        self.childScopeTable = symbolTable;
    }

    MSymbolTable* getFatherScopeTable() {
        return self.fatherScopeTable;
    }

    MSymbolTable* getChildScopTable() {
        return self.childScopeTable;
    }

    MSymbolTableItem* getTableItem(std::string name) {
        return self.table[name];
    }

};


#endif //BUAA_COMPILER_SYMBOLTABLE_H
