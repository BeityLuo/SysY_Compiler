#ifndef BUAA_COMPILER_IRGENERATOR_H
#define BUAA_COMPILER_IRGENERATOR_H


#include "../ast_generate/syntax_nodes.h"
#include "MSymbolTable.h"
#include "../IR/IRStatements.h"
#include "../IR/IRVar.h"
#include "MLabelManager.h"
#include <queue>
#include "MExpCalculator.h"


class IRGenerator {
private:
    MSymbolTable *globalTable;
    MSymbolTable *currentTable;
    std::vector<MIRStatement *> *irStatements;
    MLabelManager *labelManager;
    MExpCalculator *expCalculator;
public:
    explicit IRGenerator(MSymbolTable *globalTable)
            : globalTable(globalTable), currentTable(globalTable),
              irStatements(new std::vector<MIRStatement *>()),
              labelManager(new MLabelManager()),
              expCalculator(new MExpCalculator()) {}

    std::string toString() {
        std::string ans;
        for (auto irStatement: *(self.irStatements)) {
            ans += irStatement->toString() + "\n";
        }
        return ans;
    }

    std::vector<MIRStatement *> *getIRStatements() {
        return self.irStatements;
    }

    bool isRedefined(const std::string &name) {
        // 变量名可以与函数名相同，所以不用考虑；
        return self.currentTable->contains(name);
    }

    bool isUndefined(const std::string &name) {
        for (auto table = self.currentTable; table != nullptr;
             table = table->getFatherScopeTable()) {
            if (table->contains(name))
                return false;
        }
        return true;
    }

    bool isConst(const std::string &name) {
        for (auto table = self.currentTable; table != nullptr;
             table = table->getFatherScopeTable()) {

            if (table->contains(name)) {
                auto item = table->getTableItem(name);
                if (item->getTypeID() != MSymbolTableItemTypeID::VAR_SYMBOLTABLE_ITEM)
                    throw "IRGenerator::isConst: 名字是函数名";
                return ((MVariableSymbolTableItem *) item)->variableType->isConst;
            }
        }
        throw "IRGenerator::isConst: 名字未定义";
    }

    bool isInCyclicBlock() {
        return self.currentTable->isCyclicBlock();
    }

    bool isInFunctionWithReturn() {
        auto funcItem = self.currentTable->getFatherTableItem();
        if (funcItem == nullptr)
            throw "IRGenerator::isInFunctionWithReturn";
        return funcItem->returnType->type->type != Token::VOIDTK;
    }

    bool expMatchParamType(MExp *exp, MParamType *paramType) {

        return true;
    }

    void createTable(const std::string &funcName = "") {
        if (funcName.empty()) {
            auto child = new MSymbolTable(self.currentTable);
            self.currentTable->setChildScopeTable(child);
            self.currentTable = child;
        } else {
            auto funcTableItem = self.currentTable->getTableItem(funcName);
            auto child = new MSymbolTable(self.currentTable);
            self.currentTable->setChildScopeTable(child);
            self.currentTable = child;
            if (funcTableItem != nullptr) {
                ((MFunctionSymbolTableItem *) funcTableItem)->setTable(self.currentTable);
            } else {
                throw "createTable: 函数名不存在";
            }

        }
        self.addIR(new MEnterBlockIRStatement());
    }

    void backTable(bool destroy = true) {
        if (destroy) {
            self.currentTable = self.currentTable->getFatherScopeTable();
            delete (self.currentTable->getChildScopTable());
            self.currentTable->setChildScopeTable(nullptr);
        } else {
            self.currentTable = self.currentTable->getFatherScopeTable();
            self.currentTable->setChildScopeTable(nullptr);
        }
        self.addIR(new MExitBlockIRStatement());
    }

    void addVar2Table(MType *type, std::vector<int> *dims,
                      std::string name, bool isConst, MVariableInit *init, int lineNum) {
        // TODO 对于非const的变量，要执行动态数组初始化
        if (self.isRedefined(name)) {
            MExceptionManager::pushException(
                    new MRedefinedIdentifierException(lineNum));
            // 给他换个名字，确保接下来的行为还能够进行
            do {
                name = name + "_redefined";
            } while (self.isRedefined(name));
        }
        auto varType = new MBaseType(isConst, type, dims);
        // 生成declare中间代码和初始化代码
        self.addIR(
                new MDeclIRStatement(varType, name));
        auto initExpArray = self.initVal2ExpArray((MInitVal *) init);
        if (isConst) {
            auto initArray = self.constInitVal2ConstArray((MConstInitVal *) init);
            self.currentTable->addSymbolItem(
                    new MVariableSymbolTableItem(name, varType, initExpArray, initArray));
            self.initConstVariable(name);
        } else {

            self.currentTable->addSymbolItem(
                    new MVariableSymbolTableItem(name, varType, initExpArray));
            self.initVariable(name);
        }
        self.resetRegUsage();
    }


