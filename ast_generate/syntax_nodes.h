#include <utility>


#ifndef BUAA_COMPILER_SYNTAX_NODES_H
#define BUAA_COMPILER_SYNTAX_NODES_H

#include <vector>
#include <string>
#include "syntax_nodes_classes.h"
#include "../lexical_analyzer/lexical_tools.h"

#define self (*this)

template<class T>
void pushVector(std::vector<T> *v1, std::vector<T> *v2) {
    for (auto vv: *v2) {
        v1->push_back(vv);
    }
}

enum MSyntaxNodeTypeID{
    IDENT, COMP_UNIT, B_TYPE, FUNC_TYPE, CONST_EXP_CONST_INITVAL,
    ARRAY_CONST_INITVAL, EXP_INITVAL, ARRAY_INITVAL, CONST_DEF,
    VAR_DEF, CONST_DECL, VAR_DECL, FUNC_F_PARAMS, FUNC_F_PARAM,
    FUNC_DEF, MAIN_FUNC_DEF, BLOCK, DECL_BLOCK_ITEM, STMT_BLOCK_ITEM,
    LVAL_STMT, EXP_STMT, NULL_STMT, IF_STMT, WHILE_STMT, BLOCK_STMT,
    BREAK_STMT, CONTINUE_STMT, RETURN_STMT, PRINTF_STMT, EXP, CONST_EXP,
    COND, LVAL, EXP_PRIMARY_EXP, LVAL_PRIMARY_EXP, NUMBER_PRIMARY_EXP,
    NUMBER, PRIMARY_EXP_UNARY_EXP, FUNC_UNARY_EXP, UNARY_EXP_UNARY_EXP,
    UNARY_OP, FUNC_R_PARAMS, MUL_EXP, ADD_EXP, REL_EXP, EQ_EXP, LAND_EXP,
    LOR_EXP, INT_CONST, FORMAT_STRING


};

class MSyntaxNode {
public:
    virtual std::string toString() = 0; // 全部都带有后缀的'\n'
    virtual std::string className() = 0;
    virtual int getTypeID()  = 0;
};

class MIdent : public MSyntaxNode {
public:
    std::string name;
    int lineNum;
public:
    explicit MIdent(std::string name, int lineNum) : name(std::move(name)), lineNum(lineNum) {}

    std::string toString() override {
        return token2string(Token::IDENFR) + " " + self.name + "\n";
    }

    std::string className() override {
        return "<Ident>\n";
    }

    int getTypeID() override {
        return MSyntaxNodeTypeID::IDENT;
    }
};

class MCompUnit : public MSyntaxNode {
public:
    std::vector<MDecl *> *decls;
    std::vector<MFuncDef *> *funcs;
    MMainFuncDef *mainFuncDef;
public:
    MCompUnit(std::vector<MDecl *> *decls, std::vector<MFuncDef *> *funcs, MMainFuncDef *mainFuncDef)
            : mainFuncDef(mainFuncDef), decls(new std::vector<MDecl *>()), funcs(new std::vector<MFuncDef *>()) {
        pushVector(self.decls, decls);
        pushVector(self.funcs, funcs);
    }

    std::string toString() override;

    std::string className() override {
        return "<CompUnit>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::COMP_UNIT;
    }
};

class MType : MSyntaxNode {
public:
    Token type;
public:
    MType(Token type) : type(type) {}
};


class MBType : public MType {
    // 不输出自己
public:
    MBType() : MType(Token::INTTK) {}

    std::string className() override {
        return "<BType>\n"; //虽然用不到还是要实现这个方法，否则不能实例化
    }

    std::string toString() override {
        return fixedToken2PairString(Token::INTTK);
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::B_TYPE;
    }
};

class MFuncType : public MType {
public:
    explicit MFuncType(Token token) : MType(token) {
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
    int getTypeID() override {
        return MSyntaxNodeTypeID::FUNC_TYPE;
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
public:
    MConstExp *constExp;
public:
    explicit MConstExpConstInitVal(MConstExp *constExp)
            : constExp(constExp){}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::CONST_EXP_CONST_INITVAL;
    }
};

class MArrayConstInitVal : public MConstInitVal {
public:
    std::vector<MConstInitVal *> *constInitVals;
public:
    MArrayConstInitVal(std::vector<MConstInitVal *> *constInitVals) :
            constInitVals(new std::vector<MConstInitVal *>()) {
        pushVector(self.constInitVals, constInitVals);
    }

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::ARRAY_CONST_INITVAL;
    }
};

