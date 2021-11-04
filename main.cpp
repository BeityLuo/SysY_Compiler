#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexical_analyzer/LexicalAnalyzer.h"
#include "syntax_analysis/SyntaxAnalyzer.h"
#include "semantic_analysis/SemanticAnalyzer.h"
#include "exception_management/MExceptionManager.h"

std::string file2string(std::string file_path) {
    std::ifstream ifile(file_path);
    std::ostringstream buff;
    char ch;
    while (buff && ifile.get(ch))
        buff.put(ch);
    ifile.close();
    return buff.str();
}

void string2file(std::string file_path, std::string data) {
    std::ofstream ofile(file_path);
    ofile << data;
    ofile.close();
}
const std::string in_file_path = "testfile.txt";
const std::string out_file_path = "output.txt";
const std::string error_file_path = "error.txt";

int main(){
    std::string source_code = file2string(in_file_path);
    LexicalAnalyzer lexical_analyzer(source_code);
    std::vector<TokenLexemePair*> token_list;
    std::string ans;
    lexical_analyzer.analyze(token_list);
    SyntaxAnalyzer syntax_analyzer(token_list);
    MCompUnit* ast = syntax_analyzer.analyze();
    if (ast != nullptr) {
        ans = ast->toString();
        std::cout << ans;

        SemanticAnalyzer semanticAnalyzer(syntax_analyzer.ast());
        semanticAnalyzer.analyze();
    }

//    for (auto tokenPair : token_list) {
//        delete(tokenPair);
//    }

    if (!MExceptionManager::empty()) {
        ans = MExceptionManager::toString();
        std::cout << ans;
        string2file(error_file_path, MExceptionManager::toString());
    } else {
        std::cout << ans;
        string2file(out_file_path, ans);
    }
    return 0;
}