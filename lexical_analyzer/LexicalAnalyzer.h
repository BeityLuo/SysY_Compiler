#include "lexical_tools.h"
#include <vector>
#ifndef COMPILER_LEXICAL_ANALYSIS_LEXICALANALYZER_H
#define COMPILER_LEXICAL_ANALYSIS_LEXICALANALYZER_H
#define self (*this)
#define MAIN_LEN 4
#define CONST_LEN 5
#define CONTINUE_LEN 8
#define INT_LEN 3
#define IF_LEN 2
#define BREAK_LEN 5
#define ELSE_LEN 4
#define WHILE_LEN 5
#define GETINT_LEN 6
#define PRINTF_LEN 6
#define RETURN_LEN 6
#define VOID_LEN 4




class LexicalAnalyzer {
private:
    std::string source_code;
    int position; // current position
    int code_len;
    int lineNum;

public:
    LexicalAnalyzer(std::string& _source_code) : position(0) {
        // 末尾附上一个空格，就可以使得判断关键字不用考虑最后一个字符是关键字的一部分的情况。
        self.source_code = clear_comments(_source_code);
        self.code_len = self.source_code.length();
        self.lineNum = 1;
    };

    void analyze(std::vector<TokenLexemePair*>& token_list) {
        TokenLexemePair* pair;
        while((pair = self.next_token_pair()) != nullptr) {
            token_list.push_back(pair);
        }
    }

private:
    LexicalAnalyzer& ignore_space(){
        while (is_space(self.source_code[self.position]) && self.position < self.code_len) {
            // TODO 还要处理段注释里面的行号！！！！
            if(self.source_code[self.position] == '\n') {
                self.lineNum++;
            }
            self.position ++;
        }
        return *this;
    }

    char next_char() {
        // 如果已经到文件结尾了，就返回-1
        return self.position == self.code_len ? (char)-1 : self.source_code[self.position];
    }

