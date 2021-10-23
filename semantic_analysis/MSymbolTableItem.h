#ifndef BUAA_COMPILER_MSYMBOLTABLEITEM_H
#define BUAA_COMPILER_MSYMBOLTABLEITEM_H

#include <string>
#include <vector>
#include <bits/unordered_map.h>

class MSymbolTableItem {
public:
    std::string symbol;
    unsigned int address;
};

class MVariableSymbolTableItem : public MSymbolTableItem {
public:
    MVariableType* variableType;
    bool isConst;
    int size;
};

class MFunctionVariableSymbolTableItem : public MSymbolTableItem {
public:
    int paraNum;
    std::unordered_map<std::string, MVariableSymbolTableItem*> paras;
    MReturnType* returnType;
};
#endif //BUAA_COMPILER_MSYMBOLTABLEITEM_H
