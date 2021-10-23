#include <utility>


#ifndef BUAA_COMPILER_SYNTAX_NODES_H
#define BUAA_COMPILER_SYNTAX_NODES_H

#include <vector>
#include <string>
#include "syntax_nodes_classes.h"
#include "../lexical_analyzer/lexical_tools.h"

#define self (*this)

template<class T>
void pushVector(vector<T>* v1, vector<T>* v2) {
    for (auto vv : *v2) {
        v1->push_back(vv);
    }
}

class MSyntaxNode {
public:
    virtual std::string toString() = 0; // 全部都带有后缀的'\n'
    virtual std::string className() = 0;
};

class MCompUnit : public MSyntaxNode {
private:
    std::vector<MDecl *> *decls;
    std::vector<MFuncDef *> *funcs;
    MMainFuncDef *mainFuncDef;
public:
    MCompUnit(std::vector<MDecl *> *decls, std::vector<MFuncDef *> *funcs, MMainFuncDef *mainFuncDef) : mainFuncDef(mainFuncDef) {
        pushVector(self.decls, decls);
        pushVector(self.funcs, funcs);
    }

    std::string toString() override;

    std::string className() override {
        return "<CompUnit>\n";
    }
};



class MBType : MSyntaxNode {
    // 不输出自己
public:
    std::string className() override {
        return "<BType>\n"; //虽然用不到还是要实现这个方法，否则不能实例化
    }

    std::string toString() override {
        return fixedToken2PairString(Token::INTTK);
    }
};



class MVariableInit : public MSyntaxNode {

};

class MConstInitVal : public MVariableInit {
public:
    std::string className() override {
        return "<ConstInitVal>\n";
    }
};

class MConstExpConstInitVal : public MConstInitVal {
private:
    MConstExp *constExp;
public:
    explicit MConstExpConstInitVal(MConstExp *constExp) : constExp(constExp) {}

    std::string toString() override;

};

class MArrayConstInitVal : public MConstInitVal {
private:
    std::vector<MConstInitVal *> *constInitVals;
public:
    explicit MArrayConstInitVal(std::vector<MConstInitVal *> *constInitVals) {
        pushVector(self.constInitVals, constInitVals);
    }

    std::string toString() override;

};

class MInitVal : public MVariableInit {
public:
    std::string className() override {
        return "<InitVal>\n";
    }
};

class MExpInitVal : public MInitVal {
private:
    MExp *exp;
public:
    explicit MExpInitVal(MExp *exp) : exp(exp) {}

    std::string toString() override;

};

class MArrayInitVal : public MInitVal {
private:
    std::vector<MInitVal *> *initVals;
public:
    explicit MArrayInitVal(std::vector<MInitVal *> *initVals) {
        pushVector(self.initVals, initVals);
    }

    std::string toString() override;
};

class MDef : public MSyntaxNode {
protected:
    MIdent *ident;
    std::vector<MConstExp *> *constExps;
    MVariableInit *init;
public:
    MDef(MIdent *ident, std::vector<MConstExp *> *constExps, MVariableInit *init)
            : ident(ident), init(init) {
        pushVector(self.constExps, constExps);
    }
    MDef() : ident(nullptr), constExps(new std::vector<MConstExp*>()), init(nullptr) {}
};

class MConstDef : public MDef {

public:
    MConstDef(MIdent *ident, std::vector<MConstExp *> *constExps, MConstInitVal *constInitVal) {
        self.ident = ident;
        self.init = constInitVal;
        for (auto constExp : *constExps) {
            self.constExps->push_back(constExp);
        }
    }

    std::string className() override {
        return "<ConstDef>\n";
    }

    std::string toString() override;

};

class MVarDef : public MDef {
public:
    MVarDef(MIdent *ident, std::vector<MConstExp *> *constExps, MInitVal *initVal = nullptr)
            : MDef(ident, constExps, initVal) {
        self.ident = ident;
        self.init = initVal;
        for (auto constExp : *constExps) {
            self.constExps->push_back(constExp);
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<VarDef>\n";
    }

};

class MDecl : public MSyntaxNode {
protected:
    MBType *type;
    std::vector<MDef *> *defs;
public:
    MDecl() : type(nullptr), defs(new std::vector<MDef *>()) {}
    // 不输出自己
    // 也就意味着这个类没有存在的必要性了
};

class MConstDecl : public MDecl {
public:
    MConstDecl(MBType *type, std::vector<MConstDef *> *constDefs) {
        self.type = type;
        for (auto constDef : *constDefs) {
            self.defs->push_back(constDef);
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<ConstDecl>\n";
    }


};

class MVarDecl : public MDecl {
public:

