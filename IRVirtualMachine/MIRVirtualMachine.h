#ifndef BUAA_COMPILER_MIRVIRTUALMACHINE_H
#define BUAA_COMPILER_MIRVIRTUALMACHINE_H

#include "../IR/IRStatements.h"
#include "MRAM.h"
#include "MIRSymbolTable.h"
#include <unordered_map>

class MIRVirtualMachine {
private:
    MRAM *ram;
    MIRSymbolTable *global;
    MIRSymbolTable *current;
    std::vector<MIRStatement *> *irStatements;
    int pc;
    std::unordered_map<std::string, int> *labelMap;
    std::string ans;

public:
    MIRVirtualMachine(std::vector<MIRStatement *> *irStatements)
            : ram(new MRAM()), pc(0), irStatements(irStatements),
              global(new MIRSymbolTable(nullptr)), ans(""),
              labelMap(new std::unordered_map<std::string, int>()) {
        self.current = global;
        int i = 0;
        for (auto ir: *(self.irStatements)) {
            if (ir->type == MIRStatementType::LABEL) {
                self.bindLabel(((MLabelIRStatement *) ir)->label, i);
            }
            i++;
        }
    }

    void run() {
        while ((*(self.irStatements))[self.pc]->type != MIRStatementType::EXIT) {
            self.run((*(self.irStatements))[self.pc]);
            pc++;
        }
    }

    std::string output() {
        return self.ans;
    }

private:
    void bindLabel(std::string label, int offset) {
        if (self.labelMap->find(label) != self.labelMap->end())
            throw "MIRVirtualMachine::bindLabel: duplicated label occurred.";
        (*(self.labelMap))[label] = offset;
    }

    static int calculateSize(MBaseType *type) {
        if (type->isArray) {
            if (type->isParam) {
                return 1;
            } else {
                int size = 1;
                for (auto dim: *(type->dims)) {
                    size *= dim;
                }
                return size;
            }
        } else {
            return 1;
        }
    }

    MIRSymbolTableItem *getTableItem(std::string name) {
        for (auto table = self.current; table != nullptr; table = table->father) {
            if (table->contains(name)) {
                return table->getItem(name);
            }

        }
        throw "MIRVirtualMachine::getTableItem: item not found";
    }

    int loadIRVar(MIRVar* irVar) {
        if (irVar->getTypeID() == MIRVarTypeID::REG_IR_VAR) {
            return self.ram->loadReg(((MImmIRVar*)irVar)->value);
        } else if (irVar->getTypeID() == MIRVarTypeID::VAR_IR_VAR) {
            // 说明是变量
            auto varIRVar = (MVarIRVar*)irVar;
            auto item = self.getTableItem(varIRVar->name);
            if (item->getTypeID() != MIRSymbolTableItemTypeID::VAR_IR_SYMBOL_TABLE_ITEM) {
                throw "MIRVirtualMachine::loadIRVar: should be a var but a func";
            }
            auto varItem = (MVarIRSymbolTableItem *) item;
            int address = varItem->address;
            if (varItem->varType->isArray && varIRVar->getOffset() == nullptr)
                //是数组而没有偏移量，则返回数组的首地址
                if (varItem->varType->isParam)
                    // 要将参数数组作为参数传递
                    return self.ram->load(varItem->address);
                else
                    return address;
            if (varItem->varType->isParam && varIRVar->getOffset() != nullptr)
                // 是参数又是数组，则一定是一个指针，计算指向的地址
                address = self.ram->load(varItem->address);
            if (varIRVar->getOffset() != nullptr)
                // 是数组就要加上偏移量
                address += self.loadIRVar(varIRVar->getOffset());
            return self.ram->load(address);
        } else {
            // 一定是一个立即数
            return ((MImmIRVar*)irVar)->value;
        }

    }

