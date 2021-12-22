#ifndef BUAA_COMPILER_IRSTATEMENTS_H
#define BUAA_COMPILER_IRSTATEMENTS_H

#include "IRVar.h"

enum class MIRStatementType {
    DECL, FUNC_DEF, CALL_FUNC, RETURN, PUSH, POP,
    BINARY, UNARY, LABEL, ENTER_BLOCK, EXIT_BLOCK,
    SAVE, GETINT, PUTCHAR, PUTVAR, EXIT, BNT, BR
};

class MIRStatement {
private:
    MIRStatement(){}
public:
    MIRStatementType type;
    virtual std::string toString() = 0;

protected:
    MIRStatement(MIRStatementType type) : type(type) {}
};

class MDeclIRStatement : public MIRStatement {
public:
    MBaseType *type;
    std::string name;
public:
    MDeclIRStatement(MBaseType *type, const std::string &name)
            : type(type), name(name), MIRStatement(MIRStatementType::DECL) {}

    std::string toString() override {
        return "decl " + type->toString() + " " + name;
    }
};

class MFuncDefIRStatement : public MIRStatement {
public:
    int paramNum;
    std::string funcName;
    std::vector<MParamType *> *paramTypes;
    std::vector<std::string> *paramNames;
    MSymbolTable *funcSymbolTable;
    MBaseType *returnType;


    MFuncDefIRStatement(MBaseType *returnType, std::string funcName,
                        int paramNum,
                        std::vector<MParamType *> *paramTypes,
                        std::vector<std::string> *paramNames) :
            paramNum(paramNum), paramTypes(paramTypes), funcName(funcName),
            funcSymbolTable(funcSymbolTable), returnType(returnType),
            paramNames(paramNames), MIRStatement(MIRStatementType::FUNC_DEF) {
        if (paramNum != paramNames->size() || paramNum != paramTypes->size()) {
            throw "MFuncDefIRStatement::MFuncDefIRStatement: 参数个数、参数类型、参数名字不匹配";
        }
    }

    std::string toString() override {
        auto ans = "def " + returnType->toString() + " " + funcName + " params:{ ";
        for (int i = 0; i < self.paramNum - 1; i++) {
            ans += (*(self.paramTypes))[i]->toString() +
                   " " + (*(self.paramNames))[i] + ", ";
        }
        if (self.paramNum != 0)
            ans += (*(self.paramTypes))[self.paramNum - 1]->toString() +
                   " " + (*(self.paramNames))[self.paramNum - 1] + " ";
        ans += "}";
        return ans;
    }
};


class MCallFuncIRStatement : public MIRStatement {
public:
    std::string funcName;

    MCallFuncIRStatement(std::string funcName) : funcName(funcName)
            , MIRStatement(MIRStatementType::CALL_FUNC){}

    std::string toString() override {
        return "call " + funcName;
    }
};

class MReturnIRStatement : public MIRStatement {
public:
    MReturnIRStatement() : MIRStatement(MIRStatementType::RETURN) {}
    std::string toString() override {
        return "return";
    }
};

class MPushIRStatement : public MIRStatement {
public:
    MIRVar* irVar;

    MPushIRStatement(MIRVar* irVar) :
            irVar(irVar), MIRStatement(MIRStatementType::PUSH) {}

    std::string toString() override {
        return "push " + irVar->toString();
    }
};

class MPopIRStatement : public MIRStatement {
public:
    MIRVar* irVar;

    MPopIRStatement(MIRVar* irVar) :
            irVar(irVar), MIRStatement(MIRStatementType::POP) {}

    std::string toString() override {
        return "pop " + irVar->toString();
    }
};
class MCalculateIRStatement : public MIRStatement {
public:
    MCalculateIRStatement(MIRStatementType type) : MIRStatement(type) {}
};

enum class MBinaryIRType{
    ADD, SUB, MULT, DIV, MOD, OR, AND,
    GRE, GEQ, LSS, LEQ, EQL, NEQ
};


class MBinaryIRStatement : public MCalculateIRStatement {
public:
    MIRVar* target;
    MIRVar* var1;
    MIRVar* var2;
    MBinaryIRType type;

    MBinaryIRStatement(MBinaryIRType type, MIRVar* target, MIRVar* var1, MIRVar* var2)
            : target(target), var1(var1), var2(var2), type(type)
            , MCalculateIRStatement(MIRStatementType::BINARY){
        if (type != MBinaryIRType::ADD && type != MBinaryIRType::SUB && type != MBinaryIRType::MULT&& type != MBinaryIRType::DIV
            && type != MBinaryIRType::MOD && type != MBinaryIRType::OR && type != MBinaryIRType::AND && type != MBinaryIRType::GRE
            && type != MBinaryIRType::GEQ && type !=MBinaryIRType::LSS && type != MBinaryIRType::LEQ && type != MBinaryIRType::EQL
            &&type != MBinaryIRType::NEQ ) {
            throw "MBinaryIRStatement: wrong type: " + binaryIRType2String(type);
        }
    }

