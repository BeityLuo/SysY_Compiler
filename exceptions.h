//#ifndef BUAA_COMPILER_EXCEPTIONS_H
//#define BUAA_COMPILER_EXCEPTIONS_H
//#include <exception>
//#include <bits/exception.h>
//#include <string>
//#include "syntax_analysis/syntax_nodes.h"
//#include <utility>
//#define self (*this)
//
//class CharacterNotMatchAnythingException : public std::exception{
//private:
//    std::string msg;
//public:
//    CharacterNotMatchAnythingException() : exception() {
//        self.msg = "No message has been got.";
//    }
//    CharacterNotMatchAnythingException(std::string msg) : exception(), msg(msg) {}
//
//    const char *what() const throw (){
//        return self.msg.c_str();
//    }
//};
//
//class LineException : public std::exception{
//private:
//    int exception_line_num;
//    std::string msg;
//public :
//    LineException(int exception_line_num) : exception(), exception_line_num(exception_line_num){
//        self.msg = "Exception happened in line " + std::to_string(self.exception_line_num);
//    }
//    LineException(int exception_line_num, std::string other_msg) : exception(), exception_line_num(exception_line_num){
//        self.msg = "Exception happened in line " + std::to_string(self.exception_line_num) + ": " + other_msg;
//    }
//
//    const char *what() const throw (){
//        return self.msg.c_str();
//    }
//};
//
//class TokenNotMatchFixedStringException : std::exception {
//private:
//    std::string msg;
//public:
//    TokenNotMatchFixedStringException() : exception() {
//        self.msg = "No message has been got.";
//    }
//    TokenNotMatchFixedStringException(std::string msg) : exception(), msg(msg) {}
//
//    const char* what() const throw (){
//        return self.msg.c_str();
//    }
//};
//
//class IntegerStartWithZeroException : std::exception {
//private:
//    std::string msg;
//public:
//    IntegerStartWithZeroException() : exception() {
//        self.msg = "No message has been got.";
//    }
//    IntegerStartWithZeroException(std::string msg) : exception(), msg(msg) {}
//
//    const char* what() const throw (){
//        return self.msg.c_str();
//    }
//};
//
//class RightDoubleQuotesNotFoundException : std::exception {
//private:
//    std::string msg;
//public:
//    RightDoubleQuotesNotFoundException() : exception() {
//        self.msg = "No message has been got.";
//    }
//    RightDoubleQuotesNotFoundException(std::string msg) : exception(), msg(msg) {}
//
//    const char* what() const throw (){
//        return self.msg.c_str();
//    }
//};
//
//class RightCommentSignNotFoundException : std::exception {
//private:
//    std::string msg;
//public:
//    RightCommentSignNotFoundException() : exception() {
//        self.msg = "No message has been got.";
//    }
//    RightCommentSignNotFoundException(std::string msg) : exception(), msg(msg) {}
//
//    const char* what() const throw (){
//        return self.msg.c_str();
//    }
//};
//
//
//
//#define self (*this)
//
//class ComponentNotFoundException : std::exception {
//private:
//    std::string father;
//    std::string component;
//public:
//    ComponentNotFoundException() : exception(), father(""), component(""){}
//
//    ComponentNotFoundException(std::string component, std::string father)
//            : exception(), father(std::move(father)), component(std::move(component)) {}
//
//    const char *what() const noexcept override {
//        return (self.component + " in " + self.father + " not found.").c_str();
//    }
//};
//
//class LogException : std::exception {
//private:
//    std::string log;
//public:
//    LogException() : exception(), log(""){}
//
//    explicit LogException(std::string log)
//            : exception(), log(std::move(log)) {}
//
//    const char *what() const noexcept override {
//        return self.log.c_str();
//    }
//};
//#endif //BUAA_COMPILER_EXCEPTIONS_H
