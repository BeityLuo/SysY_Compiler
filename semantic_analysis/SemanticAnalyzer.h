#ifndef BUAA_COMPILER_SEMANTICANALYZER_H
#define BUAA_COMPILER_SEMANTICANALYZER_H


#include "../syntax_analysis/syntax_nodes.h"
#include "MSymbolTableItem.h"

class SemanticAnalyzer {
private:
    MCompUnit *compUnit;
    MSymbolTable *globalTable;
    MSymbolTable *currentTable;
public:
    SemanticAnalyzer(MCompUnit *compUnit)
            : compUnit(compUnit), globalTable(new MSymbolTable), currentTable(globalTable) {}

    SemanticAnalyzer *analyze() {
        for (MDecl *decl : *(self.compUnit->decls)) {
            self.analyzeDecl(decl);
        }
        for (MFuncDef *funcDef : *(self.compUnit->funcs)) {
            self.analyzeFuncDef(funcDef);
        }
        self.analyzeMainFuncDef(self.compUnit->mainFuncDef);
        return this;
    }

private:

    void analyzeMainFuncDef(MMainFuncDef *mainFuncDef) {
        MSymbolTable *mainFuncTable = new MSymbolTable(self.globalTable);

        // 下面几条语句的顺序不能错
        // 先把main记录到符号表，再分析block
        self.currentTable->addSymbolItem(
                new MFunctionSymbolTableItem("main", 0, new std::vector<MParamType *>(),
                                             new MReturnType(new MFuncType(Token::INTTK)), mainFuncTable));
        self.globalTable->setChildScopeTable(mainFuncTable);
        self.currentTable = mainFuncTable;
        self.analyzeBlock(mainFuncDef->block);

    }

    void analyzeDecl(MDecl *decl) {
        bool isConst = typeid(*decl) == typeid(MConstDecl);
        for (auto def : *(decl->defs)) {
            self.analyzeDef(def, isConst, decl->type);
        }

    }

    void analyzeDef(MDef *def, bool isConst, MBType *type) {
        // 包括了ConstDef和VarDef
        if (self.isRedefined(def->ident->name)) {
            MExceptionManager::pushException(
                    new MRedefinedIdentifierException(def->ident->lineNum));
        } else {
            auto consts = new std::vector<int>();
            if (!def->constExps->empty()) {
                for (auto exp : *(def->constExps)) {
                    consts->push_back(self.valueOfConstExp(exp));
                }
                // TODO 还没有处理初始化的值
            }
            self.currentTable->addSymbolItem(new MVariableSymbolTableItem(
                    def->ident->name, new MVariableType(isConst, type, consts)));
        }
    }

    void analyzeFuncDef(MFuncDef *funcDef) {
//        MFunctionVariableSymbolTableItem(std::string symbol, int paramNum,
//                std::vector<MParamType> *paramTypes,
//                MReturnType *returnType, MSymbolTable *funcSymbolTable, unsigned int address = 0)
        auto paramTypes = new std::vector<MParamType *>();

        for (auto funcFParam : *(funcDef->funcFParams->funcFParams)) {
            auto consts = new std::vector<int>();
            if (!funcFParam->constExps->empty()) {
                for (auto exp : *(funcFParam->constExps)) {
                    consts->push_back(self.valueOfConstExp(exp));
                }
                // TODO 还没有处理初始化的值
            }
            paramTypes->push_back(new MParamType(funcFParam->bType, consts));
        }


        // 下面几句的顺序不能错
        auto funcSymbolTable = new MSymbolTable();

        self.currentTable->addSymbolItem(
                new MFunctionSymbolTableItem(funcDef->ident->name, paramTypes->size(), paramTypes,
                                             new MReturnType(funcDef->funcType), funcSymbolTable));
        self.analyzeBlock(funcDef->block, funcSymbolTable);
    }