    std::string toString() override {
        return binaryIRType2String(type) + " " + target->toString() + " " + var1->toString() + " " + var2->toString();
    }

    static std::string binaryIRType2String(MBinaryIRType type) {
        switch (type) {
            case MBinaryIRType::ADD: return "add";
            case MBinaryIRType::SUB: return "sub";
            case MBinaryIRType::MULT: return "mult";
            case MBinaryIRType::DIV: return "div";
            case MBinaryIRType::MOD: return "mod";
            case MBinaryIRType::OR: return "or";
            case MBinaryIRType::AND: return "and";
            case MBinaryIRType::GRE: return "gre";
            case MBinaryIRType::GEQ: return "geq";
            case MBinaryIRType::LSS: return "lss";
            case MBinaryIRType::LEQ: return "leq";
            case MBinaryIRType::EQL: return "eql";
            case MBinaryIRType::NEQ: return "neq";
            default:
                throw "MBinaryIRStatement::binaryIRType2String: unexpected type";
        }
    }
};

enum class MUnaryIRType {
    NOT, POS, NEG
};

class MUnaryIRStatement : public MCalculateIRStatement {
public:
    MIRVar* target;
    MIRVar* var;
    MUnaryIRType type;
    MUnaryIRStatement(MUnaryIRType type, MIRVar* target, MIRVar* var)
    : target(target), var(var), type(type)
            , MCalculateIRStatement(MIRStatementType::UNARY){
        if (type != MUnaryIRType::NOT && type != MUnaryIRType::POS && type != MUnaryIRType::NEG) {
            throw "MUnaryIRStatement: wrong type: " + unaryIRType2String(type);
        }
    }

    std::string toString() override {
        return unaryIRType2String(type) + " " + target->toString() + " " + var->toString() + " ";
    }

    static std::string unaryIRType2String(MUnaryIRType type) {
        switch (type) {
            case MUnaryIRType::NOT: return "not";
            case MUnaryIRType::POS: return "pos";
            case MUnaryIRType::NEG: return "neg";
            default:
                throw "MUnaryIRStatement::unaryIRType2String: unexpected type";
        }
    }

};

class MLabelIRStatement : public MIRStatement {
public:
    std::string label;

    MLabelIRStatement(std::string label) : label(label), MIRStatement(MIRStatementType::LABEL) {}

    std::string toString() override {
        return label + ":";
    }
};

class MEnterBlockIRStatement : public MIRStatement {
public:
    MEnterBlockIRStatement() : MIRStatement(MIRStatementType::ENTER_BLOCK) {}
    std::string toString() override {
        return "enter_block";
    }
};

class MExitBlockIRStatement : public MIRStatement {
public:
    MExitBlockIRStatement() : MIRStatement(MIRStatementType::EXIT_BLOCK) {}
    std::string toString() override {
        return "exit_block";
    }
};

class MSaveIRStatement : public MIRStatement {
public:
    MIRVar* target;
    MIRVar* subject;

    MSaveIRStatement(MIRVar* target, MIRVar* subject)
            : target(target), subject(subject), MIRStatement(MIRStatementType::SAVE) {}

    std::string toString() override {
        return "save " + target->toString() + " " + subject->toString();
    }
};

class MGetintIRStatement : public MIRStatement {
public:
    MIRVar* target;
    MGetintIRStatement(MIRVar* target) :
    target(target), MIRStatement(MIRStatementType::GETINT) {}

    std::string toString() override {
        return "getint " + target->toString();
    }
};

class MPutCharIRStatement : public MIRStatement {
public:
    char c;
    MPutCharIRStatement(char c) : c(c), MIRStatement(MIRStatementType::PUTCHAR) {}

    std::string toString() override {
        // e.g. putchar 65('A')
        if (self.c == '\n')
            return "putchar " + std::to_string(int(c)) + "('\\n')";
        return "putchar " + std::to_string(int(c)) + "('" + c + "')";
    }
};

class MPutVarIRStatement : public MIRStatement {
public:

    MPutVarIRStatement()
    : MIRStatement(MIRStatementType::PUTVAR) {}

    std::string toString() override {
        return "putvar ";
    }
};

class MExitIRStatement : public MIRStatement {
public:
    MExitIRStatement() : MIRStatement(MIRStatementType::EXIT) {}

    std::string toString() override {
        return "exit";
    }
};

class MBranchIRStatement : public MIRStatement {
public:
    std::string label;
    MBranchIRStatement(std::string label)
    : label(label), MIRStatement(MIRStatementType::BR) {}

    std::string toString() override {
        return "br " + label;
    }
};

class MBranchNotTrueIRStatement : public MIRStatement {
public:
    MIRVar* var;
    std::string label;
    MBranchNotTrueIRStatement(MIRVar* var, std::string label) :
    var(var), label(label), MIRStatement(MIRStatementType::BNT) {};

    std::string toString() override {
        return "bnt " + var->toString() + " " + label;
    }
};

#endif //BUAA_COMPILER_IRSTATEMENTS_H
