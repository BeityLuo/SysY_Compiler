#include <vector>
#include "../lexical_analyzer/lexical_tools.h"
#include "syntax_nodes.h"
#include "syntax_exceptions.h"
#include "../exception_management/MExceptionManager.h"
#include "../IR_generate/IRGenerator.h"

#define self (*this)

#ifndef BUAA_COMPILER_SYNTAXANALYZER_H
#define BUAA_COMPILER_SYNTAXANALYZER_H

// $$$$$$$$$$$$$$$$$$$$$ 分析过程一概不考虑长度不够导致vector越界   $$$$$$$$$$$$$$$$$$$$$$$$$$$
// 分析过程保证
class ASTGenerator {
private:
    IRGenerator *irGenerator;
    MSymbolTable *globalTable;
    MSymbolTable *currentTable;
    std::vector<TokenLexemePair *> tokenList;
    int index;
    MCompUnit *compUnit;

    std::vector<ComponentNotFoundException *> exceptionStack; //异常栈
    ASTGenerator() {}

public:

    explicit ASTGenerator(std::vector<TokenLexemePair *> &tokenList) :
            tokenList(tokenList), index(0) {}

    ASTGenerator(std::vector<TokenLexemePair *> &tokenList,
                 MSymbolTable *globalTable, IRGenerator *irGenerator) :
            tokenList(tokenList), index(0), globalTable(globalTable), currentTable(globalTable),
            irGenerator(irGenerator) {}

    MCompUnit *analyze() {
        return self.getCompUnit();
    }

    MCompUnit *ast() {
        return self.compUnit;
    }

private:

    Token tokenNow() { return self.tokenList[self.index]->token; }

    void addException(ComponentNotFoundException *e) {
        self.exceptionStack.push_back(e);
    }

    // 统一使用指针而不是引用作为返回值，因为vector里不能放引用
    MCompUnit *getCompUnit() {
        auto decls = new std::vector<MDecl *>();
        MDecl *decl;

        while ((decl = self.getDecl()) != nullptr) {
            decls->push_back(decl);
        }

        auto funcs = new std::vector<MFuncDef *>();
        MFuncDef *funcDef;
        while ((funcDef = self.getFuncDef()) != nullptr) {
            funcs->push_back(funcDef);
        }

        MMainFuncDef *mainFuncDef = self.getMainFuncDef();
        if (mainFuncDef == nullptr) {
//            std::string log;
//            for (auto e : self.exceptionStack) {
//                log += std::string(e->what()) + "\n";
//            }
//            throw LogException(log);
            throw "ASTGenerator::getCompUnit: can't get MainFuncDef";
            return nullptr;
        }
        if (index != self.tokenList.size()) throw LogException("There are something after MainFuncDef");
        self.compUnit = new MCompUnit(decls, funcs, mainFuncDef);
        return self.compUnit;


    }

    MDecl *getDecl() {
        //根据第一个token是不是const判断往哪里走，避免回溯
        if (self.tokenNow() == Token::CONSTTK) {
            return self.getConstDecl();
        } else {
            return self.getVarDecl();
        }
    }