    std::string addFunc2Table(MType *type, std::string name, MFuncFParams *funcFParams, int lineNum) {
        if (name == "main") {
            self.insertCallMain();
        }
        if (self.isRedefined(name)) {
            MExceptionManager::pushException(
                    new MRedefinedIdentifierException(lineNum));
            // 给他换个名字，确保接下来的行为还能够进行
            do {
                name = name + "_redefined";
            } while (self.isRedefined(name));
        }
        int paramNum;
        auto paramTypes = new std::vector<MParamType *>();
        auto paramNames = new std::vector<std::string>();
        auto paramLineNum = new std::vector<int>();
        if (funcFParams != nullptr) {
            paramNum = funcFParams->funcFParams->size();
            for (auto para: *(funcFParams->funcFParams)) {
                paramTypes->push_back(new MParamType(para->bType, para->dims, para->isArray));
                paramNames->push_back(para->ident->name);
                paramLineNum->push_back(para->ident->lineNum);
            }
        } else {
            paramNum = 0;
        }
        auto funcType = new MReturnType(type);
        self.currentTable->addSymbolItem(
                new MFunctionSymbolTableItem(funcType, name, paramNum, paramTypes, paramNames, paramLineNum));

        // 生成def中间代码和参数初始化代码
        // 把def放到最前面去，这样就可以从第一条ir开始顺序执行
        self.addIR2Front(new MFuncDefIRStatement(funcType, name, paramNum, paramTypes, paramNames));
        self.addIR(new MLabelIRStatement(
                self.labelManager->getFuncLabel(name)));
        // 可能会把函数名字给改掉，导致参数无法正常初始化，所以返回一个新名字
        return name;
    }

    void initParams(std::string &funcName) {
        auto symbolItem = self.globalTable->getTableItem(funcName);
        if (symbolItem == nullptr) {
            throw "IRGenerator::initParams: funcName not exists";
        } else if (symbolItem->getTypeID() != FUNC_SYMBOLTABLE_ITEM) {
            throw "IRGenerator::initParams: item is not func item";
        }
        for (int i = ((MFunctionSymbolTableItem*)symbolItem)->paramNum - 1; i >= 0; i--) {
            // 必须倒序
            auto paramName = (*(((MFunctionSymbolTableItem*)symbolItem)->paramNames))[i];
            if (self.isRedefined(paramName)) {
                // 参数也要检查重定义。只可能和前面的参数重复
                do {
                    paramName = paramName + "_redefined";
                } while (self.isRedefined(paramName));
                (*(((MFunctionSymbolTableItem*)symbolItem)->paramNames))[i] = paramName;
                auto lineNum = (*(((MFunctionSymbolTableItem*)symbolItem)->paramLineNum))[i];
                MExceptionManager::pushException(
                        new MRedefinedIdentifierException(lineNum));
            }

            auto paramType = (*(((MFunctionSymbolTableItem*)symbolItem)->paramTypes))[i];
            if (paramType->isArray)
                // 如果参数是数组类型，就都转为一维数组指针
                // 中间代码生成会负责把多维数组的偏移转换为一维数组的偏移，因此中间代码不需要保留参数的信息
                self.addIR(new MDeclIRStatement(new MParamType(new MBType(), nullptr, true), paramName));
            else
                self.addIR(new MDeclIRStatement(paramType, paramName));

            self.addIR(new MPopIRStatement(new MVarIRVar(paramName)));
            self.currentTable->addSymbolItem(new MVariableSymbolTableItem(paramName, paramType, nullptr));
        }
    }

    int constExp2int(MConstExp *constExp) {
        return self.expCalculator->
                calculateConstExp(constExp, self.globalTable, self.currentTable);
    }

    std::vector<int> *constInitVal2ConstArray(MConstInitVal *constInitVal) {
        if (constInitVal == nullptr) {
            return nullptr;
        }
        auto constArray = new std::vector<int>();
        if (constInitVal->getTypeID() == MSyntaxNodeTypeID::CONST_EXP_CONST_INITVAL) {
            constArray->push_back(
                    self.expCalculator->calculateConstExp(
                            ((MConstExpConstInitVal *) constInitVal)->constExp,
                            self.globalTable, self.currentTable));
        } else if (constInitVal->getTypeID() == MSyntaxNodeTypeID::ARRAY_CONST_INITVAL) {
            for (auto c: *((MArrayConstInitVal *) constInitVal)->constInitVals) {
                auto constArray2 = self.constInitVal2ConstArray(c);
                constArray->insert(constArray->end(), constArray2->begin(), constArray2->end());
            }
        }
        return constArray;
    }

    std::vector<MExp *> *initVal2ExpArray(MInitVal *initVal) {
        if (initVal == nullptr) {
            return nullptr;
        }
        auto expArray = new std::vector<MExp *>();
        if (initVal->getTypeID() == MSyntaxNodeTypeID::EXP_INITVAL) {
            expArray->push_back(((MExpInitVal *) initVal)->exp);
        } else if (initVal->getTypeID() == MSyntaxNodeTypeID::ARRAY_INITVAL) {
            for (auto i: *((MArrayInitVal *) initVal)->initVals) {
                auto tempArray = self.initVal2ExpArray(i);
                expArray->insert(expArray->end(), tempArray->begin(), tempArray->end());
            }
        }
        return expArray;
    }

    void branchNotTrue(MCond *cond, std::string trueLabel, std::string falseLabel) {
        self.calculateCond(cond, trueLabel);
        self.addIR(new MBranchIRStatement(falseLabel));
        self.resetRegUsage();
    }

    void branch(std::string label) {
        self.addIR(new MBranchIRStatement(label));
    }
    // type一般为"if""while"之类的，用来生成易于调试的label

    std::string generateLabel(std::string labelType) {
        return self.labelManager->getLabel(labelType);
    }

    void setLabel(std::string label) {
        self.addIR(new MLabelIRStatement(label));
    }