    MVarDecl(MBType *type, std::vector<MVarDef *> *varDefs) {
        self.type = type;
        for (auto varDef : *varDef) {
            self.defs->push_back(varDef);
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<VarDecl>\n";
    }

};

class MFuncDef : public MSyntaxNode {
private:
    MFuncType *funcType;
    MIdent *ident;
    MFuncFParams *funcFParams;
    MBlock *block;
public:
    MFuncDef(MFuncType *funcType, MIdent *ident, MFuncFParams *funcFParams, MBlock *block)
            : funcType(funcType), ident(ident), funcFParams(funcFParams), block(block) {}

    std::string toString() override;

    std::string className() override {
        return "<FuncDef>\n";
    }
};

class MMainFuncDef : public MSyntaxNode {
private:
    MBlock *block;
public:
    explicit MMainFuncDef(MBlock *block) : block(block) {}

    std::string toString() override;

    std::string className() override {
        return "<MainFuncDef>\n";
    }
};

class MFuncType : public MSyntaxNode {
private:
    Token type;
public:
    explicit MFuncType(Token token) {
        if (token == Token::VOIDTK || token == Token::INTTK) {
            self.type = token;
            return;
        } else
            throw "Wong type for MFuncType's Constructor: " + token2string(token);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncType>\n";
    }
};

class MFuncFParams : public MSyntaxNode {
private:
    std::vector<MFuncFParam *> *funcFParams;
public:
    explicit MFuncFParams(std::vector<MFuncFParam *> *funcFParams) {
        pushVector(self.funcFParams, funcFParams);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncFParams>\n";
    }
};

class MFuncFParam : public MSyntaxNode {
private:
    MBType *bType;
    MIdent *ident;
    std::vector<MConstExp *> *constExps;
public:
    MFuncFParam(MBType *bType, MIdent *ident, std::vector<MConstExp *> *constExps) :
            bType(bType), ident(ident) {
        pushVector(self.constExps, constExps);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncFParam>\n";
    }
};

class MBlock : public MSyntaxNode {
private:
    std::vector<MBlockItem *> *blockItems;
public:

    explicit MBlock(std::vector<MBlockItem *> *blockItems) {
        pushVector(self.blockItems, blockItems);
    }

    std::string toString() override;

    std::string className() override {
        return "<Block>\n";
    }
};

class MBlockItem : public MSyntaxNode {
    // 不输出自己
public:
    std::string className() override {
        return "<BlockItem>\n";
    }
};

class MDeclBlockItem : public MBlockItem {
private:
    MDecl *decl;
public:
    explicit MDeclBlockItem(MDecl *decl) : decl(decl) {}

    std::string toString() override;
};

class MStmtBlockItem : public MBlockItem {
private:
    MStmt *stmt;
public:
    explicit MStmtBlockItem(MStmt *stmt) : stmt(stmt) {}

    std::string toString() override;
};

class MStmt : public MSyntaxNode {
public:
    std::string className() override {
        return "<Stmt>\n";
    }
};

class MLValStmt : public MStmt {
private:
    MLVal *lVal;
    MExp *exp;
public:
    explicit MLValStmt(MLVal *lVal) : lVal(lVal), exp(nullptr) {}

    MLValStmt(MLVal *lVal, MExp *exp) : lVal(lVal), exp(exp) {}

    std::string toString() override;
};

class MExpStmt : public MStmt {
private:
    MExp *exp;
public:
    explicit MExpStmt(MExp *exp) : exp(exp) {}

    std::string toString() override;
};

class MNullStmt : public MStmt {
public:
    MNullStmt() = default;

    std::string toString() override;
};

class MBlockStmt : public MStmt {
private:
    MBlock *block;
public:
    explicit MBlockStmt(MBlock *block) : block(block) {}

    std::string toString() override;
};

class MIfStmt : public MStmt {
private:
    MCond *cond;
    MStmt *ifStmt;
    MStmt *elseStmt;
public:
    MIfStmt(MCond *cond, MStmt *ifStmt, MStmt *elseStmt) : cond(cond), ifStmt(ifStmt), elseStmt(elseStmt) {}

    MIfStmt(MCond *cond, MStmt *ifStmt) : cond(cond), ifStmt(ifStmt), elseStmt(nullptr) {}

    std::string toString() override;
};

class MWhileStmt : public MStmt {
private:
    MCond *cond;
    MStmt *stmt;
public:
    MWhileStmt(MCond *cond, MStmt *stmt) : cond(cond), stmt(stmt) {}

    std::string toString() override;
};

class MBreakStmt : public MStmt {
public:
    std::string toString() override;
};

class MContinueStmt : public MStmt {
public:
    std::string toString() override;
};

class MReturnStmt : public MStmt {
private:
    MExp *exp;
public:
    explicit MReturnStmt(MExp *exp) : exp(exp) {}