    TokenLexemePair* next_token_pair() {
        char c = self.ignore_space().next_char(); // 小小的搞一下流式编程嘿嘿
        TokenLexemePair* pair;
        switch(c){
            case -1:
                // 说明没有新东西了
                return nullptr;
            // 这一部分判断保留字和运算符，优先级最高
            case 'm':
                if ((pair = self.getMAIN()) != nullptr) return pair;
                break;
            case 'c':
                if ((pair = self.getCONST()) != nullptr) return pair;
                if ((pair = self.getCONTINUE()) != nullptr) return pair;
                break;
            case 'i':
                if ((pair = self.getINT()) != nullptr) return pair;
                if ((pair = self.getIF()) != nullptr) return pair;
                break;
            case 'b':
                if ((pair = self.getBREAK()) != nullptr) return pair;
                break;
            case 'e':
                if ((pair = self.getELSE()) != nullptr) return pair;
                break;
            case 'w':
                if ((pair = self.getWHILE()) != nullptr) return pair;
                break;
            case 'g':
                if ((pair = self.getGETINT()) != nullptr) return pair;
                break;
            case 'p':
                if ((pair = self.getPRINTF()) != nullptr) return pair;
                break;
            case 'r':
                if ((pair = self.getRETURN()) != nullptr) return pair;
                break;
            case 'v':
                if ((pair = self.getVOID()) != nullptr) return pair;
                break;
            case '!':
                if ((pair = self.getNEQ()) != nullptr) return pair;
                if ((pair = self.getNOT()) != nullptr) return pair;
                break;
            case '&':
                if ((pair = self.getAND()) != nullptr) return pair;
                break;
            case '|':
                if ((pair = self.getOR()) != nullptr) return pair;
                break;
            case '+':
                if ((pair = self.getPLUS()) != nullptr) return pair;
                break;
            case '-':
                if ((pair = self.getMINU()) != nullptr) return pair;
                break;
            case '*':
                if ((pair = self.getMULT()) != nullptr) return pair;
                break;
            case '/':
                if ((pair = self.getDIV()) != nullptr) return pair;
                break;
            case '%':
                if ((pair = self.getMOD()) != nullptr) return pair;
                break;
            case '<':
                if ((pair = self.getLEQ()) != nullptr) return pair;
                if ((pair = self.getLSS()) != nullptr) return pair;
                break;
            case '>':
                if ((pair = self.getGEQ()) != nullptr) return pair;
                if ((pair = self.getGRE()) != nullptr) return pair;
                break;
            case '=':
                if ((pair = self.getEQL()) != nullptr) return pair;
                if ((pair = self.getASSIGN()) != nullptr) return pair;
                break;
            case ';':
                if ((pair = self.getSEMICN()) != nullptr) return pair;
                break;
            case ',':
                if ((pair = self.getCOMMA()) != nullptr) return pair;
                break;
            case '(':
                if ((pair = self.getLPARENT()) != nullptr) return pair;
                break;
            case ')':
                if ((pair = self.getRPARENT()) != nullptr) return pair;
                break;
            case '[':
                if ((pair = self.getLBRACK()) != nullptr) return pair;
                break;
            case ']':
                if ((pair = self.getRBRACK()) != nullptr) return pair;
                break;
            case '{':
                if ((pair = self.getLBRACE()) != nullptr) return pair;
                break;
            case '}':
                if ((pair = self.getRBRACE()) != nullptr) return pair;
                break;

        }
        // 如果能到这里，说明是Ident, IntConst, FormatString中的一种
        if (is_digit(c)) {
            if ((pair = self.getINTCON()) != nullptr) return pair;
            else throw LineException(self.lineNum, "getINTCONST failed");
        } else if (is_nodigit(c)) {
            if ((pair = self.getIDENFR()) != nullptr) return pair;
            else throw LineException(self.lineNum, "getIDENFR failed");
        } else if (c == '"') {
            if ((pair = self.getSTRCON()) != nullptr) return pair;
            else throw LineException(self.lineNum, "getSTRCON failed");
        } else {
            throw CharacterNotMatchAnythingException("Analyzer meet an unexpected character: " + std::string(1, c));
        }
    }
    bool start_with(std::string str) {
        int len = str.length();
        for (int i = 0; i < len; i++) {
            if (str[i] != self.source_code[self.position + i]) {
                return false;
            }
        }
        return true;
    }
    TokenLexemePair* getMAIN(){
        if (self.start_with(token2cor_string(Token::MAINTK)) &&
        !is_digit_or_nodigit(self.source_code[self.position + MAIN_LEN])) {
            // 如果以"main"开头且main后边不是nodigit(包括下划线)或数字，就表示匹配到了关键字“main”
            self.position += MAIN_LEN;
            return new TokenLexemePair(Token::MAINTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getCONST(){
        if (self.start_with(token2cor_string(Token::CONSTTK)) &&
        !is_digit_or_nodigit(self.source_code[self.position + CONST_LEN])) {
            self.position += CONST_LEN;
            return new TokenLexemePair(Token::CONSTTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getCONTINUE(){
        if (self.start_with(token2cor_string(Token::CONTINUETK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + CONTINUE_LEN])) {
            self.position += CONTINUE_LEN;
            return new TokenLexemePair(Token::CONTINUETK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getINT(){
        if (self.start_with(token2cor_string(Token::INTTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + INT_LEN])) {
            self.position += INT_LEN;
            return new TokenLexemePair(Token::INTTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getIF(){
        if (self.start_with(token2cor_string(Token::IFTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + IF_LEN])) {
            self.position += IF_LEN;
            return new TokenLexemePair(Token::IFTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getBREAK(){
        if (self.start_with(token2cor_string(Token::BREAKTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + BREAK_LEN])) {
            self.position += BREAK_LEN;
            return new TokenLexemePair(Token::BREAKTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getELSE(){
        if (self.start_with(token2cor_string(Token::ELSETK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + ELSE_LEN])) {
            self.position += ELSE_LEN;
            return new TokenLexemePair(Token::ELSETK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getWHILE(){
        if (self.start_with(token2cor_string(Token::WHILETK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + WHILE_LEN])) {
            self.position += WHILE_LEN;
            return new TokenLexemePair(Token::WHILETK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getGETINT(){
        if (self.start_with(token2cor_string(Token::GETINTTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + GETINT_LEN])) {
            self.position += GETINT_LEN;
            return new TokenLexemePair(Token::GETINTTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getPRINTF(){
        if (self.start_with(token2cor_string(Token::PRINTFTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + PRINTF_LEN])) {
            self.position += PRINTF_LEN;
            return new TokenLexemePair(Token::PRINTFTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getRETURN(){
        if (self.start_with(token2cor_string(Token::RETURNTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + RETURN_LEN])) {
            self.position += RETURN_LEN;
            return new TokenLexemePair(Token::RETURNTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getVOID(){
        if (self.start_with(token2cor_string(Token::VOIDTK)) &&
            !is_digit_or_nodigit(self.source_code[self.position + VOID_LEN])) {
            self.position += VOID_LEN;
            return new TokenLexemePair(Token::VOIDTK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getNEQ(){
        if (self.start_with(token2cor_string(Token::NEQ))) {
            self.position += 2;
            return new TokenLexemePair(Token::NEQ, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getNOT(){
        if (self.start_with(token2cor_string(Token::NOT))) {
            self.position += 1;
            return new TokenLexemePair(Token::NOT, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getAND(){
        if (self.start_with(token2cor_string(Token::AND))) {
            self.position += 2;
            return new TokenLexemePair(Token::AND, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getOR(){
        if (self.start_with(token2cor_string(Token::OR))) {
            self.position += 2;
            return new TokenLexemePair(Token::OR, self.lineNum);
        }
        return nullptr;
    }

    TokenLexemePair* getPLUS(){
        if (self.start_with(token2cor_string(Token::PLUS))) {
            self.position += 1;
            return new TokenLexemePair(Token::PLUS, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getMINU(){
        if (self.start_with(token2cor_string(Token::MINU))) {
            self.position += 1;
            return new TokenLexemePair(Token::MINU, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getMULT(){
        if (self.start_with(token2cor_string(Token::MULT))) {
            self.position += 1;
            return new TokenLexemePair(Token::MULT, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getDIV(){
        if (self.start_with(token2cor_string(Token::DIV))) {
            self.position += 1;
            return new TokenLexemePair(Token::DIV, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getMOD(){
        if (self.start_with(token2cor_string(Token::MOD))) {
            self.position += 1;
            return new TokenLexemePair(Token::MOD, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getLEQ(){
        if (self.start_with(token2cor_string(Token::LEQ))) {
            self.position += 2;
            return new TokenLexemePair(Token::LEQ, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getLSS(){
        if (self.start_with(token2cor_string(Token::LSS))) {
            self.position += 1;
            return new TokenLexemePair(Token::LSS, self.lineNum);
        }
        return nullptr;
    }TokenLexemePair* getGEQ(){
        if (self.start_with(token2cor_string(Token::GEQ))) {
            self.position += 2;
            return new TokenLexemePair(Token::GEQ, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getGRE(){
        if (self.start_with(token2cor_string(Token::GRE))) {
            self.position += 1;
            return new TokenLexemePair(Token::GRE, self.lineNum);
        }
        return nullptr;
    }TokenLexemePair* getEQL(){
        if (self.start_with(token2cor_string(Token::EQL))) {
            self.position += 2;
            return new TokenLexemePair(Token::EQL, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getASSIGN(){
        if (self.start_with(token2cor_string(Token::ASSIGN))) {
            self.position += 1;
            return new TokenLexemePair(Token::ASSIGN, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getSEMICN(){
        if (self.start_with(token2cor_string(Token::SEMICN))) {
            self.position += 1;
            return new TokenLexemePair(Token::SEMICN, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getCOMMA(){
        if (self.start_with(token2cor_string(Token::COMMA))) {
            self.position += 1;
            return new TokenLexemePair(Token::COMMA, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getLPARENT(){
        if (self.start_with(token2cor_string(Token::LPARENT))) {
            self.position += 1;
            return new TokenLexemePair(Token::LPARENT, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getRPARENT(){
        if (self.start_with(token2cor_string(Token::RPARENT))) {
            self.position += 1;
            return new TokenLexemePair(Token::RPARENT, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getLBRACK(){
        if (self.start_with(token2cor_string(Token::LBRACK))) {
            self.position += 1;
            return new TokenLexemePair(Token::LBRACK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getRBRACK(){
        if (self.start_with(token2cor_string(Token::RBRACK))) {
            self.position += 1;
            return new TokenLexemePair(Token::RBRACK, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getLBRACE(){
        if (self.start_with(token2cor_string(Token::LBRACE))) {
            self.position += 1;
            return new TokenLexemePair(Token::LBRACE, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getRBRACE(){
        if (self.start_with(token2cor_string(Token::RBRACE))) {
            self.position += 1;
            return new TokenLexemePair(Token::RBRACE, self.lineNum);
        }
        return nullptr;
    }
    TokenLexemePair* getINTCON(){
        // INTCON是不以0开头的整数
        if (self.source_code[self.position] < '0' || self.source_code[self.position] > '9') {
            return nullptr;
        }
        int old_position = self.position;
        for(; is_digit(self.source_code[self.position]); self.position++);
        std::string intcon_str = self.source_code.substr(old_position, self.position - old_position);
        if (intcon_str[0] == '0' && intcon_str.length() != 1) {
            throw IntegerStartWithZeroException("caught a integer: " + intcon_str + ", starting with 0");
        }
        return new TokenLexemePair(Token::INTCON, intcon_str, self.lineNum);
    }
    TokenLexemePair* getIDENFR(){
        if (!is_nodigit(self.source_code[self.position])) {
            return nullptr;
        }
        int old_position = self.position;
        for(; is_digit_or_nodigit(self.source_code[self.position]); self.position++);
        std::string ident_str = self.source_code.substr(old_position, self.position - old_position);
        return new TokenLexemePair(Token::IDENFR, ident_str, self.lineNum);
    }
    TokenLexemePair* getSTRCON(){
        if (self.source_code[self.position] != '"') {
            return nullptr;
        }
        int old_position = self.position;
        self.position ++;

        std::string temp = "";
        for (; self.position < self.code_len && self.source_code[self.position] != '"'; self.position++) {
            temp += self.source_code[self.position];
        }
        self.position += 1; //把最后一个'"'跳过去
        std::string str_con = self.source_code.substr(old_position, self.position - old_position);
        if (str_con[str_con.length() - 1] != '"') { // 最后一个字符是否为'"'
            throw RightDoubleQuotesNotFoundException("We didn't found a '\"' after string: " + str_con);
        }
        return new TokenLexemePair(Token::STRCON, str_con, self.lineNum);
    }
};


#endif //COMPILER_LEXICAL_ANALYSIS_LEXICALANALYZER_H