    MConstDecl *getConstDecl() {
        int initial_index = self.index;
        if (tokenNow() != Token::CONSTTK) {
            return nullptr;
        }
        self.index++; //跳过const这个token

        MBType *bType = self.getBType();
        if (bType == nullptr) {
            self.index = initial_index;
            return nullptr;
        }

        auto constDefs = new std::vector<MConstDef *>();
        MConstDef *constDef = self.getConstDef();
        if (constDef == nullptr) {
            self.index = initial_index;
            return nullptr;
        }

        self.irGenerator->addVar2Table(
                bType, constDef->dims, constDef->ident->name, true,
                constDef->init, constDef->ident->lineNum);
        constDefs->push_back(constDef);
        while (self.tokenNow() == Token::COMMA) {
            self.index++;

            constDef = self.getConstDef();
            if (constDef == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            self.irGenerator->addVar2Table(
                    bType, constDef->dims, constDef->ident->name, true,
                    constDef->init, constDef->ident->lineNum);
            constDefs->push_back(constDef);
        }
        if (self.tokenNow() != Token::SEMICN)
            MExceptionManager::pushException(
                    new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
        else
            self.index++;
        return new MConstDecl(bType, constDefs);
    }

    MBType *getBType() {
        if (self.tokenNow() != Token::INTTK) {
            return nullptr;
        }
        self.index++;
        return new MBType();
    }

    MConstDef *getConstDef() {
        int initial_index = self.index;
        if (self.tokenNow() != Token::IDENFR) {
            return nullptr;
        }
        auto *ident = new MIdent(self.tokenList[self.index]->lexeme, self.tokenList[self.index]->lineNum);
        self.index++;

        // auto constExps = new std::vector<MConstExp *>();
        auto dims = new std::vector<int>();
        while (self.tokenNow() == Token::LBRACK) {
            self.index++;
            MConstExp *constExp = self.getConstExp();
            if (constExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            dims->push_back(self.irGenerator->constExp2int(constExp));
            if (self.tokenNow() != Token::RBRACK)
                MExceptionManager::pushException(
                        new MMissingRBracketException(self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;
        }

        // 计算下标的值


        if (self.tokenNow() != Token::ASSIGN) {
            self.index = initial_index;
            return nullptr;
        }
        self.index++;

        MConstInitVal *constInitVal = self.getConstInitVal();
        if (constInitVal == nullptr) {
            self.index = initial_index;
            return nullptr;
        }

        return new MConstDef(ident, dims, constInitVal);
    }

    MConstInitVal *getConstInitVal() {
        int initial_index = self.index;
        if (tokenNow() == Token::LBRACE) {
            self.index++;

            auto constInitVals = new std::vector<MConstInitVal *>();
            MConstInitVal *constInitVal;
            if (self.tokenNow() != Token::RBRACE) {
                constInitVal = self.getConstInitVal();
                if (constInitVal == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                constInitVals->push_back(constInitVal);
            } else {
                // 说明是ConstInitVal -> {}的情况
                // 理论上不会出现这种情况
                self.index = initial_index;
                return nullptr;
            }

            while (self.tokenNow() == Token::COMMA) {
                self.index++;

                constInitVal = self.getConstInitVal();
                if (constInitVal == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                constInitVals->push_back(constInitVal);
            }
            if (tokenNow() != Token::RBRACE) {
                // MExceptionManager::pushException(new ComponentNotFoundException("'}'", "ConstInitVal"));
                self.index = initial_index;
                return nullptr;
            }
            self.index++;

            return new MArrayConstInitVal(constInitVals);
        } else {
            MConstExp *constExp = self.getConstExp();
            if (constExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MConstExpConstInitVal(constExp);
        }
    }

    MVarDecl *getVarDecl() {
        int initial_index = self.index;
        MBType *bType = self.getBType();
        if (bType == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        auto varDefs = new std::vector<MVarDef *>();
        MVarDef *varDef = self.getVarDef();
        if (varDef == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        if (self.tokenNow() == Token::LPARENT) {
            // 进入这里说明是函数定义不是变量定义
            self.index = initial_index;
            return nullptr;
        }
        // 包括了初始化过程
        self.irGenerator->addVar2Table(bType, varDef->dims, varDef->ident->name,
                                       false, varDef->init, varDef->ident->lineNum);
        varDefs->push_back(varDef);
        while (self.tokenNow() == Token::COMMA) {
            self.index++;
            varDef = self.getVarDef();
            if (varDef == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            self.irGenerator->addVar2Table(
                    bType, varDef->dims, varDef->ident->name, false,
                    varDef->init, varDef->ident->lineNum);
            varDefs->push_back(varDef);
        }
        if (self.tokenNow() != Token::SEMICN)
            MExceptionManager::pushException(
                    new MMissingSemicolonException(
                            self.tokenList[self.index - 1]->lineNum));
        else
            self.index++;
        return new MVarDecl(bType, varDefs);
    }

    MVarDef *getVarDef() {
        int initial_index = self.index;
        if (self.tokenNow() != Token::IDENFR) {
            self.index = initial_index;
            return nullptr;
        }
        auto *ident = new MIdent(self.tokenList[self.index]->lexeme, self.tokenList[self.index]->lineNum);
        self.index++;

        auto dims = new std::vector<int>();
        while (self.tokenNow() == Token::LBRACK) {
            self.index++;
            MConstExp *constExp = self.getConstExp();
            if (constExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            dims->push_back(self.irGenerator->constExp2int(constExp));
            if (self.tokenNow() != Token::RBRACK)
                MExceptionManager::pushException(
                        new MMissingRBracketException(self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;
        }

        if (self.tokenNow() == Token::ASSIGN) {
            self.index++;
            MInitVal *initVal = self.getInitVal();
            if (initVal == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MVarDef(ident, dims, initVal);
        } else {
            return new MVarDef(ident, dims);
        }
    }

    MInitVal *getInitVal() {
        int initial_index = self.index;
        if (tokenNow() == Token::LBRACE) {
            self.index++;

            auto initVals = new std::vector<MInitVal *>();
            MInitVal *initVal;
            if (self.tokenNow() != Token::RBRACE) {
                initVal = self.getInitVal();
                if (initVal == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                initVals->push_back(initVal);
            } else {
                // 说明是ConstInitVal -> {}的情况
                // 理论上不会出现这种情况
                self.index = initial_index;
                return nullptr;
            }

            while (self.tokenNow() == Token::COMMA) {
                self.index++;

                initVal = self.getInitVal();
                if (initVal == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                initVals->push_back(initVal);
            }
            if (tokenNow() != Token::RBRACE) {
                // MExceptionManager::pushException(new ComponentNotFoundException("'}'", "InitVal"));
                self.index = initial_index;
                return nullptr;
            }
            self.index++;
            return new MArrayInitVal(initVals);
        } else {
            MExp *exp = self.getExp();
            if (exp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MExpInitVal(exp);
        }
    }

    MFuncDef *getFuncDef() {
        int initial_index = self.index;
        MFuncType *funcType = self.getFuncType();
        if (funcType == nullptr) {
            self.index = initial_index;
            return nullptr;
        }

        if (self.tokenNow() != Token::IDENFR) {
            self.index = initial_index;
            return nullptr;
        }
        auto *ident = new MIdent(self.tokenList[self.index]->lexeme, self.tokenList[self.index]->lineNum);
        self.index++;

        if (self.tokenNow() != Token::LPARENT) {
            self.index = initial_index;
            return nullptr;
        }
        self.index++;

        MFuncFParams *funcFParams = self.getFuncFParams();
        //////////////由于可能函数名字可能重定义，因此要用addFunc2Table返回的名字作为新名字

        auto newName = self.irGenerator->addFunc2Table(funcType, ident->name, funcFParams, ident->lineNum);

        if (self.tokenNow() != Token::RPARENT)
            MExceptionManager::pushException(
                    new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
        else
            self.index++;

        self.irGenerator->createTable(newName);
        self.irGenerator->initParams(newName);

        MBlock *block = self.getBlock("", "");
        int lastBlockLineNum = self.tokenList[self.index - 1]->lineNum; // block的最后的'}'的行号
        // 如果是void类型的函数，给他加一个return语句
        if (funcType->type == Token::VOIDTK) {
            self.irGenerator->addReturn();
        }
        self.irGenerator->backTable(false);
        if (block == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        self.irGenerator->checkBlockReturn(block, funcType->type == Token::VOIDTK, lastBlockLineNum);
        return new MFuncDef(funcType, ident, funcFParams, block);
    }

    MMainFuncDef *getMainFuncDef() {
        int initial_index = self.index;
        if (self.tokenNow() != Token::INTTK) {
            self.index = initial_index;
            return nullptr;
        }
        self.index++;

        if (self.tokenNow() != Token::MAINTK) {
            self.index = initial_index;
            return nullptr;
        }
        int lineNum = self.tokenList[self.index]->lineNum;
        self.index++;

        if (self.tokenNow() != Token::LPARENT) {
            self.index = initial_index;
            return nullptr;
        }
        self.index++;
        self.irGenerator->addFunc2Table(
                new MFuncType(Token::INTTK), "main", nullptr, lineNum);

        if (self.tokenNow() != Token::RPARENT)
            MExceptionManager::pushException(
                    new MMissingRParenthesesException(
                            self.tokenList[self.index - 1]->lineNum));
        else
            self.index++;

        // 创建新的符号表并连接到main函数上
         self.irGenerator->createTable("main");
        MBlock *block = self.getBlock("", ""); // false代表有返回值
        if (block == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        int lastBlockLineNum = self.tokenList[self.index - 1]->lineNum;
        self.irGenerator->backTable(false);
        self.irGenerator->checkBlockReturn(block, false, lastBlockLineNum);
        return new MMainFuncDef(block, lineNum);
    }

    MFuncType *getFuncType() {
        int initial_index = self.index;
        if (self.tokenNow() == Token::VOIDTK) {
            self.index++;
            return new MFuncType(Token::VOIDTK);
        } else if (self.tokenNow() == Token::INTTK) {
            self.index++;
            return new MFuncType(Token::INTTK);
        } else {
            self.index = initial_index;
            return nullptr;
        }
    }

    MFuncFParams *getFuncFParams() {
        int initial_index = self.index;
        auto funcFParams = new std::vector<MFuncFParam *>();
        MFuncFParam *funcFParam = self.getFuncFParam();
        if (funcFParam == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        funcFParams->push_back(funcFParam);
        while (self.tokenNow() == Token::COMMA) {
            self.index++;
            funcFParam = self.getFuncFParam();
            if (funcFParam == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            funcFParams->push_back(funcFParam);
        }
        return new MFuncFParams(funcFParams);
    }

    MFuncFParam *getFuncFParam() {
        int initial_index = self.index;
        MBType *bType = self.getBType();
        if (bType == nullptr) {
            self.index = initial_index;
            return nullptr;
        }

        if (self.tokenNow() != Token::IDENFR) {
            self.index = initial_index;
            return nullptr;
        }
        auto *ident = new MIdent(self.tokenList[self.index]->lexeme, self.tokenList[self.index]->lineNum);
        self.index++;

        if (self.tokenNow() == Token::LBRACK) { // 处理'['
            auto dims = new std::vector<int>();
            self.index++;

            if (self.tokenNow() != Token::RBRACK) // 处理']'
                MExceptionManager::pushException(
                        new MMissingRBracketException(
                                self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;

            MConstExp *constExp;
            while (self.tokenNow() == Token::LBRACK) {
                self.index++;

                constExp = self.getConstExp();
                if (constExp == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                dims->push_back(self.irGenerator->constExp2int(constExp));

                if (self.tokenNow() != Token::RBRACK)
                    MExceptionManager::pushException(
                            new MMissingRBracketException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;
            }
            // self.irGenerator->addVar2Table(bType, nullptr, ident->name, false, nullptr, ident->lineNum);
            return new MFuncFParam(bType, ident, dims, true);
        } else {
            return new MFuncFParam(bType, ident, nullptr);
        }

    }

    MBlock *getBlock(std::string cyclicLabel1, std::string cyclicLabel2) {
        // label用来break和continue

        int initial_index = self.index;
        if (self.tokenNow() != Token::LBRACE) {
            self.index = initial_index;
            return nullptr;
        }
        self.index++;

        auto blockItems = new std::vector<MBlockItem *>();
        MBlockItem *blockItem;
        if (self.tokenNow() == Token::RBRACE) {
            int lineNum = self.tokenList[self.index]->lineNum;
            self.index++;
            return new MBlock(blockItems, lineNum);
        }
        while ((blockItem = self.getBlockItem(cyclicLabel1, cyclicLabel2)) != nullptr) {
            blockItems->push_back(blockItem);
        }

        if (self.tokenNow() != Token::RBRACE) {
            // MExceptionManager::pushException(new ComponentNotFoundException("'}'", "Block"));
            self.index = initial_index;
            return nullptr;
        }
        // 用于错误处理'g'号错误
        int lineNum = self.tokenList[self.index]->lineNum;
        self.index++;
        return new MBlock(blockItems, lineNum);
    }

    MBlockItem *getBlockItem(std::string cyclicLabel1, std::string cyclicLabel2) {
        int initial_index = self.index;

        if (self.tokenNow() == Token::INTTK || self.tokenNow() == Token::CONSTTK) {
            MDecl *decl = self.getDecl();
            if (decl == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MDeclBlockItem(decl);
        } else {
            MStmt *stmt = self.getStmt(cyclicLabel1, cyclicLabel2);
            if (stmt == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MStmtBlockItem(stmt);
        }
    }

    MStmt *getStmt(std::string cyclicLabel1, std::string cyclicLabel2) {
        int initial_index = self.index;
        switch (self.tokenNow()) {
            case Token::IDENFR: {
                // 回溯一下下
                MStmt* stmt = self.getLValStmt();
                if (stmt != nullptr) {
                    return stmt;
                } else if((stmt = self.getExpStmt()) != nullptr){
                    return stmt;
                } else {
                    return nullptr;
                }
                // 可能是Exp中的函数调用，或者是LVal，或者是个普通的exp
//                self.index++;
//                if (self.tokenNow() == Token::LBRACK ||
//                    self.tokenNow() == Token::ASSIGN) {
//                    self.index-- // 消除回溯出了bug
//                    return self.getLValStmt();
//                } else {
//                    self.index--;
//                    return self.getExpStmt();
//                }
            }
            case Token::SEMICN: //空语句
                self.index++;
                return new MNullStmt();
            case Token::LBRACE: {
                self.irGenerator->createTable();
                MBlock *block = self.getBlock(cyclicLabel1, cyclicLabel2);
                self.irGenerator->backTable();
                if (block == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                return new MBlockStmt(block);
            }

            case Token::IFTK: {
                self.index++;
                if (self.tokenNow() != Token::LPARENT) {
                    self.index = initial_index;
                    return nullptr;
                }
                self.index++;

               // 短路求值
                auto condEndLabel = self.irGenerator->generateLabel("cond_end");
                std::string ifEndLabel = self.irGenerator->generateLabel("if_end");
                MCond *ifCond = self.getCond();

                if (ifCond == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }

                if (self.tokenNow() != Token::RPARENT)
                    MExceptionManager::pushException(
                            new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;


                self.irGenerator->branchNotTrue(ifCond, condEndLabel, ifEndLabel);
                self.irGenerator->setLabel(condEndLabel); // 短路求值

                MStmt *ifStmt = self.getStmt(cyclicLabel1, cyclicLabel2);
                if (ifStmt == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                if (self.tokenNow() == Token::ELSETK) {
                    // 处理有else的情况
                    self.index++;


                    std::string elseEndLabel = self.irGenerator->generateLabel("else_end");
                    self.irGenerator->branch(elseEndLabel);
                    self.irGenerator->setLabel(ifEndLabel);
                    MStmt *elseStmt = self.getStmt(cyclicLabel1, cyclicLabel2);
                    if (elseStmt == nullptr) {
                        self.index = initial_index;
                        return nullptr;
                    }
                    self.irGenerator->setLabel(elseEndLabel);
                    return new MIfStmt(ifCond, ifStmt, elseStmt);
                } else {
                    self.irGenerator->setLabel(ifEndLabel);
                    return new MIfStmt(ifCond, ifStmt);
                }
            }

            case Token::WHILETK: {
                self.index++;
                if (self.tokenNow() != Token::LPARENT) {
                    self.index = initial_index;
                    return nullptr;
                }
                self.index++;
                std::string whileBeginLabel = self.irGenerator->generateLabel("while_begin");
                std::string condEndLabel = self.irGenerator->generateLabel("cond_end");
                self.irGenerator->setLabel(whileBeginLabel);
                MCond *whileCond = self.getCond();
                if (whileCond == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                if (self.tokenNow() != Token::RPARENT)
                    MExceptionManager::pushException(
                            new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;
                std::string whileEndLabel = self.irGenerator->generateLabel("while_end");
                self.irGenerator->branchNotTrue(whileCond, condEndLabel, whileEndLabel);
                self.irGenerator->setLabel(condEndLabel);
                MStmt *whileStmt = self.getStmt(whileBeginLabel, whileEndLabel);
                self.irGenerator->branch(whileBeginLabel);
                self.irGenerator->setLabel(whileEndLabel);
                if (whileStmt == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                return new MWhileStmt(whileCond, whileStmt);
            }

            case Token::BREAKTK: {
                int lineNum = self.tokenList[self.index]->lineNum;
                self.index++;
                if (self.tokenNow() != Token::SEMICN)
                    MExceptionManager::pushException(
                            new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;
                if (cyclicLabel2.empty()) {
                    MExceptionManager::pushException(
                            new MBreakStatementInAcyclicBlockException(lineNum));
                } else {
                    self.irGenerator->branch(cyclicLabel2);
                }
                return new MBreakStmt(lineNum);
            }

            case Token::CONTINUETK: {
                int lineNum = self.tokenList[self.index]->lineNum;
                self.index++;
                if (self.tokenNow() != Token::SEMICN)
                    MExceptionManager::pushException(
                            new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;
                if (cyclicLabel1.empty()) {
                    MExceptionManager::pushException(
                            new MContinueStatementInAcyclicBlockException(lineNum));
                } else {
                    self.irGenerator->branch(cyclicLabel1);
                }
                return new MContinueStmt(lineNum);
            }

            case Token::RETURNTK: {
                self.index++;
                int lineNum = self.tokenList[self.index]->lineNum;
                MExp *exp;
                if (self.tokenNow() == Token::SEMICN) {
                    self.index++;
                    self.irGenerator->returnBack();
                    return new MReturnStmt(nullptr, lineNum);
                } else {
                    exp = self.getExp();
                    if (exp == nullptr) {
                        self.index = initial_index;
                        return nullptr;
                    }
                    if (self.tokenNow() != Token::SEMICN)
                        MExceptionManager::pushException(
                                new MMissingSemicolonException(
                                        self.tokenList[self.index - 1]->lineNum));
                    else
                        self.index++;
                    self.irGenerator->setReturnValue(exp);
                    self.irGenerator->returnBack();
                    return new MReturnStmt(exp, lineNum);
                }
            }

            case Token::PRINTFTK: {
                self.index++;
                int lineNum = self.tokenList[self.index]->lineNum;

                if (self.tokenNow() != Token::LPARENT) {
                    self.index = initial_index;
                    return nullptr;
                }
                self.index++;

                if (self.tokenNow() != Token::STRCON) {
                    self.index = initial_index;
                    return nullptr;
                }
                auto formatString = new MFormatString(self.tokenList[self.index]->lexeme);
                if (!formatString->checkLegal()) {
                    MExceptionManager::pushException(
                            new MIllegalCharInFormatStringException(self.tokenList[self.index]->lineNum));
                }
                self.index++;

                auto printfExps = new std::vector<MExp *>();
                MExp *printfExp;
                while (self.tokenNow() == Token::COMMA) {
                    self.index++;
                    if ((printfExp = self.getExp()) == nullptr) {
                        self.index = initial_index;
                        return nullptr;
                    }
                    printfExps->push_back(printfExp);
                }

                if (self.tokenNow() != Token::RPARENT) {
                    MExceptionManager::pushException(
                            new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
                    self.index = initial_index;
                    return nullptr;
                }
                self.index++;

                if (self.tokenNow() != Token::SEMICN)
                    MExceptionManager::pushException(
                            new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;
                self.irGenerator->printf(formatString, printfExps, lineNum);
                return new MPrintfStmt(formatString, printfExps, lineNum);
            }

            default: {
                // 理论上讲这里什么都不用做
                return self.getExpStmt();
            }

        }
    }

    MLValStmt *getLValStmt() {
        int initial_index = self.index;
        // 处理"LVal = getint();" 和 "LVal = Exp;"
        MLVal *lVal = self.getLVal();

        if (lVal == nullptr) {
            self.index = initial_index;
            return nullptr;
        }

        if (self.tokenNow() != Token::ASSIGN) {
            self.index = initial_index;
            return nullptr;
        }
        self.index++;

        if (self.tokenNow() == Token::GETINTTK) {
            // 处理"LVal = getint();"
            self.index++;
            if (self.tokenNow() != Token::LPARENT) {
                self.index = initial_index;
                return nullptr;
            }
            self.index++;
            if (self.tokenNow() != Token::RPARENT) {
                MExceptionManager::pushException(
                        new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
                self.index = initial_index;
                return nullptr;
            }
            self.index++;
            if (self.tokenNow() != Token::SEMICN)
                MExceptionManager::pushException(
                        new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;
            self.irGenerator->getint(lVal);
            return new MLValStmt(lVal);
        } else {
            // 处理"LVal = Exp;"
            MExp *exp = self.getExp();
            if (exp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            if (self.tokenNow() != Token::SEMICN)
                MExceptionManager::pushException(
                        new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;
            self.irGenerator->assignExp2LVal(lVal, exp);
            return new MLValStmt(lVal, exp);
        }
    }

    MExpStmt *getExpStmt() {
        // TODO 这里是不是啥都不用做
        int initial_index = self.index;
        MExp *exp = self.getExp();
        if (exp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        if (self.tokenNow() != Token::SEMICN)
            MExceptionManager::pushException(
                    new MMissingSemicolonException(self.tokenList[self.index - 1]->lineNum));
        else
            self.index++;
        self.irGenerator->handleExp(exp);
        return new MExpStmt(exp);
    }


    MExp *getExp() {
        int initial_index = self.index;
        MAddExp *addExp = self.getAddExp();
        if (addExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        return new MExp(addExp);
    }

    MCond *getCond() {
        int initial_index = self.index;
        MLOrExp *lOrExp = self.getLOrExp();
        if (lOrExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        return new MCond(lOrExp);
    }

    MLVal *getLVal() {
        int initial_index = self.index;
        if (self.tokenNow() != Token::IDENFR) {
            self.index = initial_index;
            return nullptr;
        }
        auto *ident = new MIdent(self.tokenList[self.index]->lexeme, self.tokenList[self.index]->lineNum);
        self.index++;

        auto exps = new std::vector<MExp *>();
        MExp *exp;
        while (self.tokenNow() == Token::LBRACK) {
            self.index++;

            if ((exp = self.getExp()) == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            exps->push_back(exp);

            if (self.tokenNow() != Token::RBRACK)
                MExceptionManager::pushException(
                        new MMissingRBracketException(self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;
        }
        return new MLVal(ident, exps);
    }

    MPrimaryExp *getPrimaryExp() {
        int initial_index = self.index;

        if (self.tokenNow() == Token::LPARENT) {
            self.index++;
            MExp *exp = self.getExp();
            if (exp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }

            if (self.tokenNow() != Token::RPARENT)
                MExceptionManager::pushException(
                        new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
            else
                self.index++;
            return new MExpPrimaryExp(exp);
        } else if (self.tokenNow() == Token::IDENFR) {
            MLVal *lVal = self.getLVal();
            if (lVal == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MLValPrimaryExp(lVal);
        } else if (self.tokenNow() == Token::INTCON) {
            MNumber *number = self.getNumber();
            if (number == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MNumberPrimaryExp(number);
        } else {
            self.index = initial_index;
            return nullptr;
        }
    }

    MNumber *getNumber() {
        if (self.tokenNow() != Token::INTCON) {
            return nullptr;
        }
        auto *intConst = new MIntConst(self.tokenList[self.index]->lexeme);
        self.index++;
        return new MNumber(intConst);
    }

    MUnaryExp *getUnaryExp() {
        int initial_index = self.index;

        if (self.tokenNow() == Token::IDENFR) {
            // 处理UnaryExp->Ident([FuncRParams]) 和 UnaryExp->PrimaryExp->LVal
            auto *ident = new MIdent(self.tokenList[self.index]->lexeme, self.tokenList[self.index]->lineNum);
            self.index++;

            if (self.tokenNow() == Token::LPARENT) {
                self.index++;

                MFuncRParams *funcRParams = self.getFuncRParams();

                if (self.tokenNow() != Token::RPARENT)
                    MExceptionManager::pushException(
                            new MMissingRParenthesesException(self.tokenList[self.index - 1]->lineNum));
                else
                    self.index++;
                return new MFuncUnaryExp(ident, funcRParams);
            } else {
                self.index--; //回到指向IDENFR的状态
                MPrimaryExp *primaryExp = self.getPrimaryExp();
                if (primaryExp == nullptr) {
                    self.index = initial_index;
                    return nullptr;
                }
                return new MPrimaryExpUnaryExp(primaryExp);
            }
        } else if (self.tokenNow() == Token::PLUS
                   || self.tokenNow() == Token::MINU
                   || self.tokenNow() == Token::NOT) {
            auto *unaryOp = new MUnaryOp(self.tokenNow());
            self.index++;

            MUnaryExp *unaryExp = self.getUnaryExp();
            if (unaryExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }

            return new MUnaryExpUnaryExp(unaryOp, unaryExp);
        } else {
            MPrimaryExp *primaryExp = self.getPrimaryExp();
            if (primaryExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            return new MPrimaryExpUnaryExp(primaryExp);
        }
    }

    MFuncRParams *getFuncRParams() {
        int initial_index = self.index;
        auto exps = new std::vector<MExp *>();
        MExp *exp = self.getExp();
        if (exp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        exps->push_back(exp);
        while (self.tokenNow() == Token::COMMA) {
            self.index++;

            exp = self.getExp();
            if (exp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            exps->push_back(exp);
        }
        return new MFuncRParams(exps);
    }

    MMulExp *getMulExp() {
        int initial_index = self.index;
        auto unaryExps = new std::vector<MUnaryExp *>();
        auto mulOps = new std::vector<Token>;
        MUnaryExp *unaryExp = self.getUnaryExp();
        if (unaryExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        unaryExps->push_back(unaryExp);

        while (self.tokenNow() == Token::MULT
               || self.tokenNow() == Token::DIV
               || self.tokenNow() == Token::MOD) {
            mulOps->push_back(self.tokenNow());
            self.index++;

            unaryExp = self.getUnaryExp();
            if (unaryExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            unaryExps->push_back(unaryExp);
        }
        return new MMulExp(unaryExps, mulOps); //如果没op则传一个空列表而不是nullptr
    }

    MAddExp *getAddExp() {
        int initial_index = self.index;
        auto mulExps = new std::vector<MMulExp *>();
        auto addOps = new std::vector<Token>;
        MMulExp *mulExp = self.getMulExp();
        if (mulExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        mulExps->push_back(mulExp);
        while (self.tokenNow() == Token::PLUS
               || self.tokenNow() == Token::MINU) {
            auto opToken = self.tokenNow();
            addOps->push_back(opToken);
            self.index++;

            mulExp = self.getMulExp();
            if (mulExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            mulExps->push_back(mulExp);
        }
        return new MAddExp(mulExps, addOps);
    }

    MRelExp *getRelExp() {
        int initial_index = self.index;
        auto addExps = new std::vector<MAddExp *>();
        auto relOps = new std::vector<Token>;
        MAddExp *addExp = self.getAddExp();
        if (addExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        addExps->push_back(addExp);

        while (self.tokenNow() == Token::LSS
               || self.tokenNow() == Token::GRE
               || self.tokenNow() == Token::LEQ
               || self.tokenNow() == Token::GEQ) {
            relOps->push_back(self.tokenNow());
            self.index++;

            addExp = self.getAddExp();
            if (addExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            addExps->push_back(addExp);
        }
        return new MRelExp(addExps, relOps);
    }

    MEqExp *getEqExp() {
        int initial_index = self.index;
        auto relExps = new std::vector<MRelExp *>();
        auto eqOps = new std::vector<Token>;
        MRelExp *relExp = self.getRelExp();
        if (relExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        relExps->push_back(relExp);

        while (self.tokenNow() == Token::EQL
               || self.tokenNow() == Token::NEQ) {
            eqOps->push_back(self.tokenNow());
            self.index++;

            relExp = self.getRelExp();
            if (relExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            relExps->push_back(relExp);
        }
        return new MEqExp(relExps, eqOps);
    }

    MLAndExp *getLAndExp() {
        int initial_index = self.index;
        auto eqExps = new std::vector<MEqExp *>();
        auto lAndOps = new std::vector<Token>;
        MEqExp *eqExp = self.getEqExp();
        if (eqExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        eqExps->push_back(eqExp);

        while (self.tokenNow() == Token::AND) {
            lAndOps->push_back(self.tokenNow());
            self.index++;

            eqExp = self.getEqExp();
            if (eqExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            eqExps->push_back(eqExp);
        }
        return new MLAndExp(eqExps, lAndOps);
    }

    MLOrExp *getLOrExp() {
        int initial_index = self.index;
        auto lAndExps = new std::vector<MLAndExp *>();
        auto lOrOps = new std::vector<Token>;
        MLAndExp *lAndExp = self.getLAndExp();
        if (lAndExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        lAndExps->push_back(lAndExp);

        while (self.tokenNow() == Token::OR) {
            lOrOps->push_back(self.tokenNow());
            self.index++;

            lAndExp = self.getLAndExp();
            if (lAndExp == nullptr) {
                self.index = initial_index;
                return nullptr;
            }
            lAndExps->push_back(lAndExp);
        }
        return new MLOrExp(lAndExps, lOrOps);
    }

    MConstExp *getConstExp() {
        int initial_index = index;
        MAddExp *addExp = self.getAddExp();
        if (addExp == nullptr) {
            self.index = initial_index;
            return nullptr;
        }
        return new MConstExp(addExp);
    }


};

#endif //BUAA_COMPILER_SYNTAXANALYZER_H
