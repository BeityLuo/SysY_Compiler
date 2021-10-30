#include "syntax_nodes.h"


std::string MCompUnit::toString() {
    std::string ans;
    for (MDecl *decl : *(self.decls)) {
        ans += decl->toString();
    }
    for (MFuncDef *funcDef : *(self.funcs)) {
        ans += funcDef->toString();
    }
    ans += self.mainFuncDef->toString() + self.className();
    return ans;
}


std::string MConstDecl::toString() {
    std::string ans = fixedToken2PairString(Token::CONSTTK)
                      + self.type->toString();

    for (auto ite = self.defs->begin(); ite != self.defs->end(); ++ite) {
        ans += (*ite)->toString();
        if (ite != self.defs->end() - 1) {
            ans += fixedToken2PairString(Token::COMMA);
        }
    }
    return ans + fixedToken2PairString(Token::SEMICN) + self.className();
}


std::string MVarDecl::toString() {
    std::string ans = self.type->toString();

    for (auto ite = self.defs->begin(); ite != self.defs->end(); ++ite) {
        ans += (*ite)->toString();
        if (ite != self.defs->end() - 1) {
            ans += fixedToken2PairString(Token::COMMA);
        }
    }
    return ans + fixedToken2PairString(Token::SEMICN) + self.className();
}


std::string MConstDef::toString() {
    std::string ans = self.ident->toString();
    for (auto constExp : *(self.constExps)) {
        ans += fixedToken2PairString(Token::LBRACK)
               + constExp->toString()
               + fixedToken2PairString(Token::RBRACK);
    }
    ans += fixedToken2PairString(Token::ASSIGN) + self.init->toString()
           + self.className();
    return ans;
}


std::string MVarDef::toString() {
    std::string ans = self.ident->toString();
    for (auto constExp : *(self.constExps)) {
        ans += fixedToken2PairString(Token::LBRACK)
               + constExp->toString()
               + fixedToken2PairString(Token::RBRACK);
    }
    if (self.init != nullptr)
        ans += fixedToken2PairString(Token::ASSIGN) + self.init->toString();
    ans += self.className();
    return ans;
}


std::string MConstExpConstInitVal::toString() {
    return self.constExp->toString() + self.className();
}


std::string MArrayConstInitVal::toString() {
    std::string ans = fixedToken2PairString(Token::LBRACE);
    for (auto ite = self.constInitVals->begin(); ite != self.constInitVals->end(); ++ite) {
        if (ite != self.constInitVals->end() - 1)
            ans += (*ite)->toString() + fixedToken2PairString(Token::COMMA);
        else
            ans += (*ite)->toString();
    }
    ans += fixedToken2PairString(Token::RBRACE) + self.className();
    return ans;
}

std::string MExpInitVal::toString() {
    return self.exp->toString() + self.className();
}

std::string MArrayInitVal::toString() {
    std::string ans = fixedToken2PairString(Token::LBRACE);
    for (auto ite = self.initVals->begin(); ite != self.initVals->end(); ++ite) {
        if (ite != self.initVals->end() - 1)
            ans += (*ite)->toString() + fixedToken2PairString(Token::COMMA);
        else
            ans += (*ite)->toString();
    }
    ans += fixedToken2PairString(Token::RBRACE) + self.className();
    return ans;
}

std::string MFuncDef::toString() {
    std::string ans = self.funcType->toString() + self.ident->toString()
                      + fixedToken2PairString(Token::LPARENT);
    if (self.funcFParams != nullptr)
        ans += self.funcFParams->toString();
    ans += fixedToken2PairString(Token::RPARENT) + self.block->toString()
           + self.className();
    return ans;
}

std::string MMainFuncDef::toString() {
    return fixedToken2PairString(Token::INTTK)
           + fixedToken2PairString(Token::MAINTK)
           + fixedToken2PairString(Token::LPARENT)
           + fixedToken2PairString(Token::RPARENT)
           + self.block->toString() + self.className();
}

std::string MFuncType::toString() {
    return fixedToken2PairString(self.type) + self.className();
}