    void analyzeBlock(MBlock *block, MSymbolTable *newTable = nullptr) {
        // newTable != nullptr, 就将分析出来的东西加入到newTable中
        // newTable == nullptr, 就新建一个表作为currentTable的子表
        MSymbolTable *preTable = self.currentTable;
        if (newTable != nullptr) {
            self.currentTable = newTable;
        } else {
            self.currentTable = new MSymbolTable(self.currentTable);
        }

        for (auto blockItem : *(block->blockItems)) {
            if (typeid(*blockItem) == typeid(MDeclBlockItem)) {
                self.analyzeDecl(((MDeclBlockItem *) blockItem)->decl);
            } else {
                self.analyzeStmt(((MStmtBlockItem *) blockItem)->stmt);
            }
        }

        self.currentTable = preTable;
    }

    void analyzeStmt(MStmt *stmt) {
        // TODO 检查Stmt的语义
        if (typeid(*stmt) == typeid(MLValStmt)) {
            MLValStmt *pStmt = (MLValStmt *) stmt;
            if (self.isUndefined(pStmt->lVal->ident->name)) {
                throw new MUndefinedIdentifierException(pStmt->lVal->ident->lineNum);
            }
            if (self.isConst(pStmt->lVal->ident->name)) {
                throw new MChangingConstException(pStmt->lVal->ident->lineNum);
            }

            self.analyzeExp(pStmt->exp);
        } else if (typeid(*stmt) == typeid(MExpStmt)) {
            MExpStmt *pStmt = (MExpStmt *) stmt;
            self.analyzeExp(pStmt->exp);
        } else if (typeid(*stmt) == typeid(MNullStmt)) {
            // Do nothing

        } else if (typeid(*stmt) == typeid(MBlockStmt)) {
            MBlockStmt *pStmt = (MBlockStmt *) stmt;
            self.analyzeBlock(pStmt->block);

        } else if (typeid(*stmt) == typeid(MIfStmt)) {
            MIfStmt *pStmt = (MIfStmt *) stmt;
            self.analyzeCond(pStmt->cond);
            self.analyzeStmt(pStmt->ifStmt);
            if (pStmt->elseStmt != nullptr)
                self.analyzeStmt(pStmt->elseStmt);

        } else if (typeid(*stmt) == typeid(MWhileStmt)) {
            MWhileStmt *pStmt = (MWhileStmt *) stmt;
            self.analyzeCond(pStmt->cond);
            self.analyzeStmt(pStmt->stmt);

        } else if (typeid(*stmt) == typeid(MBreakStmt)) {
            MBreakStmt *pStmt = (MBreakStmt *) stmt;
            if (!self.isInCyclicBlock())
                MExceptionManager::pushException(
                        new MBreakStatementInAcyclicBlockException(pStmt->lineNum));

        } else if (typeid(*stmt) == typeid(MContinueStmt)) {
            MContinueStmt *pStmt = (MContinueStmt *) stmt;
            if (!self.isInCyclicBlock())
                MExceptionManager::pushException(
                        new MBreakStatementInAcyclicBlockException(pStmt->lineNum));

        } else if (typeid(*stmt) == typeid(MReturnStmt)) {
            MReturnStmt *pStmt = (MReturnStmt *) stmt;
            if (!self.isInFunctionWithReturn())
                MExceptionManager::pushException(
                        new MRedundantReturnStatementException(pStmt->lineNum));

        } else if (typeid(*stmt) == typeid(MPrintfStmt)) {
            MPrintfStmt *pStmt = (MPrintfStmt *) stmt;
            self.analyzePrintfStmt(pStmt);

        } else {
            throw "In SemanticAnalyzer.analyzeStmt: unknown stmt type";
        }
    }

    void analyzeExp(MExp *exp) {
        self.analyzeAddExp(exp->addExp);
    }

