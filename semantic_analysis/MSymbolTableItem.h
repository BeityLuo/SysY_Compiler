#include <utility>

#ifndef BUAA_COMPILER_MSYMBOLTABLEITEM_H
#define BUAA_COMPILER_MSYMBOLTABLEITEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include "../syntax_analysis/syntax_nodes.h"
#include "MSymbolTable.h"

class MBaseType {
protected:
    const bool isConst;
    const MType* type;
    std::vector<int> *consts;
public:
    MBaseType(bool isConst, const MType* type, const std::vector<int> *consts)
    : isConst(isConst), type(type) {
        if (consts != nullptr) {
            self.consts = new std::vector<int>();
            for (auto c : *consts) {
                self.consts->push_back(c);
            }
        } else {
            self.consts = nullptr;
        }

    }
};
class MVariableType : public MBaseType{
private:

public:
    MVariableType(bool isConst, const MBType* bType, const std::vector<int> *consts) :
            MBaseType(isConst, bType, consts) {}
};

class MParamType : public MBaseType{
private:

public:
    MParamType(const MBType* bType, const std::vector<int> *consts) :
            MBaseType(false, bType, consts) {}
};

class MReturnType : public MBaseType{
private:
public:
    MReturnType(const MFuncType* funcType = nullptr, const std::vector<int> *consts = nullptr) :
            MBaseType(false, funcType, consts) {}
};

class MSymbolTableItem {
public:
    std::string symbol;
    unsigned int address;
    bool isVariable;

    explicit MSymbolTableItem(const std::string symbol, bool isVariable, unsigned int address = 0)
            : symbol(symbol), isVariable(isVariable), address(address) {}

    std::string getSymbol() {
        return symbol;
    }
};

class MVariableSymbolTableItem : public MSymbolTableItem {
public:
    MVariableType *variableType;
    int size;

    MVariableSymbolTableItem(const std::string symbol, MVariableType *variableType,
                             unsigned int address = 0, int size = 0)
            : MSymbolTableItem(symbol, true, address), variableType(variableType), size(size) {}
};

class MFunctionSymbolTableItem : public MSymbolTableItem {
public:
    int paramNum;
    std::vector<MParamType*> paramTypes;
    MSymbolTable *funcSymbolTable;
    MReturnType *returnType;

    MFunctionSymbolTableItem(std::string symbol, int paramNum,
                                     std::vector<MParamType*> *paramTypes,
                                     MReturnType *returnType, MSymbolTable *funcSymbolTable, unsigned int address = 0) :
            MSymbolTableItem(symbol, false, address), paramNum(paramNum),
            returnType(returnType), funcSymbolTable(funcSymbolTable) {
        for (auto type : *paramTypes) {
            self.paramTypes.push_back(type);
        }
    }
};

#endif //BUAA_COMPILER_MSYMBOLTABLEITEM_H
