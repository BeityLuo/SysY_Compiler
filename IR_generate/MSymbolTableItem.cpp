#include "MSymbolTableItem.h"
#include "MSymbolTable.h"

MFunctionSymbolTableItem::MFunctionSymbolTableItem(
        MBaseType *returnType, std::string &symbol,
        int paramNum,
        std::vector<MParamType *> *paramTypes,
        std::vector<std::string> *paramNames,
        std::vector<int>* paramLineNum,
        MSymbolTable *funcSymbolTable) :
        MSymbolTableItem(symbol, false),
        paramNum(paramNum), paramTypes(new std::vector<MParamType*>()),
        funcSymbolTable(funcSymbolTable), returnType(returnType),
        paramNames(new std::vector<std::string>()),
        paramLineNum(new std::vector<int>()){
    if (self.funcSymbolTable != nullptr)
        self.funcSymbolTable->setFatherTableItem(this);

    pushVector(self.paramTypes, paramTypes);
    pushVector(self.paramNames, paramNames);
    pushVector(self.paramLineNum, paramLineNum);
}