    void analyzeCond(MCond *cond) {
        for (auto andExp : *(cond->lOrExp->lAndExps)) {
            for (auto eqExp : *(andExp->eqExps)) {
                for (auto relExp : *(eqExp->relExps)) {
                    for (auto addExp : *(relExp->addExps)) {
                        self.analyzeAddExp(addExp);
                    }
                }
            }
        }
    }

    void analyzeAddExp(MAddExp *addExp) {
        for (int i = 0; i < addExp->mulExps->size(); i++) {
            MMulExp *mulExp = (*(addExp->mulExps))[i];

            for (int j = 0; j < mulExp->unaryExps->size(); j++) {
                MUnaryExp *unaryExp = (*(mulExp->unaryExps))[j];

                if (typeid(*unaryExp) == typeid(MPrimaryExpUnaryExp)) {
                    MPrimaryExp *primaryExp = ((MPrimaryExpUnaryExp *) unaryExp)->primaryExp;

                    if (typeid(*primaryExp) == typeid(MExpPrimaryExp)) {
                        MExpPrimaryExp *expPrimaryExp = (MExpPrimaryExp *) primaryExp;
                        self.analyzeExp(expPrimaryExp->exp);

                    } else if (typeid(*primaryExp) == typeid(MLValPrimaryExp)) {
                        MLValPrimaryExp *lValPrimaryExp = (MLValPrimaryExp *) primaryExp;
                        if (self.isUndefined(lValPrimaryExp->lVal->ident->name))
                            MExceptionManager::pushException(
                                    new MUndefinedIdentifierException(
                                            lValPrimaryExp->lVal->ident->lineNum));
                    } else if (typeid(*primaryExp) == typeid(MNumberPrimaryExp)) {
                        // Do nothing

                    } else {
                        throw "Fuck12cxvf";
                    }
                } else if (typeid(*unaryExp) == typeid(MFuncUnaryExp)) {
                    self.analyzeFuncUnaryExp((MFuncUnaryExp *) unaryExp);
                }
            }
        }
    }

    void analyzePrintfStmt(MPrintfStmt *printfStmt) {
        if (printfStmt->formatString->getFormatCharsNum() != printfStmt->exps->size())
            MExceptionManager::pushException(
                    new MPrintfParamNumNotMatchException(printfStmt->lineNum));
        // TODO
    }

    void analyzeFuncUnaryExp(MFuncUnaryExp *funcUnaryExp) {
        if (self.isUndefined(funcUnaryExp->ident->name))
            MExceptionManager::pushException(
                    new MUndefinedIdentifierException(funcUnaryExp->ident->lineNum));

        auto tableItem = (MFunctionSymbolTableItem *) (self.currentTable
                ->getTableItem(funcUnaryExp->ident->name));
        if (tableItem->paramNum != funcUnaryExp->funcRParams->exps->size())
            MExceptionManager::pushException(
                    new MParamNumNotMatchException(funcUnaryExp->ident->lineNum));

        int i = 0;
        MExp* exp = nullptr;
        MParamType* paramType = nullptr;
        for (; i < funcUnaryExp->funcRParams->exps->size(); i++){
            // 检查参数类型是否匹配
            exp = (*(funcUnaryExp->funcRParams->exps))[i];
            paramType = tableItem->paramTypes[i];
            if (!self.expMatchParamType(exp, paramType)){
                MExceptionManager::pushException(
                        new MParamTypeNotMatchException(funcUnaryExp->ident->lineNum));
            }
        }

    }

    int valueOfConstExp(MConstExp *constExp) {
        return 0;
    }

    bool isRedefined(std::string name) {

        return false;
    }

    bool isUndefined(std::string name) {
        return false;
    }

    bool isConst(std::string name) {
        return false;
    }

    bool isInCyclicBlock() {
        return false;
    }

    bool isInFunctionWithReturn() {
        return false;
    }

    bool expMatchParamType(MExp* exp, MParamType* paramType) {
        return false;
    }

};

#endif //BUAA_COMPILER_SEMANTICANALYZER_H