    void returnBack() {
        self.addIR(new MReturnIRStatement());
    }

    void setReturnValue(MExp *exp) {
        auto irVar = self.calculateExp(exp);
        if (irVar != nullptr)
            self.addIR(new MSaveIRStatement(new MRegIRVar("return_value"), irVar));
        self.resetRegUsage();
    }

    void printf(MFormatString *formatString, std::vector<MExp *> *printfExps, int lineNum) {
        // TODO 还没检查参数类型匹配
        // 只需要检查个数是否正确，MFormatString保证了formatString的格式正确
        if (formatString->formatCharNum != printfExps->size()) {
            MExceptionManager::pushException(
                    new MPrintfParamNumNotMatchException(lineNum));
            // TODO 此处直接返回存在风险，可能printf的参数有未定义行为
            return;
        }

        int size = formatString->formatString.size();
        int expNum = printfExps->size() - 1;
        bool hasNoException = true;
        // 两个循环，先处理%d, 再处理其他，相当于先计算参数再调用函数
        std::vector<MIRVar*> tempIRs; // TODO 好像没有用？

        for (int i = size - 1; i >= 0; i--) {
            // 倒叙push，正序putvar
            char c = formatString->formatString[i];
            if (c == '%') {
                i++;
                auto irVar = self.calculateExp((*printfExps)[expNum--]);
                if (irVar == nullptr) hasNoException = false;
                else {
                    self.addIR(new MPushIRStatement(irVar));
                    tempIRs.push_back(irVar);
                }
                self.resetRegUsage();
                i--;
            }
        }
        if (!hasNoException) {
            self.resetRegUsage();
            return;
        }
        for (int i = 0; i < size; i++) {
            char c = formatString->formatString[i];
            if (c == '%') {
                i++;
                self.addIR(new MPutVarIRStatement());
            } else if (c == '\\') {
                i++;
                self.addIR(new MPutCharIRStatement('\n'));
            } else if (c != '"') {
                self.addIR(new MPutCharIRStatement(c));
            }
        }
        self.resetRegUsage();
    }

    void getint(MLVal *lVal) {
        auto lValIRVar = self.calculateLVal(lVal, true);
        if (lValIRVar == nullptr) {
            self.resetRegUsage();
            return;
        }
        self.addIR(new MGetintIRStatement(lValIRVar));
        self.resetRegUsage();
    }

    void assignExp2LVal(MLVal *lVal, MExp *exp) {
        auto irVar = self.calculateExp(exp);
        if (irVar == nullptr) {
            self.resetRegUsage();
            return;
        }
        // true代表了要将LVal作为赋值的对象
        auto lValIRVar = self.calculateLVal(lVal, true);
        if (lValIRVar == nullptr) return;
        self.addIR(new MSaveIRStatement(lValIRVar, irVar));
        self.resetRegUsage();
    }

    void handleExp(MExp *exp) {
        // 用来处理单独一个Exp的Stmt
        self.calculateExp(exp);
        self.resetRegUsage();
    }

    void addReturn() {
        self.addIR(new MReturnIRStatement());
    }

    void checkBlockReturn(MBlock* block, bool isVoid, int lastBlockLineNum) {
        // lastBlockLineNum是block的'}'的行号

        // 先检查普通的return错误
        _checkBlockReturn(block, isVoid);

        // 看看最后一项是不是return
        auto blockItems = block->blockItems;
        bool hasReturn = false;
        if (blockItems != nullptr && blockItems->size() > 0) {
            auto stmt = ((MStmtBlockItem*)(*(block->blockItems))[block->blockItems->size() - 1])->stmt;
            hasReturn = self.checkStmtReturn(stmt, isVoid);
        }
        if (!isVoid && !hasReturn)
            MExceptionManager::pushException(
                    new MMissingReturnStatementException(lastBlockLineNum));
    }
private:
    void _checkBlockReturn(MBlock* block, bool isVoid) {
        // 不检查有“有返回值的函数最后没有return语句（g号）”这个错误
        for (auto blockItem : (*(block->blockItems))) {
            if (blockItem->getTypeID() == STMT_BLOCK_ITEM) {
                self.checkStmtReturn(((MStmtBlockItem*)blockItem)->stmt, isVoid);
            }
        }
    }
    bool checkStmtReturn(MStmt* stmt, bool isVoid) {
        // 如果有return则返回true，不在乎应不应该有返回值
        if (stmt == nullptr) return false;
        if (stmt->getTypeID() == RETURN_STMT) {
            if (((MReturnStmt*)stmt)->exp == nullptr && !isVoid)
                // 需要有返回值却没有，理论上没有这种情况
                throw "MIRGenerator::checkStmtReturn: unhandled condition.";

            else if (((MReturnStmt*)stmt)->exp != nullptr && isVoid) {
                // 不能有返回值却有了
                MExceptionManager::pushException(
                        new MRedundantReturnStatementException(((MReturnStmt*)stmt)->lineNum));
            }
            return true;
        } else if (stmt->getTypeID() == IF_STMT) {
            self.checkStmtReturn(((MIfStmt*)stmt)->ifStmt, isVoid);
            self.checkStmtReturn(((MIfStmt*)stmt)->elseStmt, isVoid);
        } else if (stmt->getTypeID() == WHILE_STMT) {
            self.checkStmtReturn(((MWhileStmt*)stmt)->stmt, isVoid);
        } else if (stmt->getTypeID() == BLOCK_STMT) {
            self._checkBlockReturn(((MBlockStmt*)stmt)->block, isVoid);
        }
        return false;
    }


private:
    void addIR(MIRStatement *ir) {
        self.irStatements->push_back(ir);
    }

