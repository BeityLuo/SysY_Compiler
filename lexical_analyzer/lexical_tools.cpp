#include "lexical_tools.h"
std::set<char> space_set = {' ', '\t', '\n', '\r'};

std::string token2string(Token token) {
    switch (token){
        case Token::IDENFR: return "IDENFR";
        case Token::INTCON: return "INTCON";
        case Token::STRCON: return "STRCON";
        case Token::MAINTK: return "MAINTK";
        case Token::CONSTTK: return "CONSTTK";
        case Token::INTTK: return "INTTK";
        case Token::BREAKTK: return "BREAKTK";
        case Token::CONTINUETK: return "CONTINUETK";
        case Token::IFTK: return "IFTK";
        case Token::ELSETK: return "ELSETK";
        case Token::NOT: return "NOT";
        case Token::AND: return "AND";
        case Token::OR: return "OR";
        case Token::WHILETK: return "WHILETK";
        case Token::GETINTTK: return "GETINTTK";
        case Token::PRINTFTK: return "PRINTFTK";
        case Token::RETURNTK: return "RETURNTK";
        case Token::PLUS: return "PLUS";
        case Token::MINU: return "MINU";
        case Token::VOIDTK: return "VOIDTK";
        case Token::MULT: return "MULT";
        case Token::DIV: return "DIV";
        case Token::MOD: return "MOD";
        case Token::LSS: return "LSS";
        case Token::LEQ: return "LEQ";
        case Token::GRE: return "GRE";
        case Token::GEQ: return "GEQ";
        case Token::EQL: return "EQL";
        case Token::NEQ: return "NEQ";
        case Token::ASSIGN: return "ASSIGN";
        case Token::SEMICN: return "SEMICN";
        case Token::COMMA: return "COMMA";
        case Token::LPARENT: return "LPARENT";
        case Token::RPARENT: return "RPARENT";
        case Token::LBRACK: return "LBRACK";
        case Token::RBRACK: return "RBRACK";
        case Token::LBRACE: return "LBRACE";
        case Token::RBRACE: return "RBRACE";
    }
    return ""; //一点用都没有；
}

std::string token2cor_string(Token _token) {
    // 返回这个token对应的字符串，如MAINTK对应main。
    switch (_token) {
        case(Token::MAINTK): return "main";
        case(Token::CONSTTK): return "const";
        case(Token::INTTK): return "int";
        case(Token::BREAKTK): return "break";
        case(Token::CONTINUETK): return "continue";
        case(Token::IFTK): return "if";
        case(Token::ELSETK): return "else";
        case(Token::NOT): return "!";
        case(Token::AND): return "&&";
        case(Token::OR): return "||";
        case(Token::WHILETK): return "while";
        case(Token::GETINTTK): return "getint";
        case(Token::PRINTFTK): return "printf";
        case(Token::RETURNTK): return "return";
        case(Token::PLUS): return "+";
        case(Token::MINU): return "-";
        case(Token::VOIDTK): return "void";
        case(Token::MULT): return "*";
        case(Token::DIV): return "/";
        case(Token::MOD): return "%";
        case(Token::LSS): return "<";
        case(Token::LEQ): return "<=";
        case(Token::GRE): return ">";
        case(Token::GEQ): return ">=";
        case(Token::EQL): return "==";
        case(Token::NEQ): return "!=";
        case(Token::ASSIGN): return "=";
        case(Token::SEMICN): return ";";
        case(Token::COMMA): return ",";
        case(Token::LPARENT): return "(";
        case(Token::RPARENT): return ")";
        case(Token::LBRACK): return "[";
        case(Token::RBRACK): return "]";
        case(Token::LBRACE): return "{";
        case(Token::RBRACE): return "}";
        default: throw TokenNotMatchFixedStringException("token: Token::" + token2string(_token) +
            " does not match a fixed corresponding string.");
    }
}


std::string TokenLexemePair2string(TokenLexemePair* pair) {
    std::string ans = "";
    return token2string(pair->token) + " " + pair->lexeme;
}

std::string fixedToken2PairString(Token token) {
    return token2string(token) + " " + token2cor_string(token) + "\n";
}


bool is_space(char c) {
    return space_set.count(c) == 1;
}
bool is_digit_or_nodigit(char c) {
    return is_digit(c) || is_nodigit(c);
}
bool is_digit(char c) {
    return c >= '0' && c <= '9';
}
bool is_nodigit(char c) {
    return c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

std::string clear_comments(std::string code) {
    std::string ans = "";
    char c;
    int index = 0;
    int len = code.length();
    for (; index < len; index++) {
        c = code[index];
        // "//" is prior than "/*"
        if (c == '"') {
            // 字符串的优先级最高
            ans += code[index]; // 先把'"'加进去
            index++;
            for (; index < len && code[index] != '"'; index++) {
                ans += code[index];
            }
            if (index != len) {
                ans += code[index];
            }

        } else if (c == '/' && index != len - 1 && code[index + 1] == '/') {
            // 行注释的优先级第二
            index += 2;
            for(; index != len && code[index] != '\n'; index++);
            ans += '\n';
        } else if (c == '/' && index != len - 1 && code[index + 1] == '*') {
            // 块注释的优先级最低
            index += 2; // 跳过"/*"
            int lineNum = 0;
            for(; index != len - 1 && !(code[index] == '*' && code[index + 1] == '/'); index++) {
                if (code[index] == '\n')
                    lineNum++;
            }
            if (index == len - 1 && !(code[index] == '*' && code[index + 1] == '/')) {
                // 直到文件结尾也没找到"*/"，就抛出异常
                throw RightCommentSignNotFoundException("We dont found a \"*/\" till the end of file");
            }
            index += 1; //index处于"*/"的'*', 把'/'跳过去
            for(int i = 0; i < lineNum; i++) {
                ans += '\n';
            }
        } else {
            ans += c;
        }

    }
    return ans;
}