    void saveIRVar(MIRVar* irVar, int value) {
        if (irVar->getTypeID() == MIRVarTypeID::REG_IR_VAR) {
            // 临时寄存器
            self.ram->saveReg(value, ((MRegIRVar*)irVar)->regID);
        } else if (irVar->getTypeID() == MIRVarTypeID::VAR_IR_VAR) {
            // 变量
            auto varIRVar = (MVarIRVar*)irVar;
            auto item = self.getTableItem(varIRVar->name);
            if (item->getTypeID() != MIRSymbolTableItemTypeID::VAR_IR_SYMBOL_TABLE_ITEM) {
                throw "MIRVirtualMachine::saveIRVar: should be a var but a func";
            }
            auto varItem = (MVarIRSymbolTableItem *) item;
            int address = varItem->address;
            if (varItem->varType->isArray && varIRVar->getOffset() == nullptr)
                //是数组而没有偏移量，则返回数组的首地址
                if (varItem->varType->isParam) {
                    // 要将数组赋值给数组指针（参数数组）
                    self.ram->save(value, varItem->address);
                    return;
                } else {
                    // 要将数组赋值给数组，理论上不允许
                    throw "MIRVirtualMachine::saveIRVar: assign array to array";
                }
            if (varItem->varType->isParam && varIRVar->getOffset() != nullptr)
                // 是参数又是数组，则一定是一个指针，计算指向的地址
                address = self.ram->load(varItem->address);
            if (varIRVar->getOffset() != nullptr)
                // 是数组就要加上偏移量
                address += self.loadIRVar(varIRVar->getOffset());
            self.ram->save(value, address);
        } else {
            // 一定是一个立即数
            throw "MIRVirtualMachine::saveIRVar: irVar is not a TempIRVar nor a IRVar but a number";
        }
    }

    std::string irVar2Ident(std::string irVar) {
        int i = 0;
        for (; irVar[i] != '[' && i != irVar.size(); i++);
        return irVar.substr(0, i);
    }

    void runDecl(MDeclIRStatement *ir) {
        if (self.current->contains(ir->name))
            throw "MIRVirtualMachine::runDecl: redefination";
        int address = ram->allocate(self.calculateSize(ir->type));
        self.current->addItem(
                new MVarIRSymbolTableItem(self.irVar2Ident(ir->name), ir->type, address));

        return;
    }

    void runFuncDef(MFuncDefIRStatement *ir) {
        if (self.current->contains(ir->funcName))
            throw "MIRVirtualMachine::runFuncDef: redefination";
        self.current->addItem(new MFuncIRSymbolTableItem(ir->funcName));
        return;
    }

    void runCallFunc(MCallFuncIRStatement *ir) {
        if (self.labelMap->find(ir->funcName) == self.labelMap->end())
            throw "MIRVirtualMachine::runCallFunc: corresponding label not found";

        self.ram->push(self.pc); // 记录下call语句的pc
        self.ram->push(self.current->id);
        self.ram->savePtr();
        // 以下部分留着删除参数栈之后再写
//        auto item = self.getTableItem(ir->funcName);
//        if (item->getTypeID() != MIRSymbolTableItemTypeID::FUNC_IR_SYMBOL_TABLE_ITEM) {
//            throw "MIRVirtualMachine::runCallFunc: should be a func but a var";
//        }
        self.pc = (*(self.labelMap))[ir->funcName];
    }

    void runReturn(MReturnIRStatement *ir) {
        self.ram->loadPtr();
        // 将pc设置为call语句的pc，run的循环里还会pc++到call的下一条
        int preTableID = self.ram->pop();
        self.pc = self.ram->pop();
        while (self.current->id != preTableID)
            self.runExitBlock();
    }

    void runPush(MPushIRStatement *ir) {
        self.ram->pushParam(self.loadIRVar(ir->irVar));
    }

    void runPop(MPopIRStatement *ir) {
        int value = self.ram->popParam();
        self.saveIRVar(ir->irVar, value);
    }

    void runBinary(MBinaryIRStatement *ir) {
        int value1 = self.loadIRVar(ir->var1);
        int value2 = self.loadIRVar(ir->var2);
        if (ir->type == MBinaryIRType::ADD) {
            self.saveIRVar(ir->target, value1 + value2);
        } else if (ir->type == MBinaryIRType::SUB) {
            self.saveIRVar(ir->target, value1 - value2);
        } else if (ir->type == MBinaryIRType::MULT) {
            self.saveIRVar(ir->target, value1 * value2);
        } else if (ir->type == MBinaryIRType::DIV) {
            self.saveIRVar(ir->target, value1 / value2);
        } else if (ir->type == MBinaryIRType::MOD) {
            self.saveIRVar(ir->target, value1 % value2);
        } else if (ir->type == MBinaryIRType::OR) {
            self.saveIRVar(ir->target, value1 || value2);
        } else if (ir->type == MBinaryIRType::AND) {
            self.saveIRVar(ir->target, value1 && value2);
        } else if (ir->type == MBinaryIRType::EQL) {
            self.saveIRVar(ir->target, value1 == value2);
        } else if (ir->type == MBinaryIRType::NEQ) {
            self.saveIRVar(ir->target, value1 != value2);
        } else if (ir->type == MBinaryIRType::LSS) {
            self.saveIRVar(ir->target, value1 < value2);
        } else if (ir->type == MBinaryIRType::LEQ) {
            self.saveIRVar(ir->target, value1 <= value2);
        } else if (ir->type == MBinaryIRType::GRE) {
            self.saveIRVar(ir->target, value1 > value2);
        } else if (ir->type == MBinaryIRType::GEQ) {
            self.saveIRVar(ir->target, value1 >= value2);
        } else {
            throw "MIRVirtualMachine::runBinary: unhandled Binary calculation type";
        }
    }

