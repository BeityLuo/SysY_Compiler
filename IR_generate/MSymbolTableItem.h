#include <utility>

#ifndef BUAA_COMPILER_MSYMBOLTABLEITEM_H
#define BUAA_COMPILER_MSYMBOLTABLEITEM_H

#include <string>
#include <vector>
#include <unordered_map>
#include "../ast_generate/syntax_nodes.h"

class MSymbolTable;

class MBaseType {
public:
    const bool isConst;
    const MType *type;
    std::vector<int> *dims;
    bool isArray;
    bool isParam;

public:
    MBaseType(bool isConst, const MType *type, const std::vector<int> *dims,
              bool isParam = false, bool isArray = false)
            : isConst(isConst), type(type),
            isParam(isParam), isArray(isParam ? isArray : dims != nullptr && !dims->empty()) {

        if (dims != nullptr) {
            self.dims = new std::vector<int>();
            for (auto c: *dims) {
                self.dims->push_back(c);
            }
        } else {
            self.dims = nullptr;
        }
    }


    std::string toString() {
        std::string ans = "";
        if (self.isConst) ans += "const ";
        ans += token2cor_string(self.type->type);
        if (self.isParam && self.isArray)
            ans += "[]";
        if (self.dims != nullptr)
            for (auto d : *self.dims) {
                ans += "[" + std::to_string(d) + "]";
            }
        return ans;
    }

    bool equals(MBaseType* type) {
        // 这段代码目前来讲用不上
//        if (self.type->type == Token::VOIDTK)
//            // 判断void
//            return type->type->type == Token::VOIDTK;
//        if (!self.isArray)
//            // 判断普通变量
//            return !type->isArray;
//        // 判断数组
//        if (self.isParam) {
//            if (type->isParam) {
//                if (self.dims->size() != type->dims->size())
//                    throw "MBaseType::equals: type's dim not match";
//                for (int i = 0; i < self.dims->size(); i++)
//                    if ((*(self.dims))[i] != (*(type->dims))[i])
//                        return false;
//            } else {
//                if (self.dims->size() != type->dims->size() - 1)
//                    throw "MBaseType::equals: type's dim not match";
//                for (int i = 0; i < self.dims->size(); i++)
//                    if ((*(self.dims))[i] != (*(type->dims))[i + 1])
//                        return false;
//            }
//        } else {
//            if (type->isParam) {
//                if (self.dims->size() != type->dims->size() - 1)
//                    throw "MBaseType::equals: type's dim not match";
//                for (int i = 0; i < self.dims->size(); i++)
//                    if ((*(self.dims))[i + 1] != (*(type->dims))[i])
//                        return false;
//            } else {
//                if (self.dims->size() != type->dims->size())
//                    throw "MBaseType::equals: type's dim not match";
//                for (int i = 0; i < self.dims->size(); i++)
//                    if ((*(self.dims))[i] != (*(type->dims))[i])
//                        return false;
//            }
//        }
        return true;
    }
};

class MVarType : public MBaseType {
private:

public:
    MVarType(bool isConst, const MType *type, const std::vector<int> *dims) :
            MBaseType(isConst, type, dims) {}
};

class MParamType : public MBaseType {
private:

public:
    MParamType(const MType *type, const std::vector<int> *dims, bool isArray) :
            MBaseType(false, type, dims, true, isArray) {}
};

class MReturnType : public MBaseType {
private:
public:
    MReturnType(const MType *funcType = nullptr, const std::vector<int> *dims = nullptr) :
            MBaseType(false, funcType, dims) {}
};

enum MSymbolTableItemTypeID{
    VAR_SYMBOLTABLE_ITEM, FUNC_SYMBOLTABLE_ITEM
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
    virtual int getTypeID() = 0;
};

class MVariableSymbolTableItem : public MSymbolTableItem {
public:
    MBaseType *variableType;
    int size;
    std::vector<MExp*> *initExpArray;
    std::vector<int> *initArray; // 只有const的变量这一项才有用


    MVariableSymbolTableItem(const std::string symbol, MBaseType *variableType,
                             std::vector<MExp*>* initExpArray,
                             std::vector<int> *initArray = nullptr,
                             unsigned int address = 0, int size = 0)
            : MSymbolTableItem(symbol, true, address),
              variableType(variableType), size(size), initArray(initArray),
              initExpArray(initExpArray){}

    int getTypeID() override {
        return MSymbolTableItemTypeID::VAR_SYMBOLTABLE_ITEM;
    }
};

class MFunctionSymbolTableItem : public MSymbolTableItem {
public:
    int paramNum;
    std::vector<MParamType *> *paramTypes;
    std::vector<std::string> *paramNames;
    std::vector<int>* paramLineNum;
    MSymbolTable *funcSymbolTable;
    MBaseType *returnType;


    MFunctionSymbolTableItem(MBaseType *returnType, std::string &symbol,
                             int paramNum,
                             std::vector<MParamType *> *paramTypes,
                             std::vector<std::string> *paramNames,
                             std::vector<int>* paramLineNum,
                             MSymbolTable *funcSymbolTable = nullptr);

    void setTable(MSymbolTable *table) {
        self.funcSymbolTable = table;
    }

    int getTypeID() override {
        return MSymbolTableItemTypeID::FUNC_SYMBOLTABLE_ITEM;
    }

};

#endif //BUAA_COMPILER_MSYMBOLTABLEITEM_H