std::string MFuncFParams::toString() {
    std::string ans;
    for (auto ite = self.funcFParams->begin(); ite != self.funcFParams->end(); ++ite) {
        if (ite != self.funcFParams->end() - 1)
            ans += (*ite)->toString() + fixedToken2PairString(Token::COMMA);
        else
            ans += (*ite)->toString();
    }
    return ans + self.className();
}

std::string MFuncFParam::toString() {
    std::string ans = self.bType->toString() + self.ident->toString();
    if (self.constExps != nullptr) {
        ans += fixedToken2PairString(Token::LBRACK) + fixedToken2PairString(Token::RBRACK);
        for (auto constExp : *(self.constExps)) {
            ans += fixedToken2PairString(Token::LBRACK)
                   + constExp->toString()
                   + fixedToken2PairString(Token::RBRACK);
        }
    }
    ans += self.className();
    return ans;
}

std::string MBlock::toString() {
    std::string ans = fixedToken2PairString(Token::LBRACE);
    for (auto blockItem : *(self.blockItems)) {
        ans += blockItem->toString();
    }
    ans += fixedToken2PairString(Token::RBRACE) + self.className();
    return ans;
}

std::string MDeclBlockItem::toString() {
    return self.decl->toString();
}

std::string MStmtBlockItem::toString() {
    return self.stmt->toString();
}

std::string MLValStmt::toString() {
    std::string ans = self.lVal->toString() + fixedToken2PairString(Token::ASSIGN);
    if (self.exp != nullptr)
        ans += self.exp->toString();
    else
        ans += fixedToken2PairString(Token::GETINTTK)
               + fixedToken2PairString(Token::LPARENT)
               + fixedToken2PairString(Token::RPARENT);

    ans += fixedToken2PairString(Token::SEMICN) + self.className();
    return ans;
}

std::string MExpStmt::toString() {
    return self.exp->toString() + fixedToken2PairString(Token::SEMICN)
           + self.className();
}

std::string MNullStmt::toString() {
    return fixedToken2PairString(Token::SEMICN) + self.className();
}

std::string MBlockStmt::toString() {
    return self.block->toString() + self.className();
}

std::string MIfStmt::toString() {
    std::string ans = fixedToken2PairString(Token::IFTK) + fixedToken2PairString(Token::LPARENT)
                      + self.cond->toString() + fixedToken2PairString(Token::RPARENT)
                      + self.ifStmt->toString();
    if (self.elseStmt != nullptr)
        ans += fixedToken2PairString(Token::ELSETK) + self.elseStmt->toString();
    ans += self.className();
    return ans;
}

std::string MWhileStmt::toString() {
    return fixedToken2PairString(Token::WHILETK) + fixedToken2PairString(Token::LPARENT)
           + self.cond->toString() + fixedToken2PairString(Token::RPARENT)
           + self.stmt->toString() + self.className();
}

std::string MBreakStmt::toString() {
    return fixedToken2PairString(Token::BREAKTK) + fixedToken2PairString(Token::SEMICN)
           + self.className();
}

std::string MContinueStmt::toString() {
    return fixedToken2PairString(Token::CONTINUETK) + fixedToken2PairString(Token::SEMICN)
           + self.className();
}

std::string MReturnStmt::toString() {
    std::string ans = fixedToken2PairString(Token::RETURNTK);
    if (self.exp != nullptr)
        ans += exp->toString();
    ans += fixedToken2PairString(Token::SEMICN) + self.className();
    return ans;
}

std::string MPrintfStmt::toString() {
    std::string ans = fixedToken2PairString(Token::PRINTFTK)
                      + fixedToken2PairString(Token::LPARENT)
                      + self.formatString->toString();
    for (auto exp : *(self.exps)) {
        ans += fixedToken2PairString(Token::COMMA) + exp->toString();
    }
    ans += fixedToken2PairString(Token::RPARENT) + fixedToken2PairString(Token::SEMICN)
           + self.className();
    return ans;
}

std::string MExp::toString() {
    return self.addExp->toString() + self.className();
}

std::string MConstExp::toString() {
    return self.addExp->toString() + self.className();
}

std::string MCond::toString() {
    return self.lOrExp->toString() + self.className();
}