class MInitVal : public MVariableInit {
public:
    std::string className() override {
        return "<InitVal>\n";
    }
};

class MExpInitVal : public MInitVal {
public:
    MExp *exp;
public:
    explicit MExpInitVal(MExp *exp) : exp(exp) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::EXP_INITVAL;
    }
};

class MArrayInitVal : public MInitVal {
public:
    std::vector<MInitVal *> *initVals;
public:
    explicit MArrayInitVal(std::vector<MInitVal *> *initVals) : initVals(new std::vector<MInitVal *>()) {
        pushVector(self.initVals, initVals);
    }

    std::string toString() override;

    int getTypeID() override {
        return MSyntaxNodeTypeID::ARRAY_INITVAL;
    }
};

class MDef : public MSyntaxNode {
public:
    MIdent *ident;
    std::vector<int> *dims;
    MVariableInit *init;
public:
    MDef(MIdent *ident, std::vector<int> *dims, MVariableInit *init)
            : ident(ident), init(init), dims(new std::vector<int>()) {
        pushVector(self.dims, dims);
    }

    MDef() : ident(nullptr), dims(new std::vector<int>()), init(nullptr) {}
};

class MConstDef : public MDef {

public:
    MConstDef(MIdent *ident, std::vector<int> *dims, MConstInitVal *constInitVal)
            : MDef(ident, dims, constInitVal) {
        self.ident = ident;
        self.init = constInitVal;
    }

    std::string className() override {
        return "<ConstDef>\n";
    }

    std::string toString() override;

    int getTypeID() override {
        return MSyntaxNodeTypeID::CONST_DEF;
    }

};

class MVarDef : public MDef {
public:
    MVarDef(MIdent *ident, std::vector<int> *dims, MInitVal *initVal = nullptr)
            : MDef(ident, dims, initVal) {
        self.ident = ident;
        self.init = initVal;
    }

    std::string toString() override;

    std::string className() override {
        return "<VarDef>\n";
    }

    int getTypeID() override {
        return MSyntaxNodeTypeID::VAR_DEF;
    }

};

class MDecl : public MSyntaxNode {
public:
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
        for (auto constDef: *constDefs) {
            self.defs->push_back(constDef);
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<ConstDecl>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::CONST_DECL;
    }

};

class MVarDecl : public MDecl {
public:

    MVarDecl(MBType *type, std::vector<MVarDef *> *varDefs) {
        self.type = type;
        for (auto varDef: *varDefs) {
            self.defs->push_back(varDef);
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<VarDecl>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::VAR_DECL;
    }
};

class MFuncFParams : public MSyntaxNode {
public:
    std::vector<MFuncFParam *> *funcFParams;
public:
    explicit MFuncFParams(std::vector<MFuncFParam *> *funcFParams)
        : funcFParams(new std::vector<MFuncFParam *>()) {
        if (funcFParams != nullptr)
            pushVector(self.funcFParams, funcFParams);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncFParams>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::FUNC_F_PARAMS;
    }
};

class MFuncFParam : public MSyntaxNode {
public:
    MBType *bType;
    MIdent *ident;
    std::vector<int> *dims;
    bool isArray;
public:
    MFuncFParam(MBType *bType, MIdent *ident, std::vector<int> *dims,
                bool isArray = false) :
            bType(bType), ident(ident), dims(new std::vector<int>()), isArray(isArray) {
        if (dims != nullptr)
            pushVector(self.dims, dims);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncFParam>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::FUNC_F_PARAM;
    }
};

class MFuncDef : public MSyntaxNode {
public:
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
    int getTypeID() override {
        return MSyntaxNodeTypeID::FUNC_DEF;
    }
};

class MMainFuncDef : public MFuncDef {
public:
public:
    explicit MMainFuncDef(MBlock *block, int lineNum) :
            MFuncDef(new MFuncType(Token::INTTK), new MIdent("main", lineNum),
                     new MFuncFParams(new std::vector<MFuncFParam *>()), block) {}

