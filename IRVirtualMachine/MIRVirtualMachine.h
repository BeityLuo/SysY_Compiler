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

    bool doCout;

public:
    MIRVirtualMachine(std::vector<MIRStatement *> *irStatements, bool doCout)
            : ram(new MRAM()), pc(0), irStatements(irStatements), doCout(doCout),
              global(new MIRSymbolTable(nullptr)), ans(""),
              labelMap(new std::unordered_map<std::string, int>()) {
        self.current = global;
        int i = 0;
        for (auto ir: *(self.irStatements)) {
            if (ir->type == IRType::LABEL) {
                self.bindLabel(((MLabelIRStatement *) ir)->label, i);
            }
            i++;
        }
    }

    void run() {
        while ((*(self.irStatements))[self.pc]->type != IRType::EXIT) {
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

    int calcuSize(MBaseType *type) {
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
        auto table = self.current;
        for (auto table = self.current; table != nullptr; table = table->father) {
            if (table->contains(name)) {
                return table->getItem(name);
            }

        }
        throw "MIRVirtualMachine::getTableItem: item not found";
    }

    int loadIRVar(std::string irVar) {
        if (irVar[0] == '%') {
            // 说明是立即数
            return self.ram->loadTempRegister(irVar.substr(1, irVar.size() - 1));
        } else if (!(irVar[0] >= '0' && irVar[0] <= '9' || irVar[0] == '-')) {
            // 说明是变量
            auto item = self.getTableItem(self.irVar2Ident(irVar));
            if (item->getTypeID() != MIRSymbolTableItemTypeID::VAR_IR_SYMBOL_TABLE_ITEM) {
                throw "MIRVirtualMachine::loadIRVar: should be a var but a func";
            }
            auto varItem = (MVarIRSymbolTableItem *) item;
            int address = varItem->address;
            if (varItem->varType->isParam && varItem->varType->isArray) {
                address = self.ram->load(varItem->address);
                // true代表了是param
                auto arrayIndexes = self.analyzeArrayIndexes(irVar);
                if (arrayIndexes->size() == 0) {
                    // 说明要的是指针而不是值
                    return address;
                }

                address += self.calcuArrayOffset(varItem->varType->dims, arrayIndexes, true);
            } else if (varItem->varType->isArray) {
                auto arrayIndexes = self.analyzeArrayIndexes(irVar);
                if (arrayIndexes->size() == 0) {
                    // 说明要的是指针而不是值
                    return address;
                }
                address += self.calcuArrayOffset(varItem->varType->dims, arrayIndexes);
            }
            return self.ram->load(address);
        } else {
            // 一定是一个立即数
            return std::stoi(irVar);
        }

    }

    void saveIRVar(std::string irVar, int value) {
        if (irVar[0] == '%') {
            // 临时寄存器
            self.ram->saveTempRegister(value, irVar.substr(1, irVar.size() - 1));
        } else if (!(irVar[0] >= '0' && irVar[0] <= '9' || irVar[0] == '-')) {
            // 变量
            auto item = self.getTableItem(self.irVar2Ident(irVar));
            if (item->getTypeID() != MIRSymbolTableItemTypeID::VAR_IR_SYMBOL_TABLE_ITEM) {
                throw "MIRVirtualMachine::saveIRVar: should be a var but a func";
            }
            auto varItem = (MVarIRSymbolTableItem *) item;
            int address = varItem->address;
            if (varItem->varType->isParam && varItem->varType->isArray) {
                address = self.ram->load(varItem->address);
                // true代表了是param
                auto arrayIndexes = self.analyzeArrayIndexes(irVar);
                if (arrayIndexes->size() == 0) {
                    // 说明要的是指针而不是值
                    return self.ram->save(value, varItem->address);
                }
                address += self.calcuArrayOffset(varItem->varType->dims, arrayIndexes, true);
            } else if (varItem->varType->isArray) {
                auto arrayIndexes = self.analyzeArrayIndexes(irVar);
                if (arrayIndexes->size() == 0) {
                    // 说明要的是指针而不是值
                    return self.ram->save(value, varItem->address);
                }
                address += self.calcuArrayOffset(varItem->varType->dims, arrayIndexes);
            }
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


    std::vector<int> *analyzeArrayIndexes(std::string var) {
        // e.g. var = $a[2][3], output = [2, 3];
        auto ans = new std::vector<int>();
        int i = 0;
        for (; i < var.size() && var[i] != '['; i++);
        i++;
//        if (isParam) {
//            i += 2;
//        } //把][跳过去
        for (; i < var.size(); i++) {
            if (var[i] >= '0' && var[i] <= '9') {
                // 处理x[1]这种
                int index = 0;
                for (; i < var.size() && var[i] >= '0' && var[i] <= '9'; i++)
                    index = index * 10 + var[i] - '0';
                ans->push_back(index);
            } else if (var[i] == '%') {
                // 处理x[%0]这种
                std::string tempIR = "%";
                i++;
                for (; i < var.size() && var[i] >= '0' && var[i] <= '9'; i++)
                    tempIR += var[i];
                ans->push_back(this->loadIRVar(tempIR));
            } else if (var[i] >= 'a' && var[i] <= 'z' || var[i] == '_' ||
                       var[i] >= 'A' && var[i] <= 'Z') {
                // 处理x[x[0]]这种
                std::string name = "";
                for (; i < var.size() && var[i] >= 'a' && var[i] <= 'z' ||
                       var[i] == '_' || var[i] >= 'A' && var[i] <= 'Z'; i++)
                    name += var[i];
                ans->push_back(self.loadIRVar(name));
            }
        }
        return ans;
    }

    int calcuArrayOffset(std::vector<int> *dims, std::vector<int> *arrayIndexes, bool isParam=false) {
        //忘了有啥用了
        if (arrayIndexes->size() == 0)
            return 0;
        // 如果是param的话，dim比arrayIndexes少一维
        if (!isParam && dims->size() != arrayIndexes->size() ||
                dims->size() != arrayIndexes->size() - 1)
            throw "MIRVirtualMachine::calcuArrayOffset: dims and arrayIndexes not match";

        int num = arrayIndexes->size();
        int offset = (*(arrayIndexes))[num - 1];
        for (int i = num - 2; i >= 0; i--) {
            int size = 1;
            for (int j = i + 1; j < num; j++) {
                if (isParam)
                    size *= (*(dims))[j-1];
                else
                    size *= (*(dims))[j];
            }
            offset += (*(arrayIndexes))[i] * size;
        }
        return offset;
    }

    void runDecl(MDeclIRStatement *ir) {
        if (self.current->contains(ir->name))
            throw "MIRVirtualMachine::runDecl: redefination";
        int address = ram->allocate(self.calcuSize(ir->type));
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
        self.ram->pushParam(self.loadIRVar(ir->varName));
    }

    void runPop(MPopIRStatement *ir) {
        int value = self.ram->popParam();
        self.saveIRVar(ir->varName, value);
    }

    void runBinary(MBinaryIRStatement *ir) {
        int value1 = self.loadIRVar(ir->var1);
        int value2 = self.loadIRVar(ir->var2);
        if (ir->type == "add") {
            self.saveIRVar(ir->target, value1 + value2);
        } else if (ir->type == "sub") {
            self.saveIRVar(ir->target, value1 - value2);
        } else if (ir->type == "mult") {
            self.saveIRVar(ir->target, value1 * value2);
        } else if (ir->type == "div") {
            self.saveIRVar(ir->target, value1 / value2);
        } else if (ir->type == "mod") {
            self.saveIRVar(ir->target, value1 % value2);
        } else if (ir->type == "or") {
            self.saveIRVar(ir->target, value1 || value2);
        } else if (ir->type == "and") {
            self.saveIRVar(ir->target, value1 && value2);
        } else if (ir->type == "eql") {
            self.saveIRVar(ir->target, value1 == value2);
        } else if (ir->type == "neq") {
            self.saveIRVar(ir->target, value1 != value2);
        } else if (ir->type == "lss") {
            self.saveIRVar(ir->target, value1 < value2);
        } else if (ir->type == "leq") {
            self.saveIRVar(ir->target, value1 <= value2);
        } else if (ir->type == "gre") {
            self.saveIRVar(ir->target, value1 > value2);
        } else if (ir->type == "geq") {
            self.saveIRVar(ir->target, value1 >= value2);
        } else {
            throw "MIRVirtualMachine::runBinary: unhandled Binary calculation type";
        }
    }

    void runUnary(MUnaryIRStatement *ir) {
        int value = self.loadIRVar(ir->var);
        if (ir->type == "pos") {
            self.saveIRVar(ir->target, value);
        } else if (ir->type == "neg") {
            self.saveIRVar(ir->target, -value);
        } else if (ir->type == "not") {
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
        if (doCout)
            printf("%c", ir->c);
    }

    void runPutvar(MPutVarIRStatement *ir) {
        std::string value = std::to_string(self.ram->popParam());
        self.ans += value;
        if (doCout)
            printf("%s", value.c_str());
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
            case IRType::DECL: {
                self.runDecl((MDeclIRStatement *) ir);
                return;
            }
            case IRType::FUNC_DEF: {
                self.runFuncDef((MFuncDefIRStatement *) ir);
                return;
            }
            case IRType::CALL_FUNC: {
                self.runCallFunc((MCallFuncIRStatement *) ir);
                return;
            }
            case IRType::RETURN: {
                self.runReturn((MReturnIRStatement *) ir);
                return;
            }
            case IRType::PUSH: {
                self.runPush((MPushIRStatement *) ir);
                return;
            }
            case IRType::POP: {
                self.runPop((MPopIRStatement *) ir);
                return;
            }
            case IRType::BINARY: {
                self.runBinary((MBinaryIRStatement *) ir);
                return;
            }
            case IRType::UNARY: {
                self.runUnary((MUnaryIRStatement *) ir);
                return;
            }
            case IRType::LABEL: {
                self.runLabel((MLabelIRStatement *) ir);
                return;
            }
            case IRType::ENTER_BLOCK: {
                self.runEnterBlock();
                return;
            }
            case IRType::EXIT_BLOCK: {
                self.runExitBlock();
                return;
            }
            case IRType::SAVE: {
                self.runSave((MSaveIRStatement *) ir);
                return;
            }
            case IRType::GETINT: {
                self.runGetint((MGetintIRStatement *) ir);
                return;
            }
            case IRType::PUTCHAR: {
                self.runPutchar((MPutCharIRStatement *) ir);
                return;
            }
            case IRType::PUTVAR: {
                self.runPutvar((MPutVarIRStatement *) ir);
                return;
            }
            case IRType::BR: {
                self.runBranch((MBranchIRStatement *) ir);
                return;
            }
            case IRType::BNT: {
                self.runBranchNotTrue((MBranchNotTrueIRStatement *) ir);
                return;
            }
            default:
                throw "MIRVirtualMachine::run: unhandled IRType";

        }
    }
};

#endif //BUAA_COMPILER_MIRVIRTUALMACHINE_H
