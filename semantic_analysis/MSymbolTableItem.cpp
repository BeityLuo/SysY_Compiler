#include "MSymbolTableItem.h"
#include "MSymbolTable.h"

MFunctionSymbolTableItem::MFunctionSymbolTableItem(
        std::string symbol, int paramNum, std::vector<MParamType *> *paramTypes,
        MReturnType *returnType, MSymbolTable *funcSymbolTable, unsigned int address) :
        MSymbolTableItem(symbol, false, address), paramNum(paramNum),
        returnType(returnType), funcSymbolTable(funcSymbolTable) {

    self.funcSymbolTable->setFatherTableItem(this);
    for (auto type : *paramTypes) {
        self.paramTypes.push_back(type);
    }
}