    std::string toString() override;

    std::string className() override {
        return "<MainFuncDef>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::MAIN_FUNC_DEF;
    }
};


class MBlock : public MSyntaxNode {
public:
    std::vector<MBlockItem *> *blockItems;
    int lineNum;
public:

    MBlock(std::vector<MBlockItem *> *blockItems, int lineNum)
            : blockItems(new std::vector<MBlockItem *>()), lineNum(lineNum) {
        if (blockItems != nullptr)
            pushVector(self.blockItems, blockItems);
    }

    std::string toString() override;

    std::string className() override {
        return "<Block>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::BLOCK;
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
public:
    MDecl *decl;
public:
    explicit MDeclBlockItem(MDecl *decl) : decl(decl) {}

    std::string toString() override;

    int getTypeID() override {
        return MSyntaxNodeTypeID::DECL_BLOCK_ITEM;
    }
};

class MStmtBlockItem : public MBlockItem {
public:
    MStmt *stmt;
public:
    explicit MStmtBlockItem(MStmt *stmt) : stmt(stmt) {}

    std::string toString() override;

    int getTypeID() override {
        return MSyntaxNodeTypeID::STMT_BLOCK_ITEM;
    }
};

class MStmt : public MSyntaxNode {
public:
    std::string className() override {
        return "<Stmt>\n";
    }
};

class MLValStmt : public MStmt {
public:
    MLVal *lVal;
    MExp *exp;
public:
    explicit MLValStmt(MLVal *lVal) : lVal(lVal), exp(nullptr) {}

    MLValStmt(MLVal *lVal, MExp *exp) : lVal(lVal), exp(exp) {}

    std::string toString() override;

    int getTypeID() override {
        return MSyntaxNodeTypeID::LVAL_STMT;
    }
};

class MExpStmt : public MStmt {
public:
    MExp *exp;
public:
    explicit MExpStmt(MExp *exp) : exp(exp) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::EXP_STMT;
    }
};

class MNullStmt : public MStmt {
public:
    MNullStmt() = default;

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::NULL_STMT;
    }
};

class MBlockStmt : public MStmt {
public:
    MBlock *block;
public:
    explicit MBlockStmt(MBlock *block) : block(block) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::BLOCK_STMT;
    }
};

class MIfStmt : public MStmt {
public:
    MCond *cond;
    MStmt *ifStmt;
    MStmt *elseStmt;
public:
    MIfStmt(MCond *cond, MStmt *ifStmt, MStmt *elseStmt) : cond(cond), ifStmt(ifStmt), elseStmt(elseStmt) {}

    MIfStmt(MCond *cond, MStmt *ifStmt) : cond(cond), ifStmt(ifStmt), elseStmt(nullptr) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::IF_STMT;
    }
};

class MWhileStmt : public MStmt {
public:
    MCond *cond;
    MStmt *stmt;
public:
    MWhileStmt(MCond *cond, MStmt *stmt) : cond(cond), stmt(stmt) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::WHILE_STMT;
    }
};

class MBreakStmt : public MStmt {
public:
    int lineNum;
public:
    MBreakStmt(int lineNum) : lineNum(lineNum) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::BREAK_STMT;
    }
};

class MContinueStmt : public MStmt {
public:
    int lineNum;
public:
    MContinueStmt(int lineNum) : lineNum(lineNum) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::CONTINUE_STMT;
    }
};

class MReturnStmt : public MStmt {
public:
    MExp *exp;
    int lineNum;
public:
    explicit MReturnStmt(MExp *exp, int lineNum) : exp(exp), lineNum(lineNum) {}

    MReturnStmt() : exp(nullptr) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::RETURN_STMT;
    }
};

class MPrintfStmt : public MStmt {
public:
    MFormatString *formatString;
    std::vector<MExp *> *exps;
    int lineNum;
public:
    MPrintfStmt(MFormatString *formatString, std::vector<MExp *> *exps, int lineNum)
            : formatString(formatString), exps(new std::vector<MExp *>()), lineNum(lineNum) {
        if (exps != nullptr)
            pushVector(self.exps, exps);
    }

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::PRINTF_STMT;
    }
};