std::string MLVal::toString() {
    std::string ans = self.ident->toString();
    for (auto exp : *(self.exps)) {
        ans += fixedToken2PairString(Token::LBRACK) + exp->toString()
               + fixedToken2PairString(Token::RBRACK);
    }
    ans += self.className();
    return ans;
}

std::string MExpPrimaryExp::toString() {
    return fixedToken2PairString(Token::LPARENT) + self.exp->toString()
           + fixedToken2PairString(Token::RPARENT) + self.className();
}

std::string MLValPrimaryExp::toString() {
    return self.lVal->toString() + self.className();
}

std::string MNumberPrimaryExp::toString() {
    return self.number->toString() + self.className();
}

std::string MNumber::toString() {
    return self.intConst->toString() + self.className();
}

std::string MPrimaryExpUnaryExp::toString() {
    return self.primaryExp->toString() + self.className();
}

std::string MFuncUnaryExp::toString() {
    std::string ans = self.ident->toString() + fixedToken2PairString(Token::LPARENT);
    if (self.funcRParams != nullptr)
        ans += self.funcRParams->toString();
    ans += fixedToken2PairString(Token::RPARENT) + self.className();
    return ans;
}

std::string MUnaryExpUnaryExp::toString() {
    return self.unaryOp->toString() + self.unaryExp->toString()
           + self.className();
}

std::string MUnaryOp::toString() {
    return fixedToken2PairString(self.op) + self.className();
}

std::string MFuncRParams::toString() {
    std::string ans;
    for (auto ite = self.exps->begin(); ite != self.exps->end(); ++ite) {
        if (ite == self.exps->end() - 1)
            ans += (*ite)->toString();
        else
            ans += (*ite)->toString() + fixedToken2PairString(Token::COMMA);
    }
    return ans + self.className();
}

std::string MMulExp::toString() {
    std::string ans;
    auto expIte = self.unaryExps->begin();
    auto opIte = self.ops->begin();
    ans += (*expIte)->toString();
    ++expIte;
    for (; expIte != self.unaryExps->end(); ++expIte, ++opIte) {
        ans += self.className() + fixedToken2PairString(*opIte)
               + (*expIte)->toString();

    }
    return ans + self.className();
}

std::string MAddExp::toString() {
    std::string ans;
    auto expIte = self.mulExps->begin();
    auto opIte = self.ops->begin();
    ans += (*expIte)->toString();
    ++expIte;
    for (; expIte != self.mulExps->end(); ++expIte, ++opIte) {
        ans += self.className() + fixedToken2PairString(*opIte)
               + (*expIte)->toString();

    }
    return ans + self.className();
}

std::string MRelExp::toString() {
    std::string ans;
    auto expIte = self.addExps->begin();
    auto opIte = self.ops->begin();
    ans += (*expIte)->toString();
    ++expIte;
    for (; expIte != self.addExps->end(); ++expIte, ++opIte) {
        ans += self.className() + fixedToken2PairString(*opIte)
               + (*expIte)->toString();

    }
    return ans + self.className();
}

std::string MEqExp::toString() {
    std::string ans;
    auto expIte = self.relExps->begin();
    auto opIte = self.ops->begin();
    ans += (*expIte)->toString();
    ++expIte;
    for (; expIte != self.relExps->end(); ++expIte, ++opIte) {
        ans += self.className() + fixedToken2PairString(*opIte)
               + (*expIte)->toString();

    }
    return ans + self.className();
}

std::string MLAndExp::toString() {
    std::string ans;
    auto expIte = self.eqExps->begin();
    auto opIte = self.ops->begin();
    ans += (*expIte)->toString();
    ++expIte;
    for (; expIte != self.eqExps->end(); ++expIte, ++opIte) {
        ans += self.className() + fixedToken2PairString(*opIte)
               + (*expIte)->toString();

    }
    return ans + self.className();
}

std::string MLOrExp::toString() {
    std::string ans;
    auto expIte = self.lAndExps->begin();
    auto opIte = self.ops->begin();
    ans += (*expIte)->toString();
    ++expIte;
    for (; expIte != self.lAndExps->end(); ++expIte, ++opIte) {
        ans += self.className() + fixedToken2PairString(*opIte)
               + (*expIte)->toString();

    }
    return ans + self.className();
}


