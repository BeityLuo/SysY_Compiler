#ifndef BUAA_COMPILER_MIRSYMBOLTABLEITEM_H
#define BUAA_COMPILER_MIRSYMBOLTABLEITEM_H
enum MIRSymbolTableItemTypeID {
    VAR_IR_SYMBOL_TABLE_ITEM, FUNC_IR_SYMBOL_TABLE_ITEM
};
class MIRSymbolTableItem {
public:
    std::string name;
    MIRSymbolTableItem(std::string name) : name(name) {};
    virtual int getTypeID() = 0;
};

class MVarIRSymbolTableItem : public MIRSymbolTableItem {
public:
    MBaseType* varType;
    int address;
    MVarIRSymbolTableItem(std::string name, MBaseType* type, int address)
    : MIRSymbolTableItem(name), varType(type), address(address) {}

    int getTypeID() override {
        return MIRSymbolTableItemTypeID::VAR_IR_SYMBOL_TABLE_ITEM;
    }
};

class MFuncIRSymbolTableItem : public MIRSymbolTableItem {
public:
    MFuncIRSymbolTableItem(std::string name) : MIRSymbolTableItem(name) {}

    int getTypeID() override {
        return MIRSymbolTableItemTypeID::FUNC_IR_SYMBOL_TABLE_ITEM;
    }
};

#endif //BUAA_COMPILER_MIRSYMBOLTABLEITEM_H