    MReturnStmt() : exp(nullptr) {}

    std::string toString() override;
};

class MPrintfStmt : public MStmt {
private:
    MFormatString *formatString;
    std::vector<MExp *> *exps;
public:
    MPrintfStmt(MFormatString *formatString, std::vector<MExp *> *exps)
            : formatString(formatString) {
        pushVector(self.exps, exps);
    }

    std::string toString() override;
};

class MExp : public MSyntaxNode {
private:
    MAddExp *addExp;
public:
    explicit MExp(MAddExp *addExp) : addExp(addExp) {}

    std::string toString() override;

    std::string className() override {
        return "<Exp>\n";
    }
};

class MConstExp : public MSyntaxNode {
private:
    MAddExp *addExp;
public:
    explicit MConstExp(MAddExp *addExp) : addExp(addExp) {}

    std::string toString() override;

    std::string className() override {
        return "<ConstExp>\n";
    }
};


class MCond : public MSyntaxNode {
private:
    MLOrExp *lOrExp;
public:
    explicit MCond(MLOrExp *lOrExp) : lOrExp(lOrExp) {}

    std::string toString() override;

    std::string className() override {
        return "<Cond>\n";
    }
};

class MLVal : public MSyntaxNode {
private:
    MIdent *ident;
    std::vector<MExp *> *exps;
public:
    MLVal(MIdent *ident, std::vector<MExp *> *exps)
            : ident(ident) {
        pushVector(self.exps, exps);
    }

    std::string toString() override;

    std::string className() override {
        return "<LVal>\n";
    }
};

class MPrimaryExp : public MSyntaxNode {
public:
    std::string className() override {
        return "<PrimaryExp>\n";
    }
};

class MExpPrimaryExp : public MPrimaryExp {
private:
    MExp *exp;
public:
    explicit MExpPrimaryExp(MExp *exp) : exp(exp) {}

    std::string toString() override;
};

class MLValPrimaryExp : public MPrimaryExp {
private:
    MLVal *lVal;
public:
    explicit MLValPrimaryExp(MLVal *lVal) : lVal(lVal) {}

    std::string toString() override;
};

class MNumberPrimaryExp : public MPrimaryExp {
private:
    MNumber *number;
public:
    explicit MNumberPrimaryExp(MNumber *number) : number(number) {}

    std::string toString() override;
};

class MNumber : public MSyntaxNode {
private:
    MIntConst *intConst;
public:
    explicit MNumber(MIntConst *intConst) : intConst(intConst) {}

    std::string toString() override;

    std::string className() override {
        return "<Number>\n";
    }
};

class MUnaryExp : public MSyntaxNode {
public:
    std::string className() override {
        return "<UnaryExp>\n";
    }
};

class MPrimaryExpUnaryExp : public MUnaryExp {
private:
    MPrimaryExp *primaryExp;
public:
    explicit MPrimaryExpUnaryExp(MPrimaryExp *primaryExp) : primaryExp(primaryExp) {}

    std::string toString() override;
};

class MFuncUnaryExp : public MUnaryExp {
private:
    MIdent *ident;
    MFuncRParams *funcRParams;
public:
    MFuncUnaryExp(MIdent *ident, MFuncRParams *funcRParams)
            : ident(ident), funcRParams(funcRParams) {}

    std::string toString() override;
};

class MUnaryExpUnaryExp : public MUnaryExp {
private:
    MUnaryOp *unaryOp;
    MUnaryExp *unaryExp;
public:
    MUnaryExpUnaryExp(MUnaryOp *unaryOp, MUnaryExp *unaryExp)
            : unaryOp(unaryOp), unaryExp(unaryExp) {}

    std::string toString() override;
};

class MUnaryOp : public MSyntaxNode {
private:
    Token op;
public:
    // 只能是
    explicit MUnaryOp(Token op) : op(op) {
        if (op != Token::PLUS && op != Token::MINU && op != Token::NOT)
            throw "In MUnaryOp's constructor: wong para: op = " + token2string(op);
    }

    std::string toString() override;