    void runUnary(MUnaryIRStatement *ir) {
        int value = self.loadIRVar(ir->var);
        if (ir->type == MUnaryIRType::POS) {
            self.saveIRVar(ir->target, value);
        } else if (ir->type == MUnaryIRType::NEG) {
            self.saveIRVar(ir->target, -value);
        } else if (ir->type == MUnaryIRType::NOT) {
            self.saveIRVar(ir->target, value == 0 ? 1 : 0);
        } else {
            throw "MIRVirtualMachine::runUnary: unhandled Unary calculation type";
        }
    }

    void runLabel(MLabelIRStatement *ir) {}

    void runEnterBlock() {
        self.current->setChild(new MIRSymbolTable(self.current));
        self.current = self.current->getChild();
    }

    void runExitBlock() {
        self.current = self.current->getFather();

        delete (self.current->getChild());
        self.current->setChild(nullptr);
    }

    void runSave(MSaveIRStatement *ir) {
        self.saveIRVar(ir->target, self.loadIRVar(ir->subject));
    }

    void runGetint(MGetintIRStatement *ir) {
        int value;
        scanf("%d", &value);
        self.saveIRVar(ir->target, value);
    }

    void runPutchar(MPutCharIRStatement *ir) {
        ans += ir->c;
    }

    void runPutvar(MPutVarIRStatement *ir) {
        std::string value = std::to_string(self.ram->popParam());
        self.ans += value;
    }

    void runBranch(MBranchIRStatement *ir) {
        self.pc = (*(self.labelMap))[ir->label];
    }

    void runBranchNotTrue(MBranchNotTrueIRStatement *ir) {
        int value = self.loadIRVar(ir->var);
        if (value == 0)
            self.pc = (*(self.labelMap))[ir->label];
    }

    void run(MIRStatement *ir) {
        switch (ir->type) {
            case MIRStatementType::DECL: {
                self.runDecl((MDeclIRStatement *) ir);
                return;
            }
            case MIRStatementType::FUNC_DEF: {
                self.runFuncDef((MFuncDefIRStatement *) ir);
                return;
            }
            case MIRStatementType::CALL_FUNC: {
                self.runCallFunc((MCallFuncIRStatement *) ir);
                return;
            }
            case MIRStatementType::RETURN: {
                self.runReturn((MReturnIRStatement *) ir);
                return;
            }
            case MIRStatementType::PUSH: {
                self.runPush((MPushIRStatement *) ir);
                return;
            }
            case MIRStatementType::POP: {
                self.runPop((MPopIRStatement *) ir);
                return;
            }
            case MIRStatementType::BINARY: {
                self.runBinary((MBinaryIRStatement *) ir);
                return;
            }
            case MIRStatementType::UNARY: {
                self.runUnary((MUnaryIRStatement *) ir);
                return;
            }
            case MIRStatementType::LABEL: {
                self.runLabel((MLabelIRStatement *) ir);
                return;
            }
            case MIRStatementType::ENTER_BLOCK: {
                self.runEnterBlock();
                return;
            }
            case MIRStatementType::EXIT_BLOCK: {
                self.runExitBlock();
                return;
            }
            case MIRStatementType::SAVE: {
                self.runSave((MSaveIRStatement *) ir);
                return;
            }
            case MIRStatementType::GETINT: {
                self.runGetint((MGetintIRStatement *) ir);
                return;
            }
            case MIRStatementType::PUTCHAR: {
                self.runPutchar((MPutCharIRStatement *) ir);
                return;
            }
            case MIRStatementType::PUTVAR: {
                self.runPutvar((MPutVarIRStatement *) ir);
                return;
            }
            case MIRStatementType::BR: {
                self.runBranch((MBranchIRStatement *) ir);
                return;
            }
            case MIRStatementType::BNT: {
                self.runBranchNotTrue((MBranchNotTrueIRStatement *) ir);
                return;
            }
            default:
                throw "MIRVirtualMachine::run: unhandled MIRStatementType";

        }
    }
};

#endif //BUAA_COMPILER_MIRVIRTUALMACHINE_H