class MExp : public MSyntaxNode {
public:
    MAddExp *addExp;
public:
    explicit MExp(MAddExp *addExp) : addExp(addExp) {}

    std::string toString() override;

    std::string className() override {
        return "<Exp>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::EXP;
    }


};

class MConstExp : public MSyntaxNode {
public:
    MAddExp *addExp;
public:
    explicit MConstExp(MAddExp *addExp) : addExp(addExp) {}

    std::string toString() override;

    std::string className() override {
        return "<ConstExp>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::CONST_EXP;
    }
};


class MCond : public MSyntaxNode {
public:
    MLOrExp *lOrExp;
public:
    explicit MCond(MLOrExp *lOrExp) : lOrExp(lOrExp) {}

    std::string toString() override;

    std::string className() override {
        return "<Cond>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::COND;
    }
};

class MLVal : public MSyntaxNode {
public:
    MIdent *ident;
    std::vector<MExp *> *exps;
public:
    MLVal(MIdent *ident, std::vector<MExp *> *exps)
            : ident(ident), exps(new std::vector<MExp *>()) {
        if (exps != nullptr)
            pushVector(self.exps, exps);
    }

    std::string toString() override;

    std::string className() override {
        return "<LVal>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::LVAL;
    }
};

class MPrimaryExp : public MSyntaxNode {
public:
    std::string className() override {
        return "<PrimaryExp>\n";
    }
};

class MExpPrimaryExp : public MPrimaryExp {
public:
    MExp *exp;
public:
    explicit MExpPrimaryExp(MExp *exp) : exp(exp) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::EXP_PRIMARY_EXP;
    }
};

class MLValPrimaryExp : public MPrimaryExp {
public:
    MLVal *lVal;
public:
    explicit MLValPrimaryExp(MLVal *lVal) : lVal(lVal) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::LVAL_PRIMARY_EXP;
    }
};

class MNumberPrimaryExp : public MPrimaryExp {
public:
    MNumber *number;
public:
    explicit MNumberPrimaryExp(MNumber *number) : number(number) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::NUMBER_PRIMARY_EXP;
    }
};

class MNumber : public MSyntaxNode {
public:
    MIntConst *intConst;
public:
    explicit MNumber(MIntConst *intConst) : intConst(intConst) {}

    std::string toString() override;

    std::string className() override {
        return "<Number>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::NUMBER;
    }
};

class MUnaryExp : public MSyntaxNode {
public:
    std::string className() override {
        return "<UnaryExp>\n";
    }

};

class MPrimaryExpUnaryExp : public MUnaryExp {
public:
    MPrimaryExp *primaryExp;
public:
    explicit MPrimaryExpUnaryExp(MPrimaryExp *primaryExp) : primaryExp(primaryExp) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::PRIMARY_EXP_UNARY_EXP;
    }
};

class MFuncUnaryExp : public MUnaryExp {
public:
    MIdent *ident;
    MFuncRParams *funcRParams;
public:
    MFuncUnaryExp(MIdent *ident, MFuncRParams *funcRParams)
            : ident(ident), funcRParams(funcRParams) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::FUNC_UNARY_EXP;
    }
};

class MUnaryExpUnaryExp : public MUnaryExp {
public:
    MUnaryOp *unaryOp;
    MUnaryExp *unaryExp;
public:
    MUnaryExpUnaryExp(MUnaryOp *unaryOp, MUnaryExp *unaryExp)
            : unaryOp(unaryOp), unaryExp(unaryExp) {}

    std::string toString() override;
    int getTypeID() override {
        return MSyntaxNodeTypeID::UNARY_EXP_UNARY_EXP;
    }
};

class MUnaryOp : public MSyntaxNode {
public:
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
    int getTypeID() override {
        return MSyntaxNodeTypeID::UNARY_OP;
    }
};

class MFuncRParams : public MSyntaxNode {
public:
    std::vector<MExp *> *exps;
public:
    explicit MFuncRParams(std::vector<MExp *> *exps) : exps(new std::vector<MExp *>()) {
        pushVector(self.exps, exps);
    }

