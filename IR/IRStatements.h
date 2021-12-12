#ifndef BUAA_COMPILER_IRSTATEMENTS_H
#define BUAA_COMPILER_IRSTATEMENTS_H

enum class IRType {
    DECL, FUNC_DEF, CALL_FUNC, RETURN, PUSH, POP,
    BINARY, UNARY, LABEL, ENTER_BLOCK, EXIT_BLOCK,
    SAVE, GETINT, PUTCHAR, PUTVAR, EXIT, BNT, BR
};

class MIRStatement {
private:
    MIRStatement(){}
public:
    IRType type;
    virtual std::string toString() = 0;

protected:
    MIRStatement(IRType type) : type(type) {}
};

class MDeclIRStatement : public MIRStatement {
public:
    MBaseType *type;
    std::string name;
public:
    MDeclIRStatement(MBaseType *type, const std::string &name)
            : type(type), name(name), MIRStatement(IRType::DECL) {}

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
            paramNames(paramNames), MIRStatement(IRType::FUNC_DEF) {
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
            , MIRStatement(IRType::CALL_FUNC){}

    std::string toString() override {
        return "call " + funcName;
    }
};

class MReturnIRStatement : public MIRStatement {
public:
    MReturnIRStatement() : MIRStatement(IRType::RETURN) {}
    std::string toString() override {
        return "return";
    }
};

class MPushIRStatement : public MIRStatement {
public:
    std::string varName;

    MPushIRStatement(std::string varName) :
    varName(varName), MIRStatement(IRType::PUSH) {}

    std::string toString() override {
        return "push " + varName;
    }
};

class MPopIRStatement : public MIRStatement {
public:
    std::string varName;

    MPopIRStatement(std::string varName) :
    varName(varName), MIRStatement(IRType::POP) {}

    std::string toString() override {
        return "pop " + varName;
    }
};

class MCalculateIRStatement : public MIRStatement {
public:
//    static enum class OPERATOR_TYPE {
//        ADD, SUB, MULT, DIV, MOD, OR, AND, GRE,
//        GEQ, LSS, LEQ, EQL, NEQ, NOT, POS, NEG
//    };
    // OPERATOR_TYPE type;
    MCalculateIRStatement(IRType type) : MIRStatement(type) {}
};

class MBinaryIRStatement : public MCalculateIRStatement {
public:
    std::string target;
    std::string var1;
    std::string var2;
    std::string type;

    MBinaryIRStatement(std::string type, std::string target, std::string var1, std::string var2)
            : target(target), var1(var1), var2(var2), type(type)
            , MCalculateIRStatement(IRType::BINARY){
        if (type != "add" && type != "sub" && type != "mult" && type != "div"
            && type != "mod" && type != "or" && type != "and" && type != "gre"
            && type != "geq" && type !="lss" && type != "leq" && type != "eql"
            &&type != "neq" && type != "not" && type != "pos" && type != "neg") {
            throw "MBinaryIRStatement: wrong type: " + type;
        }
    }

    std::string toString() override {
        return type + " " + target + " " + var1 + " " + var2;
    }
};

class MUnaryIRStatement : public MCalculateIRStatement {
public:
    std::string target;
    std::string var;
    std::string type;
    MUnaryIRStatement(std::string type, std::string target, std::string var)
    : target(target), var(var), type(type)
            , MCalculateIRStatement(IRType::UNARY){
        if (type != "not" && type != "pos" && type != "neg") {
            throw "MUnaryIRStatement: wrong type: " + type;
        }
    }

    std::string toString() override {
        return type + " " + target + " " + var + " ";
    }

};

class MLabelIRStatement : public MIRStatement {
public:
    std::string label;

    MLabelIRStatement(std::string label) : label(label), MIRStatement(IRType::LABEL) {}

    std::string toString() override {
        return label + ":";
    }
};

class MEnterBlockIRStatement : public MIRStatement {
public:
    MEnterBlockIRStatement() : MIRStatement(IRType::ENTER_BLOCK) {}
    std::string toString() override {
        return "enter_block";
    }
};

class MExitBlockIRStatement : public MIRStatement {
public:
    MExitBlockIRStatement() : MIRStatement(IRType::EXIT_BLOCK) {}
    std::string toString() override {
        return "exit_block";
    }
};

class MSaveIRStatement : public MIRStatement {
public:
    std::string target;
    std::string subject;

    MSaveIRStatement(std::string target, std::string subject)
            : target(target), subject(subject), MIRStatement(IRType::SAVE) {}

    std::string toString() override {
        return "save " + target + " " + subject;
    }
};

class MGetintIRStatement : public MIRStatement {
public:
    std::string target;
    MGetintIRStatement(std::string target) :
    target(target), MIRStatement(IRType::GETINT) {}

    std::string toString() override {
        return "getint " + target;
    }
};

class MPutCharIRStatement : public MIRStatement {
public:
    char c;
    MPutCharIRStatement(char c) : c(c), MIRStatement(IRType::PUTCHAR) {}

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
    : MIRStatement(IRType::PUTVAR) {}

    std::string toString() override {
        return "putvar ";
    }
};

class MExitIRStatement : public MIRStatement {
public:
    MExitIRStatement() : MIRStatement(IRType::EXIT) {}

    std::string toString() override {
        return "exit";
    }
};

class MBranchIRStatement : public MIRStatement {
public:
    std::string label;
    MBranchIRStatement(std::string label)
    : label(label), MIRStatement(IRType::BR) {}

    std::string toString() override {
        return "br " + label;
    }
};

class MBranchNotTrueIRStatement : public MIRStatement {
public:
    std::string var;
    std::string label;
    MBranchNotTrueIRStatement(std::string var, std::string label) :
    var(var), label(label), MIRStatement(IRType::BNT) {};

    std::string toString() override {
        return "bnt " + var + " " + label;
    }
};

#endif //BUAA_COMPILER_IRSTATEMENTS_H