    void addIR2Front(MIRStatement *ir) {
        self.irStatements->insert(self.irStatements->begin(), ir);
    }

    void insertCallMain() {
        // 在中间代码的第一个label之前插入一个call main和一个exit
        int len = self.irStatements->size();
        int i;
        for (i = 0; i < len; i++) {
            if ((*(self.irStatements))[i]->type == MIRStatementType::LABEL) {
                break;
            }
        }
        self.irStatements->insert(self.irStatements->begin() + i, new MExitIRStatement());
        self.irStatements->insert(self.irStatements->begin() + i, new MCallFuncIRStatement("main"));
    }

    MVariableSymbolTableItem *getVarTableItem(std::string name) {
        // 使用这个函数之前一定要先检查undefined问题
        for (auto table = self.currentTable; table != nullptr; table = table->getFatherScopeTable()) {
            if (table->contains(name)) {
                auto item = table->getTableItem(name);
                if (item->isVariable) {
                    return (MVariableSymbolTableItem *) item;
                } else {
                    throw "MIRGenerator::getVarTableItem: is function name not variable";
                }
            }
        }
        throw "MIRGenerator::getVarTableItem: can't find name";
    }
    MFunctionSymbolTableItem *getFuncTableItem(std::string name) {
        // 使用这个函数之前一定要先检查undefined问题
        for (auto table = self.currentTable; table != nullptr; table = table->getFatherScopeTable()) {
            if (table->contains(name)) {
                auto item = table->getTableItem(name);
                if (!item->isVariable) {
                    return (MFunctionSymbolTableItem *) item;
                } else {
                    throw "MIRGenerator::getFuncTableItem: is var name not func";
                }
            }
        }
        throw "MIRGenerator::getFuncTableItem: can't find name";
    }

    void initConstVariable(std::string name) {
        int offset = 0;
        auto item = self.getVarTableItem(name);
        auto initArray = item->initArray;
        if (initArray == nullptr) {
            return;
        }
        // 分为一般变量与数组
        if (!item->variableType->isArray) {
            self.addIR(
                    new MSaveIRStatement(
                            new MVarIRVar(name),
                            new MImmIRVar((*(initArray))[offset])));
        } else {
            // 是数组
            auto dims = item->variableType->dims;
            int totalSize = 1;
            // 计算数组所占内存的大小
            // 全部转换为一维数组
            for (auto index: *dims) totalSize *= index;
            if (totalSize != initArray->size())
                throw "MIRGenerator::initContVariable: " + name + "'s dim and initArray's size ";
            for (int offset = 0; offset < totalSize; offset++)
                self.addIR(new MSaveIRStatement(
                        new MVarIRVar(name, new MImmIRVar(offset)),
                        new MImmIRVar((*(initArray))[offset])));
        }
    }

    void initVariable(std::string name) {
        auto item = self.getVarTableItem(name);
        auto expArray = item->initExpArray;
        if (expArray == nullptr) {
            return;
        }
        int offset = 0;
        if (!item->variableType->isArray) {
            if (expArray->size() > 1) {
                throw "MIRGenerator::initVariable: init contains more "
                      "than 1 exp when init a non-array variable";
            }
            auto result = self.calculateExp((*expArray)[offset]);
            if (result == nullptr) return;
            else self.addIR(
                    new MSaveIRStatement(
                            new MVarIRVar(name), result));
        } else {
            // 是数组
            auto dims = item->variableType->dims;
            int totalSize = 1;
            // 计算数组所占内存的大小
            // 全部转换为一维数组
            for (auto index: *dims) totalSize *= index;
            bool hasNoException = true;
            if (totalSize != expArray->size())
                throw "MIRGenerator::initVariable: " + name + "'s dim and initArray's size not match";
            for (int offset = 0; offset < totalSize; offset++) {
                auto result = self.calculateExp((*expArray)[offset]);
                if (result == nullptr) hasNoException = false;

                if (hasNoException)
                    self.addIR(new MSaveIRStatement(
                        new MVarIRVar(name, new MImmIRVar(offset)), result));
                self.resetRegUsage();
            }
        }
    }

    // 管理临时变量的序号
    int regNum = 0;

    MRegIRVar *newRegIRVar() {
        return new MRegIRVar(regNum++);
    }

    void resetRegUsage() {
        regNum = 0;
    }

    void calculateCond(MCond *cond, std::string trueLabel) {
        self.calculateLOrExp(cond->lOrExp, trueLabel);
    }

    void calculateLOrExp(MLOrExp *lOrExp, std::string trueLabel) {
        // label用来短路求值，当足以判断表达式值的时候，就跳到trueLabel
        // 不会有返回值
        for (auto lAndExp: *(lOrExp->lAndExps)) {
            // 告诉lAndExp，只有所有元素都是1才跳到trueLabel，否则跳到andExpFalseLabel
            // andExpFalseLabel会设置在lAndExp的最后
            // 当这一整个lAndExp结束了，就跳到andExpFalseLabel
            auto andExpFalseLabel = self.labelManager->getLabel("and_exp_false_label_for_short_circuit");
            self.calculateLAndExp(lAndExp, andExpFalseLabel, trueLabel);
            self.addIR(new MLabelIRStatement(andExpFalseLabel));
        }
    }

