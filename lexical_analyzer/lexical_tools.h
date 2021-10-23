#include <string>
#include <set>
#include "exceptions.h"
#ifndef COMPILER_LEXICAL_ANALYSIS_LEXICAL_TOOLS_H
#define COMPILER_LEXICAL_ANALYSIS_LEXICAL_TOOLS_H

enum class Token{
    IDENFR, INTCON, STRCON, MAINTK, CONSTTK, INTTK,
    BREAKTK, CONTINUETK, IFTK, ELSETK, NOT, AND, OR,
    WHILETK, GETINTTK, PRINTFTK, RETURNTK, PLUS, MINU,
    VOIDTK, MULT, DIV, MOD, LSS, LEQ, GRE, GEQ, EQL,
    NEQ, ASSIGN, SEMICN, COMMA, LPARENT, RPARENT,
    LBRACK, RBRACK, LBRACE, RBRACE
};
std::string token2cor_string(Token _token);
std::string token2string(Token token);

class TokenLexemePair{
public:
    Token token;
    std::string lexeme; // lexeme指的是源代码中与token匹配的一串字符
    int lineNum;
public:
    TokenLexemePair(Token token, std::string lexeme, int lineNum)
        : token(token), lexeme(lexeme), lineNum(lineNum) {}
    TokenLexemePair(Token _token, int lineNum) : lineNum(lineNum){
        self.token = _token;
        self.lexeme = token2cor_string(_token);
    }
};

std::string clear_comments(std::string code);


std::string TokenLexemePair2string(TokenLexemePair* pair);
std::string fixedToken2PairString(Token token);

extern std::set<char> space_set;

bool is_space(char c);
bool is_digit_or_nodigit(char c);
bool is_digit(char c);
bool is_nodigit(char c); //字母或下划线

#endif //COMPILER_LEXICAL_ANALYSIS_LEXICAL_TOOLS_H
