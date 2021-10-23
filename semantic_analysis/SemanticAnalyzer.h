#ifndef BUAA_COMPILER_SEMANTICANALYZER_H
#define BUAA_COMPILER_SEMANTICANALYZER_H


#include "../syntax_analysis/syntax_nodes.h"
#include "SymbolTable.h"

class SemanticAnalyzer {
private:
    MCompUnit *compUnit;
    MSymbolTable *globalTable;
    MSymbolTable *currentTable;
public:
    SemanticAnalyzer(MCompUnit *compUnit)
            : compUnit(compUnit), globalTable(new MSymbolTable), currentTable(globalTable) {}

    SemanticAnalyzer *analyze() {
        for (MDecl* decl : self.compUnit->getDecls()) {
            self.analyzeDecl(decl);
        }
        for (MFuncDef* funcDef : self.compUnit->getFuncDefs()) {
            self.analyzeFuncDef(funcDef);
        }
        self.analyzeMainFuncDef(self.compUnit->getMainFuncDef());
        return this;
    }

    void analyzeDecl(MDecl *decl) {
        bool isConst = typeid(*decl) == typeid(MConstDecl);


    }

    void analyzeFuncDef(MFuncDef *funcDef) {

    }

    void analyzeStmt(MStmt* stmt) {

    }

    void analyzeMainFuncDef(MMainFuncDef* mainFuncDef) {
        /////////////////////////////////////////////////////
        // 一定要记得把main这个符号添加到符号表里去并检查是否重名！
        /////////////////////////////////////////////////////
        self.currentTable = new MSymbolTable(self.globalTable);
        self.globalTable->setChildScopeTable(self.currentTable);

        MBlock* block = mainFuncDef->getBlock();
        std::vector<MBlockItem*> blockItems = block->getBlockItems();
        for (auto blockItem : blockItems) {
            if (blockItem->isDecl()) {
                self.analyzeDecl(blockItem->getItem());
            } else {
                self.analyzeStmt(blockItem->getItem());
            }
        }
    }

};

#endif //BUAA_COMPILER_SEMANTICANALYZER_H
