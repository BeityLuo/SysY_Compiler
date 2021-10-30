#ifndef BUAA_COMPILER_EXCEPTIONS_H
#define BUAA_COMPILER_EXCEPTIONS_H

#include <string>

#define self (*this)

class MException {
private:
    char code;
    int lineNum;
public:
    MException(char code, int lineNum) : code(code), lineNum(lineNum) {}
    std::string toString() {
        return "" + std::to_string(lineNum) + " " + code + "\n";
    }
};

class MIllegalCharInFormatStringException : public MException{
public:
    MIllegalCharInFormatStringException(int lineNum) : MException('a', lineNum) {}
};

class MRedefinedIdentifierException : public MException{
public:
    MRedefinedIdentifierException(int lineNum) : MException('b', lineNum) {}
};
class MUndefinedIdentifierException : public MException{
public:
    MUndefinedIdentifierException(int lineNum) : MException('c', lineNum) {}
};
class MParamNumNotMatchException : public MException{
public:
    MParamNumNotMatchException(int lineNum) : MException('d', lineNum) {}
};
class MParamTypeNotMatchException : public MException{
public:
    MParamTypeNotMatchException(int lineNum) : MException('e', lineNum) {}
};
class MRedundantReturnStatementException : public MException{
public:
    MRedundantReturnStatementException(int lineNum) : MException('f', lineNum) {}
};
class MMissingReturnStatementException : public MException{
public:
    MMissingReturnStatementException(int lineNum) : MException('g', lineNum) {}
};
class MChangingConstException : public MException{
public:
    MChangingConstException(int lineNum) : MException('h', lineNum) {}
};
class MMissingSemicolonException : public MException{
public:
    MMissingSemicolonException(int lineNum) : MException('i', lineNum) {}
};
class MMissingRParenthesesException: public MException{
public:
    MMissingRParenthesesException(int lineNum) : MException('j', lineNum) {}
};
class MMissingRBracketException : public MException{
public:
    MMissingRBracketException(int lineNum) : MException('k', lineNum) {}
};
class MPrintfParamNumNotMatchException : public MException{
public:
    MPrintfParamNumNotMatchException(int lineNum) : MException('l', lineNum) {}
};

class MContinueStatementInAcyclicBlockException : public MException{
public:
    MContinueStatementInAcyclicBlockException(int lineNum) : MException('m', lineNum) {}
};

class MBreakStatementInAcyclicBlockException : public MException{
public:
    MBreakStatementInAcyclicBlockException(int lineNum) : MException('m', lineNum) {}
};

#endif //BUAA_COMPILER_EXCEPTIONS_H
