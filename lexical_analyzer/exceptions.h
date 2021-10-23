#include <exception>
#ifndef COMPILER_LEXICAL_ANALYSIS_EXCEPTIONS_H
#define COMPILER_LEXICAL_ANALYSIS_EXCEPTIONS_H
#define self (*this)

class CharacterNotMatchAnythingException : public std::exception{
private:
    std::string msg;
public:
    CharacterNotMatchAnythingException() : exception() {
        self.msg = "No message has been got.";
    }
    CharacterNotMatchAnythingException(std::string msg) : exception(), msg(msg) {}

    const char *what() const throw (){
        return self.msg.c_str();
    }
};

class LineException : public std::exception{
private:
    int exception_line_num;
    std::string msg;
public :
    LineException(int exception_line_num) : exception(), exception_line_num(exception_line_num){
        self.msg = "Exception happened in line " + std::to_string(self.exception_line_num);
    }
    LineException(int exception_line_num, std::string other_msg) : exception(), exception_line_num(exception_line_num){
        self.msg = "Exception happened in line " + std::to_string(self.exception_line_num) + ": " + other_msg;
    }

    const char *what() const throw (){
        return self.msg.c_str();
    }
};

class TokenNotMatchFixedStringException : std::exception {
private:
    std::string msg;
public:
    TokenNotMatchFixedStringException() : exception() {
        self.msg = "No message has been got.";
    }
    TokenNotMatchFixedStringException(std::string msg) : exception(), msg(msg) {}

    const char* what() const throw (){
        return self.msg.c_str();
    }
};

class IntegerStartWithZeroException : std::exception {
private:
std::string msg;
public:
IntegerStartWithZeroException() : exception() {
    self.msg = "No message has been got.";
}
IntegerStartWithZeroException(std::string msg) : exception(), msg(msg) {}

const char* what() const throw (){
    return self.msg.c_str();
}
};

class RightDoubleQuotesNotFoundException : std::exception {
private:
    std::string msg;
public:
    RightDoubleQuotesNotFoundException() : exception() {
        self.msg = "No message has been got.";
    }
    RightDoubleQuotesNotFoundException(std::string msg) : exception(), msg(msg) {}

    const char* what() const throw (){
        return self.msg.c_str();
    }
};

class RightCommentSignNotFoundException : std::exception {
private:
    std::string msg;
public:
    RightCommentSignNotFoundException() : exception() {
        self.msg = "No message has been got.";
    }
    RightCommentSignNotFoundException(std::string msg) : exception(), msg(msg) {}

    const char* what() const throw (){
        return self.msg.c_str();
    }
};

#endif //COMPILER_LEXICAL_ANALYSIS_EXCEPTIONS_H