    std::string className() override {
        return "<UnaryOp>\n";
    }

};

class MFuncRParams : public MSyntaxNode {
private:
    std::vector<MExp *> *exps;
public:
    explicit MFuncRParams(std::vector<MExp *> *exps) {
        pushVector(self.exps, exps);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncRParams>\n";
    }
};

class MMulExp : public MSyntaxNode {
private:
    std::vector<MUnaryExp *> *unaryExps;
    std::vector<Token> *ops;
public:
    MMulExp(std::vector<MUnaryExp *> *unaryExps, std::vector<Token> *ops)
            : unaryExps(unaryExps) {
        pushVector(self.ops, ops);

        if (self.unaryExps->size() != self.ops->size() + 1) {
            throw "In MMulExp's constructor: size of unaryExps ans ops not match";
        }
        for (Token op : *(self.ops)) {
            if (op != Token::MULT && op != Token::DIV && op != Token::MOD) {
                throw "In MMulExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<MulExp>\n";
    }
};

class MAddExp : public MSyntaxNode {
private:
    std::vector<MMulExp *> *mulExps;
    std::vector<Token> *ops;
public:
    MAddExp(std::vector<MMulExp *> *mulExps, std::vector<Token> *ops)
            : mulExps(mulExps) {
        pushVector(self.ops, ops);

        if (self.mulExps->size() != self.ops->size() + 1) {
            throw "In MAddExp's constructor: size of mulExps ans ops not match";
        }
        for (Token op : *(self.ops)) {
            if (op != Token::PLUS && op != Token::MINU) {
                throw "In MAddExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<AddExp>\n";
    }
};

class MRelExp : public MSyntaxNode {
private:
    std::vector<MAddExp *> *addExps;
    std::vector<Token> *ops;
public:
    MRelExp(std::vector<MAddExp *> *addExps, std::vector<Token> *ops)
            : addExps(addExps) {
        pushVector(self.ops, ops);
        if (self.addExps->size() != self.ops->size() + 1) {
            throw "In MAddExp's constructor: size of addExps ans ops not match";
        }
        for (Token op : *(self.ops)) {
            if (op != Token::LSS && op != Token::LEQ
                && op != Token::GRE && op != Token::GEQ) {
                throw "In MAddExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<RelExp>\n";
    }
};

class MEqExp : public MSyntaxNode {
private:
    std::vector<MRelExp *> *relExps;
    std::vector<Token> *ops;
public:
    MEqExp(std::vector<MRelExp *> *relExps, std::vector<Token> *ops)
            : relExps(relExps) {
        pushVector(self.ops, ops);

        if (self.relExps->size() != self.ops->size() + 1) {
            throw "In MEqExp's constructor: size of relExps ans ops not match";
        }
        for (Token op : *(self.ops)) {
            if (op != Token::EQL && op != Token::NEQ) {
                throw "In MEqExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<EqExp>\n";
    }
};

class MLAndExp : public MSyntaxNode {
private:
    std::vector<MEqExp *> *eqExps;
    std::vector<Token> *ops;
public:
    MLAndExp(std::vector<MEqExp *> *eqExps, std::vector<Token> *ops)
            : eqExps(eqExps) {
        pushVector(self.ops, ops);

        if (self.eqExps->size() != self.ops->size() + 1) {
            throw "In MLAndExp's constructor: size of eqExps ans ops not match";
        }
        for (Token op : *(self.ops)) {
            if (op != Token::AND) {
                throw "In MLAndExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<LAndExp>\n";
    }
};


class MLOrExp : public MSyntaxNode {
private:
    std::vector<MLAndExp *> *lAndExps;
    std::vector<Token> *ops;
public:
    MLOrExp(std::vector<MLAndExp *> *lAndExps, std::vector<Token> *ops)
            : lAndExps(lAndExps) {
        pushVector(self.ops, ops);

        if (self.lAndExps->size() != self.ops->size() + 1) {
            throw "In MLOrExp's constructor: size of lAndExps ans ops not match";
        }
        for (Token op : *(self.ops)) {
            if (op != Token::OR) {
                throw "In MLOrExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<LOrExp>\n";
    }
};

class MIdent : public MSyntaxNode {
private:
    std::string ident;
public:
    explicit MIdent(std::string ident) : ident(std::move(ident)) {}

    std::string toString() override {
        return token2string(Token::IDENFR) + " " + ident + "\n";
    }

    std::string className() override {
        return "<Ident>\n";
    }
};

class MIntConst : public MSyntaxNode {
private:
    std::string intConst;
public:
    explicit MIntConst(std::string intConst) : intConst(std::move(intConst)) {}

    std::string toString() override {
        return token2string(Token::INTCON) + " " + intConst + "\n";
    }

    std::string className() override {
        return "<IntConst>\n";
    }
};

class MFormatString : public MSyntaxNode {
private:
    std::string formatString;
public:
    explicit MFormatString(std::string formatString) : formatString(std::move(formatString)) {}

    std::string toString() override {
        return token2string(Token::STRCON) + " " + formatString + "\n";
    }

    std::string className() override {
        return "<FormatString>\n";
    }
};

#endif //BUAA_COMPILER_SYNTAX_NODES_H
