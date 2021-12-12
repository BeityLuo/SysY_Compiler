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
    IRGenerator(MSymbolTable *globalTable)
            : globalTable(globalTable), currentTable(globalTable),
              irStatements(new std::vector<MIRStatement *>()),
              labelManager(new MLabelManager()),
              expCalculator(new MExpCalculator()) {}

    std::string toString() {
        std::string ans = "";
        for (auto irStatement: *(self.irStatements)) {
            ans += irStatement->toString() + "\n";
        }
        return ans;
    }

    std::vector<MIRStatement*>* getIRStatements() {
        return self.irStatements;
    }

    bool isRedefined(std::string &name) {
        return self.currentTable->contains(name);
    }

    bool isUndefined(std::string &name) {
        for (auto table = self.currentTable; table != nullptr;
             table = table->getFatherScopeTable()) {

            if (table->contains(name))
                return false;
        }
        return true;
    }

    bool isConst(std::string &name) {
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
            return;
        }
        auto varType = new MBaseType(isConst, type, dims);
        // 生成declare中间代码和初始化代码
        self.addIR(
                new MDeclIRStatement(varType, name));
        auto initExpArray = self.initVal2ExpArray((MInitVal*)init);
        if (isConst) {
            auto initArray = self.constInitVal2ConstArray((MConstInitVal *) init);
            self.currentTable->addSymbolItem(
                    new MVariableSymbolTableItem(name, varType, initExpArray, initArray));
            self.initConstVariable(name);
        } else {

            self.currentTable->addSymbolItem(
                    new MVariableSymbolTableItem(name, varType, initExpArray));
            self.initVariable(name);
            self.resetRegUsage();
        }
    }


    void addFunc2Table(MType *type, std::string name, MFuncFParams *funcFParams, int lineNum) {
        if (name == "main") {
            self.insertCallMain();
        }
        int scopeLevel = self.currentTable->getScopeLevel();
        if (self.isRedefined(name)) {
            MExceptionManager::pushException(
                    new MRedefinedIdentifierException(lineNum));
            return;
        }
        int paramNum;
        auto paramTypes = new std::vector<MParamType *>();
        auto paramNames = new std::vector<std::string>();
        if (funcFParams != nullptr) {
            paramNum = funcFParams->funcFParams->size();
            for (auto para: *(funcFParams->funcFParams)) {
                paramTypes->push_back(new MParamType(para->bType, para->dims, para->isArray));
                paramNames->push_back(para->ident->name);
            }
        } else {
            paramNum = 0;
        }
        auto funcType = new MReturnType(type);
        self.currentTable->addSymbolItem(
                new MFunctionSymbolTableItem(funcType, name, paramNum, paramTypes, paramNames));

        // 生成def中间代码和参数初始化代码
        // 把def放到最前面去，这样就可以从第一条ir开始顺序执行
        self.addIR2Front(new MFuncDefIRStatement(funcType, name, paramNum, paramTypes, paramNames));
        self.addIR(new MLabelIRStatement(
                self.labelManager->getFuncLabel(name)));


    }

    void initParams(std::string &funcName) {
        if (!self.globalTable->contains(funcName)) {
            throw "IRGenerator::initParams: funcName not exists";
        }
        auto symbolItem = (MFunctionSymbolTableItem *) self.globalTable
                ->getTableItem(funcName);


        for (int i = symbolItem->paramNum - 1; i >= 0; i--) {
            // 必须倒序
            auto paramName = (*(symbolItem->paramNames))[i];
            auto paramType = (*(symbolItem->paramTypes))[i];
            self.addIR(new MDeclIRStatement(paramType, paramName));
            self.addIR(new MPopIRStatement(new MVarIRVar(paramName, nullptr)));
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

    std::vector<MExp*> *initVal2ExpArray(MInitVal* initVal) {
        if (initVal == nullptr) {
            return nullptr;
        }
        auto expArray = new std::vector<MExp*>();
        if (initVal->getTypeID() == MSyntaxNodeTypeID::EXP_INITVAL) {
            expArray->push_back(((MExpInitVal*)initVal)->exp);
        } else if (initVal->getTypeID() == MSyntaxNodeTypeID::ARRAY_INITVAL) {
            for (auto i: *((MArrayInitVal *) initVal)->initVals) {
                auto tempArray = self.initVal2ExpArray(i);
                expArray->insert(expArray->end(), tempArray->begin(), tempArray->end());
            }
        }
        return expArray;
    }

    void branchNotTrue(MCond *cond, std::string label) {
        auto irVar = self.calculateCond(cond);
        self.addIR(new MBranchNotTrueIRStatement(irVar, label));
        self.resetRegUsage();
        return;
    }

    void branch(std::string label) {
        self.addIR(new MBranchIRStatement(label));
        return;
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
        self.addIR(new MSaveIRStatement("%return_value", irVar));
        self.resetRegUsage();
    }

    void printf(MFormatString *formatString, std::vector<MExp *> *printfExps, int lineNum) {
        // TODO 还没检查参数类型匹配
        // 只需要检查个数是否正确，MFormatString保证了formatString的格式正确
        if (formatString->formatCharNum != printfExps->size()) {
            MExceptionManager::pushException(
                    new MPrintfParamNumNotMatchException(lineNum));
        }

        int size = formatString->formatString.size();
        int expNum = printfExps->size() - 1;
        // 两个循环，先处理%d, 再处理其他，相当于先计算参数再调用函数
        std::vector<std::string> tempIRs;

        for (int i = size - 1; i >= 0; i--) {
            // 倒叙push，正序putvar
            char c = formatString->formatString[i];
            if (c == '%') {
                i++;
                auto irVar = self.calculateExp((*printfExps)[expNum--]);
                self.addIR(new MPushIRStatement(irVar));
                tempIRs.push_back(irVar);
                self.resetRegUsage();
                i--;
            }
        }
        for (int i = 0, tempIRIndex = 0; i < size; i++) {
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
        if (lValIRVar == nullptr) return;
        self.addIR(new MGetintIRStatement(lValIRVar));
        self.resetRegUsage();
    }

    void assignExp2LVal(MLVal *lVal, MExp *exp) {
        auto irVar = self.calculateExp(exp);
        if (irVar == nullptr) return;
        // true代表了要将LVal作为赋值的对象
        auto lValIRVar = self.calculateLVal(lVal, true);
        if (lValIRVar == nullptr) return;
        self.addIR(
                new MSaveIRStatement(lValIRVar, irVar));
        self.resetRegUsage();
    }

    void handleExp(MExp* exp) {
        // 用来处理单独一个Exp的Stmt
        self.calculateExp(exp);
        self.resetRegUsage();
    }

    void addReturn() {
        self.addIR(new MReturnIRStatement());
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
            if ((*(self.irStatements))[i]->type == IRType::LABEL) {
                break;
            }
        }
        self.irStatements->insert(self.irStatements->begin() + i, new MExitIRStatement());
        self.irStatements->insert(self.irStatements->begin() + i, new MCallFuncIRStatement("main"));
    }

    MVariableSymbolTableItem *getVarTableItem(std::string name) {
        auto table = self.currentTable;
        while (table != nullptr) {
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
                            new MVarIRVar(name, nullptr),
                            std::to_string((*(initArray))[offset])));
        } else {
            // 是数组
            auto dims = item->variableType->dims;
            int totalSize = 1;
            // 计算数组所占内存的大小
            for (auto index : *dims) totalSize *= index;
            if (totalSize != initArray->size())
                throw "MIRGenerator::initContVariable: " + name + "'s dim and initArray's size not match";
            for (int offset = 0; offset < totalSize; offset++)
                self.addIR(new MSaveIRStatement(
                        new MVarIRVar(name, new MImmIRVar(offset)),
                        new MImmIRVar((*(initArray))[offset])));

//            if (dims->size() == 1) {
//                if ((*dims)[0] != initArray->size()) {
//                    throw "MIRGenerator::initConstVariable: size not match";
//                }
//                for (int i = 0; i < (*dims)[0]; i++) {
//                    std::string element = name + "[" + std::to_string(i) + "]";
//                    self.addIR(
//                            new MSaveIRStatement(
//                                    element,
//                                    std::to_string((*initArray)[offset++])));
//                }
//            } else if (dims->size() == 2) {
//                if ((*dims)[0] * (*dims)[1] != initArray->size()) {
//                    throw "MIRGenerator::initConstVariable: size not match";
//                }
//                for (int i = 0; i < (*dims)[0]; i++) {
//                    for (int j = 0; j < (*dims)[1]; j++) {
//                        std::string element = name + "[" + std::to_string(i) + "]["
//                                              + std::to_string(j) + "]";
//                        self.addIR(
//                                new MSaveIRStatement(
//                                        element,
//                                        std::to_string((*initArray)[offset++])));
//                    }
//                }
//            } else {
//                throw "MIRGenerator::initConstVariable: array's dim > 2";
//            }
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
            self.addIR(
                    new MSaveIRStatement(
                            name,
                            self.calculateExp((*expArray)[offset])));
        } else {
            auto dims = item->variableType->dims;
            if (dims->size() == 1) {
                if ((*dims)[0] != expArray->size()) {
                    throw "MIRGenerator::initVariable: size not match";
                }
                for (int i = 0; i < (*dims)[0]; i++) {
                    auto tempIR = self.calculateExp((*expArray)[offset++]);
                    std::string element = name + "[" + std::to_string(i) + "]";
                    self.addIR(
                            new MSaveIRStatement(
                                    element, tempIR));
                }
            } else if (dims->size() == 2) {
                if ((*dims)[0] * (*dims)[1] != expArray->size()) {
                    throw "MIRGenerator::initVariable: size not match";
                }
                for (int i = 0; i < (*dims)[0]; i++) {
                    for (int j = 0; j < (*dims)[1]; j++) {
                        auto tempIR = self.calculateExp((*expArray)[offset++]);
                        std::string element = name + "[" + std::to_string(i) + "]["
                                              + std::to_string(j) + "]";
                        self.addIR(
                                new MSaveIRStatement(
                                        element,
                                        tempIR));
                    }
                }
            } else {
                throw "MIRGenerator::initVariable: array's dim > 2";
            }
        }
    }

    // 管理临时变量的序号
    int tempVarNum = 0;

    std::string newTempVarName() {
        tempVarNum++;
        return "%" + std::to_string(tempVarNum - 1);
    }

    void resetRegUsage() {
        tempVarNum = 0;
    }

    std::string calculateCond(MCond* cond) {
        return self.calculateLOrExp(cond->lOrExp);
    }

    std::string calculateLOrExp(MLOrExp* lOrExp) {
        int lAndExpNum = lOrExp->lAndExps->size();
        if (lAndExpNum < 2) {
            return calculateLAndExp((*(lOrExp->lAndExps))[0]);
        }
        auto lAndResults = new std::vector<std::string>();
        for (auto lAndExp: *(lOrExp->lAndExps)) {
            lAndResults->push_back(self.calculateLAndExp(lAndExp));
        }
        auto tempVarName = self.newTempVarName();
        self.addIR(
                new MBinaryIRStatement("or", tempVarName,
                                       (*lAndResults)[0], (*lAndResults)[1]));

        for (int i = 2; i < lAndExpNum; i++) {
            self.addIR(
                    new MBinaryIRStatement("or", tempVarName,
                                           tempVarName, (*lAndResults)[i]));
        }
        return tempVarName;
    }

    std::string calculateLAndExp(MLAndExp* lAndExp) {
        int eqExpNum = lAndExp->eqExps->size();
        if (eqExpNum < 2) {
            return calculateEqExp((*(lAndExp->eqExps))[0]);
        }
        auto eqResults = new std::vector<std::string>();
        for (auto eqExp: *(lAndExp->eqExps)) {
            eqResults->push_back(self.calculateEqExp(eqExp));
        }
        auto tempVarName = self.newTempVarName();
        self.addIR(
                new MBinaryIRStatement("and", tempVarName,
                                       (*eqResults)[0], (*eqResults)[1]));

        for (int i = 2; i < eqExpNum; i++) {
            self.addIR(
                    new MBinaryIRStatement("and", tempVarName,
                                           tempVarName, (*eqResults)[i]));
        }
        return tempVarName;
    }

    std::string calculateEqExp(MEqExp* eqExp) {
        int relExpNum = eqExp->relExps->size();
        if (relExpNum < 2) {
            return calculateRelExp((*(eqExp->relExps))[0]);
        }
        auto relResults = new std::vector<std::string>();
        for (auto relExp: *(eqExp->relExps)) {
            relResults->push_back(self.calculateRelExp(relExp));
        }
        auto tempVarName = self.newTempVarName();
        if ((*(eqExp->ops))[0] == Token::EQL)
            self.addIR(
                    new MBinaryIRStatement("eql", tempVarName,
                                           (*relResults)[0], (*relResults)[1]));
        else if ((*(eqExp->ops))[0] == Token::NEQ)
            self.addIR(
                    new MBinaryIRStatement("neq", tempVarName,
                                           (*relResults)[0], (*relResults)[1]));

        for (int i = 2; i < relExpNum; i++) {
            if ((*(eqExp->ops))[i - 1] == Token::EQL)
                self.addIR(
                        new MBinaryIRStatement("eql", tempVarName,
                                               tempVarName, (*relResults)[i]));
            else if ((*(eqExp->ops))[i - 1] == Token::NEQ)
                self.addIR(
                        new MBinaryIRStatement("neq", tempVarName,
                                               tempVarName, (*relResults)[i]));
        }
        return tempVarName;
    }

    std::string calculateRelExp(MRelExp* relExp) {
        int addExpNum = relExp->addExps->size();
        if (addExpNum < 2) {
            return calculateAddExp((*(relExp->addExps))[0]);
        }
        auto addResults = new std::vector<std::string>();
        for (auto addExp: *(relExp->addExps)) {
            addResults->push_back(self.calculateAddExp(addExp));
        }
        auto tempVarName = self.newTempVarName();
        if ((*(relExp->ops))[0] == Token::LSS)
            self.addIR(
                    new MBinaryIRStatement("lss", tempVarName,
                                           (*addResults)[0], (*addResults)[1]));
        else if ((*(relExp->ops))[0] == Token::LEQ)
            self.addIR(
                    new MBinaryIRStatement("leq", tempVarName,
                                           (*addResults)[0], (*addResults)[1]));
        else if ((*(relExp->ops))[0] == Token::GRE)
            self.addIR(
                    new MBinaryIRStatement("gre", tempVarName,
                                           (*addResults)[0], (*addResults)[1]));
        else if ((*(relExp->ops))[0] == Token::GEQ)
            self.addIR(
                    new MBinaryIRStatement("geq", tempVarName,
                                           (*addResults)[0], (*addResults)[1]));

        for (int i = 2; i < addExpNum; i++) {
            if ((*(relExp->ops))[i - 1] == Token::LSS)
                self.addIR(
                        new MBinaryIRStatement("lss", tempVarName,
                                               tempVarName, (*addResults)[i]));
            else if ((*(relExp->ops))[i - 1] == Token::LEQ)
                self.addIR(
                        new MBinaryIRStatement("leq", tempVarName,
                                               tempVarName, (*addResults)[i]));
            else if ((*(relExp->ops))[i - 1] == Token::GRE)
                self.addIR(
                        new MBinaryIRStatement("gre", tempVarName,
                                               tempVarName, (*addResults)[i]));
            else if ((*(relExp->ops))[i - 1] == Token::GEQ)
                self.addIR(
                        new MBinaryIRStatement("geq", tempVarName,
                                               tempVarName, (*addResults)[i]));
        }
        return tempVarName;
    }

    std::string calculateExp(MExp *exp) {
        return self.calculateAddExp(exp->addExp);
    }

    std::string calculateAddExp(MAddExp *addExp) {
        int mulExpNum = addExp->mulExps->size();
        if (mulExpNum < 2) {
            return calculateMulExp((*(addExp->mulExps))[0]);
        }
        auto mulResults = new std::vector<std::string>();
        for (auto mulExp: *(addExp->mulExps)) {
            mulResults->push_back(self.calculateMulExp(mulExp));
        }
        auto tempVarName = self.newTempVarName();
        if ((*(addExp->ops))[0] == Token::PLUS)
            self.addIR(
                    new MBinaryIRStatement("add", tempVarName,
                                           (*mulResults)[0], (*mulResults)[1]));
        else if ((*(addExp->ops))[0] == Token::MINU)
            self.addIR(
                    new MBinaryIRStatement("sub", tempVarName,
                                           (*mulResults)[0], (*mulResults)[1]));

        for (int i = 2; i < mulExpNum; i++) {
            if ((*(addExp->ops))[i - 1] == Token::PLUS)
                self.addIR(
                        new MBinaryIRStatement("add", tempVarName,
                                               tempVarName, (*mulResults)[i]));
            else if ((*(addExp->ops))[i - 1] == Token::MINU)
                self.addIR(
                        new MBinaryIRStatement("sub", tempVarName,
                                               tempVarName, (*mulResults)[i]));
        }
        return tempVarName;
    }

    std::string calculateMulExp(MMulExp *mulExp) {
        int unaryExpNum = mulExp->unaryExps->size();
        if (unaryExpNum < 2) {
            return calculateUnaryExp((*(mulExp->unaryExps))[0]);
        }
        auto unaryResults = new std::vector<std::string>();
        for (auto unaryExp: *(mulExp->unaryExps)) {
            unaryResults->push_back(self.calculateUnaryExp(unaryExp));
        }
        auto tempVarName = self.newTempVarName();
        if ((*(mulExp->ops))[0] == Token::MULT)
            self.addIR(
                    new MBinaryIRStatement("mult", tempVarName,
                                           (*unaryResults)[0], (*unaryResults)[1]));
        else if ((*(mulExp->ops))[0] == Token::DIV)
            self.addIR(
                    new MBinaryIRStatement("div", tempVarName,
                                           (*unaryResults)[0], (*unaryResults)[1]));
        else if ((*(mulExp->ops))[0] == Token::MOD)
            self.addIR(
                    new MBinaryIRStatement("mod", tempVarName,
                                           (*unaryResults)[0], (*unaryResults)[1]));

        for (int i = 2; i < unaryExpNum; i++) {
            if ((*(mulExp->ops))[i - 1] == Token::MULT)
                self.addIR(
                        new MBinaryIRStatement("mult", tempVarName,
                                               tempVarName, (*unaryResults)[i]));
            else if ((*(mulExp->ops))[i - 1] == Token::DIV)
                self.addIR(
                        new MBinaryIRStatement("div", tempVarName,
                                               tempVarName, (*unaryResults)[i]));
            else if ((*(mulExp->ops))[i - 1] == Token::MOD)
                self.addIR(
                        new MBinaryIRStatement("mod", tempVarName,
                                               tempVarName, (*unaryResults)[i]));

        }
        return tempVarName;
    }

    std::string calculateUnaryExp(MUnaryExp *unaryExp) {
        if (unaryExp->getTypeID() == MSyntaxNodeTypeID::FUNC_UNARY_EXP) {
            return self.callFuncWithReturn(((MFuncUnaryExp *) unaryExp)->ident->name,
                                 ((MFuncUnaryExp *) unaryExp)->funcRParams);
        } else if (unaryExp->getTypeID() == MSyntaxNodeTypeID::PRIMARY_EXP_UNARY_EXP) {
            return self.calculatePrimaryExp(((MPrimaryExpUnaryExp *) unaryExp)->primaryExp);
        } else if (unaryExp->getTypeID() == MSyntaxNodeTypeID::UNARY_EXP_UNARY_EXP) {
            auto _unaryExp = (MUnaryExpUnaryExp *) unaryExp;
            auto result = self.calculateUnaryExp(_unaryExp->unaryExp);
            // 计算unaryExp的结果可能是一个立即数
            std::string tempVar = self.isIRTempVar(result) ? result : self.newTempVarName();


            switch (_unaryExp->unaryOp->op) {
                case Token::PLUS:
                    self.addIR(new MUnaryIRStatement("pos", tempVar, result));
                    break;
                case Token::MINU:
                    self.addIR(new MUnaryIRStatement("neg", tempVar, result));
                    break;
                case Token::NOT:
                    self.addIR(new MUnaryIRStatement("not", tempVar, result));
                    break;
                default:
                    throw "IRGenerator::calculateUnaryExp: 一种新的MUnaryExpUnaryExp这里没有处理";
            }
            return tempVar;
        } else {
            throw "IRGenerator::calculateUnaryExp: 一种新的UnaryExp这里没有处理";
        }
    }

    std::string calculatePrimaryExp(MPrimaryExp *primaryExp) {
        if (primaryExp->getTypeID() == MSyntaxNodeTypeID::EXP_PRIMARY_EXP) {
            return self.calculateExp(((MExpPrimaryExp *) primaryExp)->exp);
        } else if (primaryExp->getTypeID() == MSyntaxNodeTypeID::LVAL_PRIMARY_EXP) {
            return self.calculateLVal(((MLValPrimaryExp *) primaryExp)->lVal, false);
        } else if (primaryExp->getTypeID() == MSyntaxNodeTypeID::NUMBER_PRIMARY_EXP) {
            return ((MNumberPrimaryExp *) primaryExp)->number->intConst->intConst;
        } else {
            throw "IRGenerator::calculatePrimaryExp: 一种新的PrimaryExp没有处理";
        }
    }

    std::string callFuncWithReturn(const std::string &funcName, MFuncRParams *funcParams) {
        if (!self.paramMatchFunc(funcName, funcParams)) {
            return "";
        }
        // 保存当前使用了多少临时寄存器，进行函数调用前后的保存
        // 必须记录一份临时的，因为在记录参数的时候会使用其他的临时寄存器
        // 而参数使用的临时寄存器无需保存
        int tempVarNum = self.tempVarNum;
        for (int i = 0; i < tempVarNum; i++) {
            self.addIR(new MPushIRStatement(
                    self.number2IRTempVar(i)));
        }

        if (funcParams != nullptr)
            for (auto exp: *(funcParams->exps)) {
                // 从左到右压栈
                self.addIR(new MPushIRStatement(self.calculateExp(exp)));
            }
        self.addIR(new MCallFuncIRStatement(funcName));
        for (int i = tempVarNum - 1; i >= 0; i--) {
            self.addIR(new MPopIRStatement(
                    self.number2IRTempVar(i)));
        }
        auto tempIRVar = self.newTempVarName();
        self.addIR(new MSaveIRStatement(tempIRVar, "%return_value"));
        return tempIRVar;
    }

    std::string calculateLVal(MLVal *lVal, bool isAssign) {
        if (isUndefined(lVal->ident->name)) {
            MExceptionManager::pushException(
                    new MUndefinedIdentifierException(
                            lVal->ident->lineNum));
            return "";
        } else if (isAssign && isConst(lVal->ident->name)) {
            MExceptionManager::pushException(
                    new MChangingConstException(lVal->ident->lineNum));
            return "";
        }
        auto ans = lVal->ident->name;
        auto dims = new std::vector<std::string>();
        if (lVal->exps != nullptr)
            for (auto exp: *(lVal->exps)) {
                auto dim = self.calculateExp(exp);
                ans += "[" + dim + "]";
            }
        return ans;
    }

    bool isIRTempVar(std::string &name) {
        return name[0] == '%';
    }

    std::string number2IRTempVar(std::string &number) {
        return "%" + number;
    }

    std::string number2IRTempVar(int num) {
        return "%" + std::to_string(num);
    }



    bool paramMatchFunc(std::string funcName, MFuncRParams *funcRParams) {
        // TODO
        return true;
    }

    bool funcHasReturn(std::string funcName) {
        // 必须确保funcName是一个函数名
        auto item = (MFunctionSymbolTableItem *) self.globalTable->getTableItem(funcName);
        return item->returnType->type->type != Token::VOIDTK;
    }

};

#endif //BUAA_COMPILER_IRGENERATOR_H
