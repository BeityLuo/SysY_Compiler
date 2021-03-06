#ifndef BUAA_COMPILER_MEXPCALCULATOR_H
#define BUAA_COMPILER_MEXPCALCULATOR_H
#include "../ast_generate/syntax_nodes.h"
#include "MSymbol.h"
#include "MSymbolTable.h"

class MExpCalculator {
private:
    MSymbolTable* globalTable;
    MSymbolTable* currentTable;
public:
    int calculateConstExp(
            MConstExp* constExp, MSymbolTable* global, MSymbolTable* current) {

        globalTable = global;
        currentTable = current;
        return calculateConstAddMulExp(constExp->addExp);
    }

private:
    int calculateConstAddMulExp(MAddExp* addExp) {
        int expNum = addExp->mulExps->size();
        int ans = calculateConstMulExp((*(addExp->mulExps))[0]);
        for (int i = 1; i < expNum; i++) {
            int value = calculateConstMulExp((*(addExp->mulExps))[i]);
            switch ((*(addExp->ops))[i-1]) {
                case Token::PLUS: {
                    ans += value;
                    break;
                }
                case Token::MINU: {
                    ans -= value;
                    break;
                }
            }
        }
        return ans;
    }
    int calculateConstMulExp(MMulExp* mulExp) {
        int expNum = mulExp->unaryExps->size();
        int ans = calculateConstUnaryExp((*(mulExp->unaryExps))[0]);
        for (int i = 1; i < expNum; i++) {
            int value = calculateConstUnaryExp((*(mulExp->unaryExps))[i]);
            switch ((*(mulExp->ops))[i-1]) {
                case Token::MULT: {
                    ans *= value;
                    break;
                }
                case Token::DIV: {
                    ans /= value;
                    break;
                }
                case Token::MOD: {
                    ans %= value;
                }
            }
        }
        return ans;
    }

    int calculateConstUnaryExp(MUnaryExp* unaryExp) {
        if (unaryExp->getTypeID() == MSyntaxNodeTypeID::PRIMARY_EXP_UNARY_EXP) {
            return calculateConstPrimaryExp(((MPrimaryExpUnaryExp*)unaryExp)->primaryExp);
        } else if (unaryExp->getTypeID() == MSyntaxNodeTypeID::FUNC_UNARY_EXP) {
            throw "MExpCalculator::calculateConstUnaryExp: ????????????FuncUnaryExp";
        } else if (unaryExp->getTypeID() == MSyntaxNodeTypeID::UNARY_EXP_UNARY_EXP) {
            auto unaryExpUnaryExp = (MUnaryExpUnaryExp*)unaryExp;
            switch (unaryExpUnaryExp->unaryOp->op) {
                case Token::PLUS: {
                    return calculateConstUnaryExp(unaryExpUnaryExp->unaryExp);
                }
                case Token::MINU: {
                    return -calculateConstUnaryExp(unaryExpUnaryExp->unaryExp);
                }
                case Token::NOT: {
                    throw "MExpCalculator::calculateConstUnaryExp: Exp???????????????'!'";
                }
                default:
                    throw "MExpCalculator::calculateConstUnaryExp: ????????????????????????Token";
            }
        } else {
            throw "MExpCalculator::calculateConstUnaryExp: ????????????";
        }
    }

    int calculateConstPrimaryExp(MPrimaryExp* primaryExp) {
        if (primaryExp->getTypeID() == MSyntaxNodeTypeID::EXP_PRIMARY_EXP) {
            // ??????Exp???????????????????????????????????????????????????
            return calculateConstAddMulExp(((MExpPrimaryExp*)primaryExp)->exp->addExp);
        } else if (primaryExp->getTypeID() == MSyntaxNodeTypeID::LVAL_PRIMARY_EXP) {
            return getLValValue(((MLValPrimaryExp*)primaryExp)->lVal);
        } else if (primaryExp->getTypeID() == MSyntaxNodeTypeID::NUMBER_PRIMARY_EXP) {
            return std::stoi(((MNumberPrimaryExp*)primaryExp)
                ->number->intConst->intConst);
        } else {
            throw "MExpCalculator::calculateConstPrimaryExp: ???????????????MPrimaryExp?????????????????????????????????";
        }
    }


    int getLValValue(MLVal* lVal) {
        auto item = getVariableTableItem(lVal->ident->name);
        if (!item->variableType->isConst) {
            throw "MExpCalculator::getLValValue: lVal is not const";
        }
        if (!item->variableType->isArray) {
            return (*(item->initArray))[0];
        } else {
            // ???????????????????????????
            int offset = 0;
            int len = 1;
            if (lVal->exps->size() != item->variableType->dims->size()) {
                throw "MExpCalculator::getLValValue: ??????????????????????????????????????????";
            }
            for (int i = lVal->exps->size() - 1; i >= 0; i--) {
                // ?????????????????????????????????????????????????????????LVal???Exp??????ConstExp
                int subOff = calculateConstAddMulExp(
                        (*(lVal->exps))[i]->addExp);
                offset += subOff * len;
                len *= (*(item->variableType->dims))[i];
            }
            return (*(item->initArray))[offset];
        }
    }

    MVariableSymbolTableItem* getVariableTableItem(std::string& name) {

        for (auto table = currentTable; table != nullptr; table = table->getFatherScopeTable()) {
            if (table->contains(name)) {
                auto item = table->getTableItem(name);
                if (!item->isVariable) {
                    throw "MExpCalculator::MVariableSymbolTableItem: " + name + "???????????????";
                }
                return (MVariableSymbolTableItem *)item;
            }
        }
        throw "MExpCalculator::MVariableSymbolTableItem: undefined ident";
    }
};

#endif //BUAA_COMPILER_MEXPCALCULATOR_H