    void calculateLAndExp(MLAndExp *lAndExp, std::string falseLabel, std::string trueLabel) {
        // 如果有一个eqExp“不成立”，falseLabel
        bool hasNoException = true;

        for (auto eqExp: *(lAndExp->eqExps)) {
            auto result = self.calculateEqExp(eqExp);
            if (result == nullptr) hasNoException = false;
            else {
                self.addIR(new MBranchNotTrueIRStatement(result, falseLabel));
            }
        }
        self.addIR(new MBranchIRStatement(trueLabel));
        if (!hasNoException)
            return;
    }

    MIRVar *calculateEqExp(MEqExp *eqExp) {
        bool hasNoException = true;
        int relExpNum = eqExp->relExps->size();
        if (relExpNum < 2) {
            return calculateRelExp((*(eqExp->relExps))[0]);
        }
        auto relResults = new std::vector<MIRVar *>();
        for (auto relExp: *(eqExp->relExps)) {
            auto result = self.calculateRelExp(relExp);
            if (result == nullptr) hasNoException = false;
            else relResults->push_back(result);
        }
        if (!hasNoException)
            return nullptr;
        auto tempVarName = self.newRegIRVar();
        if ((*(eqExp->ops))[0] == Token::EQL)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::EQL, tempVarName,
                                           (*relResults)[0], (*relResults)[1]));
        else if ((*(eqExp->ops))[0] == Token::NEQ)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::NEQ, tempVarName,
                                           (*relResults)[0], (*relResults)[1]));

        for (int i = 2; i < relExpNum; i++) {
            if ((*(eqExp->ops))[i - 1] == Token::EQL)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::EQL, tempVarName,
                                               tempVarName, (*relResults)[i]));
            else if ((*(eqExp->ops))[i - 1] == Token::NEQ)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::NEQ, tempVarName,
                                               tempVarName, (*relResults)[i]));
        }
        return tempVarName;
    }

    MIRVar *calculateRelExp(MRelExp *relExp) {
        bool hasNoException = true;
        int addExpNum = relExp->addExps->size();
        if (addExpNum < 2) {
            return calculateAddExp((*(relExp->addExps))[0]);
        }
        auto addResults = new std::vector<MIRVar *>();
        for (auto addExp: *(relExp->addExps)) {
            auto result = self.calculateAddExp(addExp);
            if (result == nullptr) hasNoException = false;
            else addResults->push_back(result);
        }
        if (!hasNoException)
            return nullptr;

        auto tempVarName = self.newRegIRVar();
        if ((*(relExp->ops))[0] == Token::LSS)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::LSS, tempVarName,
                                           (*addResults)[0], (*addResults)[1]));
        else if ((*(relExp->ops))[0] == Token::LEQ)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::LEQ, tempVarName,
                                           (*addResults)[0], (*addResults)[1]));
        else if ((*(relExp->ops))[0] == Token::GRE)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::GRE, tempVarName,
                                           (*addResults)[0], (*addResults)[1]));
        else if ((*(relExp->ops))[0] == Token::GEQ)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::GEQ, tempVarName,
                                           (*addResults)[0], (*addResults)[1]));

        for (int i = 2; i < addExpNum; i++) {
            if ((*(relExp->ops))[i - 1] == Token::LSS)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::LSS, tempVarName,
                                               tempVarName, (*addResults)[i]));
            else if ((*(relExp->ops))[i - 1] == Token::LEQ)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::LEQ, tempVarName,
                                               tempVarName, (*addResults)[i]));
            else if ((*(relExp->ops))[i - 1] == Token::GRE)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::GRE, tempVarName,
                                               tempVarName, (*addResults)[i]));
            else if ((*(relExp->ops))[i - 1] == Token::GEQ)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::GEQ, tempVarName,
                                               tempVarName, (*addResults)[i]));
        }
        return tempVarName;
    }

    MIRVar *calculateExp(MExp *exp) {
        // 一定会返回一个regIRVar
        auto tempIRVar = self.calculateAddExp(exp->addExp);
//        if (tempIRVar->getTypeID() != MIRVarTypeID::REG_IR_VAR) {
//            auto regIRVar = self.newRegIRVar();
//            self.addIR(new MSaveIRStatement(regIRVar, tempIRVar));
//            return regIRVar;
//        }
        return tempIRVar;
    }

    MIRVar *calculateAddExp(MAddExp *addExp) {
        int hasNoException = true; // 标记是否产生了异常
        int mulExpNum = addExp->mulExps->size();
        if (mulExpNum < 2) {
            return calculateMulExp((*(addExp->mulExps))[0]);
        }
        auto mulResults = new std::vector<MIRVar*>();
        for (auto mulExp: *(addExp->mulExps)) {
            auto mulResult = self.calculateMulExp(mulExp);
            if (mulResult == nullptr) hasNoException = false;
            else mulResults->push_back(mulResult);
        }

        if (!hasNoException) {
            // 如果有异常，就不继续计算了
            return nullptr;
        }        auto tempVarName = self.newRegIRVar();
        if ((*(addExp->ops))[0] == Token::PLUS)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::ADD, tempVarName,
                                           (*mulResults)[0], (*mulResults)[1]));
        else if ((*(addExp->ops))[0] == Token::MINU)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::SUB, tempVarName,
                                           (*mulResults)[0], (*mulResults)[1]));

        for (int i = 2; i < mulExpNum; i++) {
            if ((*(addExp->ops))[i - 1] == Token::PLUS)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::ADD, tempVarName,
                                               tempVarName, (*mulResults)[i]));
            else if ((*(addExp->ops))[i - 1] == Token::MINU)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::SUB, tempVarName,
                                               tempVarName, (*mulResults)[i]));
        }
        return tempVarName;

    }

    MIRVar *calculateMulExp(MMulExp *mulExp) {
        int hasNoException = true; // 标记是否产生了异常
        int unaryExpNum = mulExp->unaryExps->size();
        if (unaryExpNum < 2) {
            return calculateUnaryExp((*(mulExp->unaryExps))[0]);
        }
        auto unaryResults = new std::vector<MIRVar*>();
        for (auto unaryExp: *(mulExp->unaryExps)) {
            auto result = self.calculateUnaryExp(unaryExp);
            if (result == nullptr) hasNoException = false;
            else unaryResults->push_back(result);
        }
        if (!hasNoException) {
            // 如果有异常，就不继续计算了
            return nullptr;
        }

        auto tempVarName = self.newRegIRVar();
        if ((*(mulExp->ops))[0] == Token::MULT)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::MULT, tempVarName,
                                           (*unaryResults)[0], (*unaryResults)[1]));
        else if ((*(mulExp->ops))[0] == Token::DIV)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::DIV, tempVarName,
                                           (*unaryResults)[0], (*unaryResults)[1]));
        else if ((*(mulExp->ops))[0] == Token::MOD)
            self.addIR(
                    new MBinaryIRStatement(MBinaryIRType::MOD, tempVarName,
                                           (*unaryResults)[0], (*unaryResults)[1]));

        for (int i = 2; i < unaryExpNum; i++) {
            if ((*(mulExp->ops))[i - 1] == Token::MULT)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::MULT, tempVarName,
                                               tempVarName, (*unaryResults)[i]));
            else if ((*(mulExp->ops))[i - 1] == Token::DIV)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::DIV, tempVarName,
                                               tempVarName, (*unaryResults)[i]));
            else if ((*(mulExp->ops))[i - 1] == Token::MOD)
                self.addIR(
                        new MBinaryIRStatement(MBinaryIRType::MOD, tempVarName,
                                               tempVarName, (*unaryResults)[i]));

        }
        return tempVarName;
    }

    MIRVar *calculateUnaryExp(MUnaryExp *unaryExp) {
        if (unaryExp->getTypeID() == MSyntaxNodeTypeID::FUNC_UNARY_EXP) {
            return self.callFuncWithReturn(((MFuncUnaryExp *) unaryExp)->ident->name,
                                           ((MFuncUnaryExp *) unaryExp)->funcRParams,
                                           ((MFuncUnaryExp *) unaryExp)->ident->lineNum);
        } else if (unaryExp->getTypeID() == MSyntaxNodeTypeID::PRIMARY_EXP_UNARY_EXP) {
            return self.calculatePrimaryExp(((MPrimaryExpUnaryExp *) unaryExp)->primaryExp);
        } else if (unaryExp->getTypeID() == MSyntaxNodeTypeID::UNARY_EXP_UNARY_EXP) {
            auto _unaryExp = (MUnaryExpUnaryExp *) unaryExp;
            auto result = self.calculateUnaryExp(_unaryExp->unaryExp);
            // 计算unaryExp的结果可能是一个立即数或者变量, 就需要开一个reg来保存结果
            // 否则会改变变量的值
            auto irVar = result->getTypeID() != MIRVarTypeID::REG_IR_VAR ? self.newRegIRVar() : result;
            switch (_unaryExp->unaryOp->op) {
                case Token::PLUS:
                    self.addIR(new MUnaryIRStatement(MUnaryIRType::POS, irVar, result));
                    break;
                case Token::MINU:
                    self.addIR(new MUnaryIRStatement(MUnaryIRType::NEG, irVar, result));
                    break;
                case Token::NOT:
                    self.addIR(new MUnaryIRStatement(MUnaryIRType::NOT, irVar, result));
                    break;
                default:
                    throw "IRGenerator::calculateUnaryExp: 一种新的MUnaryExpUnaryExp这里没有处理";
            }
            return irVar;
        } else {
            throw "IRGenerator::calculateUnaryExp: 一种新的UnaryExp这里没有处理";
        }
    }

    MIRVar *calculatePrimaryExp(MPrimaryExp *primaryExp) {
        if (primaryExp->getTypeID() == MSyntaxNodeTypeID::EXP_PRIMARY_EXP) {
            return self.calculateExp(((MExpPrimaryExp *) primaryExp)->exp);
        } else if (primaryExp->getTypeID() == MSyntaxNodeTypeID::LVAL_PRIMARY_EXP) {
            return self.calculateLVal(((MLValPrimaryExp *) primaryExp)->lVal, false);
        } else if (primaryExp->getTypeID() == MSyntaxNodeTypeID::NUMBER_PRIMARY_EXP) {
            return new MImmIRVar(std::stoi(((MNumberPrimaryExp *) primaryExp)->number->intConst->intConst));
        } else {
            throw "IRGenerator::calculatePrimaryExp: 一种新的PrimaryExp没有处理";
        }
    }

    MIRVar *callFuncWithReturn(const std::string &funcName, MFuncRParams *funcParams, int lineNum) {
        // 保存当前使用了多少临时寄存器，进行函数调用前后的保存
        // 必须记录一份临时的，因为在记录参数的时候会使用其他的临时寄存器
        // 而参数使用的临时寄存器无需保存

        if (self.isUndefined(funcName)) {
            MExceptionManager::pushException(new MUndefinedIdentifierException(lineNum));
            return nullptr;
        }
        auto item = self.getFuncTableItem(funcName);
        int tempVarNum = self.regNum;
        for (int i = 0; i < tempVarNum; i++) {
            self.addIR(new MPushIRStatement(
                    self.number2IRTempVar(i)));
        }

        bool hasNoException = true;
        if (funcParams != nullptr)
            for (auto exp: *(funcParams->exps)) {
                // 从左到右把参数压栈
                auto irVar = self.calculateExp(exp);
                if (irVar == nullptr) {
                    // 说明计算失败，出了异常。标志一下，不再进行接下来的参数类型检查
                    hasNoException = false;
                }
                self.addIR(new MPushIRStatement(irVar));
            }

        if (!hasNoException)
            return nullptr;

        self.addIR(new MCallFuncIRStatement(funcName));
        // 恢复临时寄存器
        for (int i = tempVarNum - 1; i >= 0; i--) {
            self.addIR(new MPopIRStatement(
                    self.number2IRTempVar(i)));
        }
        auto regIRVar = self.newRegIRVar();
        self.addIR(new MSaveIRStatement(regIRVar, new MRegIRVar("return_value")));
        // 必须要先压栈再检查是否匹配，因为可能存在参数里有未定义变量这样的异常
        self.checkParamMatchFunc(funcName, funcParams, lineNum);
        return regIRVar;
    }

    MIRVar *calculateLVal(MLVal *lVal, bool isAssign) {
        if (isUndefined(lVal->ident->name)) {
            MExceptionManager::pushException(
                    new MUndefinedIdentifierException(
                            lVal->ident->lineNum));
            return nullptr;
        } else if (isAssign && isConst(lVal->ident->name)) {
            MExceptionManager::pushException(
                    new MChangingConstException(lVal->ident->lineNum));
            return nullptr;
        }

        // 计算offset
        int num = lVal->exps->size();
        if (num == 0) {
            // 不是数组
            return new MVarIRVar(lVal->ident->name);
        } else {
            // 处理左值为数组的情况
            // 将多维数组转为一维数组的偏移量
            auto dims = self.getVarTableItem(lVal->ident->name)->variableType->dims;
            bool isParam = self.getVarTableItem(lVal->ident->name)->variableType->isParam;

            if (dims == nullptr) dims = new std::vector<int>(); // 一维参数数组可能dims为nullptr
            if (isParam)
                // 如果是参数数组，则dim的维度数一定要小一维，所以要插一个-1
                dims->insert(dims->begin(), -1);
            auto offsetRegIRVar = self.newRegIRVar();
            auto tempRegIRVar = self.newRegIRVar(); // 保存中间结果
            if (dims->size() == lVal->exps->size()) {
                // 说明不是使用参数数组
                // 先把最后一维的偏移加上，也是给offset赋初值
                self.addIR(new MSaveIRStatement(offsetRegIRVar, self.calculateExp((*(lVal->exps))[num - 1])));
                for (int i = num - 2; i >= 0; i--) {
                    int size = 1;
                    for (int j = i + 1; j < num; j++) {
                        size *= (*(dims))[j];
                    }
                    // offset += (*(arrayIndexes))[i] * size;
                    self.addIR(new MBinaryIRStatement(MBinaryIRType::MULT, tempRegIRVar, self.calculateExp((*(lVal->exps))[i]), new MImmIRVar(size)));
                    self.addIR(new MBinaryIRStatement(MBinaryIRType::ADD, offsetRegIRVar, offsetRegIRVar, tempRegIRVar));
                }
                if (isParam)
                    // 如果是参数数组，则dim的维度数一定要小一维，所以要插一个-1. 这里把插上去的-1删除
                    dims->erase(dims->begin());
                return new MVarIRVar(lVal->ident->name, offsetRegIRVar);
            } else {
                // 处理参数数组
                // TODO 只考虑二维数组
                // 对付的是int a[3][3]; func(a[0])这种情况
                if (dims->size() != 2 || num != 1)
                    throw "MIRGenerator::calculateLVal: unconsidered condition";

                // int a[3][3]; a[2]-->save %0, a    mult %1, 2, 3    add %0, %0, %1
                self.addIR(new MSaveIRStatement(tempRegIRVar, new MVarIRVar(lVal->ident->name)));
                self.addIR(new MBinaryIRStatement(MBinaryIRType::MULT,
                                                  offsetRegIRVar,
                                                  self.calculateExp((*(lVal->exps))[0]),
                                                  new MImmIRVar((*(dims))[1])));
                self.addIR(new MBinaryIRStatement(MBinaryIRType::ADD, tempRegIRVar, tempRegIRVar, offsetRegIRVar));
                if (isParam)
                    // 如果是参数数组，则dim的维度数一定要小一维，所以要插一个-1. 这里把插上去的-1删除
                    dims->erase(dims->begin());
                return tempRegIRVar;
            }
        }
    }