    std::string toString() override;

    std::string className() override {
        return "<FuncRParams>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::FUNC_R_PARAMS;
    }
};

class MMulExp : public MSyntaxNode {
public:
    std::vector<MUnaryExp *> *unaryExps;
    std::vector<Token> *ops;
public:
    MMulExp(std::vector<MUnaryExp *> *unaryExps, std::vector<Token> *ops)
            : unaryExps(new std::vector<MUnaryExp *>()), ops(new std::vector<Token>()) {

        if (unaryExps->size() != ops->size() + 1) {
            throw "In MMulExp's constructor: size of unaryExps ans ops not match";
        }
        pushVector(self.unaryExps, unaryExps);
        pushVector(self.ops, ops);
        for (Token op: *(self.ops)) {
            if (op != Token::MULT && op != Token::DIV && op != Token::MOD) {
                throw "In MMulExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<MulExp>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::MUL_EXP;
    }
};

class MAddExp : public MSyntaxNode {
public:
    std::vector<MMulExp *> *mulExps;
    std::vector<Token> *ops;
public:
    MAddExp(std::vector<MMulExp *> *mulExps, std::vector<Token> *ops)
            : mulExps(new std::vector<MMulExp *>()), ops(new std::vector<Token>()) {


        if (mulExps->size() != ops->size() + 1) {
            throw "In MAddExp's constructor: size of mulExps ans ops not match";
        }
        pushVector(self.mulExps, mulExps);
        pushVector(self.ops, ops);
        for (Token op: *(self.ops)) {
            if (op != Token::PLUS && op != Token::MINU) {
                throw "In MAddExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<AddExp>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::ADD_EXP;
    }
};

class MRelExp : public MSyntaxNode {
public:
    std::vector<MAddExp *> *addExps;
    std::vector<Token> *ops;
public:
    MRelExp(std::vector<MAddExp *> *addExps, std::vector<Token> *ops)
            : addExps(new std::vector<MAddExp *>()), ops(new std::vector<Token>()) {
        if (addExps->size() != ops->size() + 1) {
            throw "In MAddExp's constructor: size of addExps ans ops not match";
        }
        pushVector(self.addExps, addExps);
        pushVector(self.ops, ops);
        for (Token op: *(self.ops)) {
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
    int getTypeID() override {
        return MSyntaxNodeTypeID::REL_EXP;
    }
};

class MEqExp : public MSyntaxNode {
public:
    std::vector<MRelExp *> *relExps;
    std::vector<Token> *ops;
public:
    MEqExp(std::vector<MRelExp *> *relExps, std::vector<Token> *ops)
            : relExps(new std::vector<MRelExp *>()), ops(new std::vector<Token>()) {

        if (relExps->size() != ops->size() + 1) {
            throw "In MEqExp's constructor: size of relExps ans ops not match";
        }
        pushVector(self.relExps, relExps);
        pushVector(self.ops, ops);
        for (Token op: *(self.ops)) {
            if (op != Token::EQL && op != Token::NEQ) {
                throw "In MEqExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<EqExp>\n";
    }

    int getTypeID() override {
        return MSyntaxNodeTypeID::EQ_EXP;
    }
};

class MLAndExp : public MSyntaxNode {
public:
    std::vector<MEqExp *> *eqExps;
    std::vector<Token> *ops;
public:
    MLAndExp(std::vector<MEqExp *> *eqExps, std::vector<Token> *ops)
            : eqExps(new std::vector<MEqExp *>()), ops(new std::vector<Token>()) {


        if (eqExps->size() != ops->size() + 1) {
            throw "In MLAndExp's constructor: size of eqExps ans ops not match";
        }
        pushVector(self.eqExps, eqExps);
        pushVector(self.ops, ops);
        for (Token op: *(self.ops)) {
            if (op != Token::AND) {
                throw "In MLAndExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<LAndExp>\n";
    }

    int getTypeID() override {
        return MSyntaxNodeTypeID::LAND_EXP;
    }
};


class MLOrExp : public MSyntaxNode {
public:
    std::vector<MLAndExp *> *lAndExps;
    std::vector<Token> *ops;
public:
    MLOrExp(std::vector<MLAndExp *> *lAndExps, std::vector<Token> *ops)
            : lAndExps(new std::vector<MLAndExp *>()), ops(new std::vector<Token>()) {
        if (lAndExps->size() != ops->size() + 1) {
            throw "In MLOrExp's constructor: size of lAndExps ans ops not match";
        }
        pushVector(self.lAndExps, lAndExps);
        pushVector(self.ops, ops);
        for (Token op: *(self.ops)) {
            if (op != Token::OR) {
                throw "In MLOrExp's constructor: wong para: op = " + token2string(op);
            }
        }
    }

    std::string toString() override;

    std::string className() override {
        return "<LOrExp>\n";
    }
    int getTypeID() override {
        return MSyntaxNodeTypeID::LOR_EXP;
    }
};


class MIntConst : public MSyntaxNode {
public:
    std::string intConst;
public:
    explicit MIntConst(std::string intConst) : intConst(std::move(intConst)) {}

    std::string toString() override {
        return token2string(Token::INTCON) + " " + intConst + "\n";
    }

    std::string className() override {
        return "<IntConst>\n";
    }

    int getTypeID() override {
        return MSyntaxNodeTypeID::INT_CONST;
    }
};

class MFormatString : public MSyntaxNode {
public:
    std::string formatString;
    int formatCharNum;
    bool isLegal;
public:
    explicit MFormatString(std::string formatString)
            : formatString(std::move(formatString)), formatCharNum(0) {
        // 只允许 ' ', '!', '%d', '\n'出现
        self.isLegal = true;
        char c;
        int len = self.formatString.size();
        if (len < 2 || self.formatString[0] != '"' ||
            self.formatString[len - 1] != '"') {
            // 两头都是双引号
            self.isLegal = false;
            return;
        }
        int i = 1;
        for (c = self.formatString[1]; i < len - 1; i++, c = self.formatString[i]) {
            if (c == '%') {
                // 判断%d
                i++;
                if (i == len) {
                    // 字符串末尾有%
                    self.isLegal = false;
                    i -= 2; // 让i回到%之前一个字符
                    self.formatString = self.formatString.substr(0, i + 1);
                    len = self.formatString.length();
                } else if (self.formatString[i] != 'd') {
                    // 把不符合规范的部分裁剪掉
                    self.isLegal = false;
                    i -= 2; // 让i回到%之前一个字符
                    self.formatString = self.formatString.substr(0, i + 1) + self.formatString.substr(i + 3, len - i - 3);
                    len = self.formatString.length();
                } else {
                    self.formatCharNum++;
                }
            } else if (c == '\\') {
                // 判断%\n
                i++;
                if (i == len) {
                    // 字符串末尾有%
                    self.isLegal = false;
                    i -= 2; // 让i回到'/'之前一个字符
                    self.formatString = self.formatString.substr(0, i + 1);
                    len = self.formatString.length();
                } else if (self.formatString[i] != 'n') {
                    // 把不符合规范的部分裁剪掉
                    self.isLegal = false;
                    i -= 2; // 让i回到'/'之前一个字符
                    self.formatString = self.formatString.substr(0, i + 1) + self.formatString.substr(i + 3, len - i - 3);
                    len = self.formatString.length();
                }
            } else if (!((c > 39 && c < 127) || c == 32 || c == 33)) {
                self.isLegal = false;
                i -= 1; // 让i回到非法符号之前一个字符
                self.formatString = self.formatString.substr(0, i + 1) + self.formatString.substr(i + 2, len - i - 2);;
                len = self.formatString.length();
            }
        }

    }

    std::string toString() override {
        return token2string(Token::STRCON) + " " + formatString + "\n";
    }

    std::string className() override {
        return "<FormatString>\n";
    }

    int getTypeID() override {
        return MSyntaxNodeTypeID::FORMAT_STRING;
    }

    bool checkLegal() {
        return self.isLegal;
    }

    int getFormatCharsNum() {
        return self.formatCharNum;
    }
};

#endif //BUAA_COMPILER_SYNTAX_NODES_H