//    std::string number2IRTempVar(std::string &number) {
//        return "%" + number;
//    }
//
    MRegIRVar* number2IRTempVar(int num) {
        return new MRegIRVar(num);
    }


    bool checkParamMatchFunc(std::string funcName, MFuncRParams *funcRParams, int lineNum) {
        // 保证已经检查未定义、重定义问题了
        auto item = self.getFuncTableItem(funcName);

        int paramNum = funcRParams == nullptr ? 0 : funcRParams->exps->size();
        if (item->paramNum != paramNum) {
            MExceptionManager::pushException(new MParamNumNotMatchException(lineNum));
            return false;
        }
        for (int i = 0; i < item->paramNum; i++) {
            auto expType = self.getExpType((*(funcRParams->exps))[i]);
            if (!self.checkTypeMatchParamType(expType, (*(item->paramTypes))[i])) {
                MExceptionManager::pushException(new MParamTypeNotMatchException(lineNum));
                return false;
            }
        }
        return true;
    }

    MBaseType* getExpType(MExp* exp) {
        auto mulType = self.getMulExpType((*(exp->addExp->mulExps))[0]);
        for (int i = 1; i < exp->addExp->mulExps->size(); i++) {
            // 检查这些mulExp的类型都是一样的
            auto nextMulType = self.getMulExpType((*(exp->addExp->mulExps))[i]);
            if (!mulType->equals(nextMulType)) {
                throw "MIRGenerator::getExpType: type of mulExp not equal";
            }
        }
        return mulType;
    }
    MBaseType* getMulExpType(MMulExp* mulExp) {
        auto unaryType = self.getUnaryExpType((*(mulExp->unaryExps))[0]);
        for (int i = 1; i < mulExp->unaryExps->size(); i++) {
            auto nextUnaryType = self.getUnaryExpType((*(mulExp->unaryExps))[i]);
            if (!unaryType->equals(nextUnaryType)) {
                throw "MIRGenerator::getUnaryExpType: type of unaryExp not equal";
            }
        }
        return unaryType;
    }

    MBaseType* getUnaryExpType(MUnaryExp* unaryExp) {
        if (unaryExp->getTypeID() == PRIMARY_EXP_UNARY_EXP) {
            return self.getPrimaryExpType(((MPrimaryExpUnaryExp*)unaryExp)->primaryExp);
        } else if (unaryExp->getTypeID() == FUNC_UNARY_EXP) {
            return self.getFuncType(((MFuncUnaryExp*)unaryExp)->ident->name);
        } else if (unaryExp->getTypeID() == UNARY_EXP_UNARY_EXP) {
            return self.getUnaryExpType(((MUnaryExpUnaryExp*)unaryExp)->unaryExp);
        } else {
            throw "MIRGenerator::getUnaryExpType: unhandled unaryExp type";
        }
    }

    MBaseType* getPrimaryExpType(MPrimaryExp* primaryExp) {
        if (primaryExp->getTypeID() == EXP_PRIMARY_EXP) {
            return self.getExpType(((MExpPrimaryExp*)primaryExp)->exp);
        } else if (primaryExp->getTypeID() == LVAL_PRIMARY_EXP) {
            return self.getLValType(((MLValPrimaryExp*)primaryExp)->lVal);
        } else if (primaryExp->getTypeID() == NUMBER_PRIMARY_EXP) {
            return new MBaseType(true, new MBType(), nullptr);
        } else {
            throw "MIRGenerator::getPrimaryExpType: unhandled primaryExp type";
        }
    }
    MBaseType* getLValType(MLVal* lVal) {
        auto varType = self.getVarTableItem(lVal->ident->name)->variableType;
        if (!varType->isArray)
            return new MBaseType(varType->isConst, new MBType(), nullptr, varType->isParam);
        auto dimNum = lVal->exps->size(); // 引用到几维的数组
        // 将varType“减掉”dimNum个维度之后返回
        auto originDim = varType->dims;
        auto tempDim = new std::vector<int>();
        if (varType->isParam) {
            tempDim->insert(tempDim->begin(), -1);
        }
        for (int d : *originDim) {
            tempDim->push_back(d);
        }
        for (int i = 0; i < dimNum; i++) {
            tempDim->erase(tempDim->begin());
        }
        // 经过消减之后，可能不再是一个数组
        return new MBaseType(varType->isConst, new MBType(),
                             tempDim, varType->isParam && tempDim->empty(), tempDim->size() > 0);
    }
    MBaseType* getFuncType(std::string funcName) {
        return self.getFuncTableItem(funcName)->returnType;
    }

    bool checkTypeMatchParamType(MBaseType* type, MParamType* paramType) {
        if (type->type->type != paramType->type->type)
            // int 和void不相等
            return false;
        if (!paramType->isArray)
            // 判断普通变量
            return !type->isArray;
        else if (!type->isArray)
            return false; // param是数组而type不是数组
        // 两个都是数组的情况
        if (type->isParam) {
            if (paramType->dims->size() != type->dims->size())
                return false;
            for (int i = 0; i < paramType->dims->size(); i++)
                if ((*(paramType->dims))[i] != (*(type->dims))[i])
                    return false;
        } else {
            if (paramType->dims->size() != type->dims->size() - 1)
                return false;
            for (int i = 0; i < paramType->dims->size(); i++)
                if ((*(paramType->dims))[i] != (*(type->dims))[i + 1])
                    return false;
        }
        return true;
    }

};

#endif //BUAA_COMPILER_IRGENERATOR_H
