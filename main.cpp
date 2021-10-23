#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexical_analyzer/LexicalAnalyzer.h"
#include "syntax_analysis/SyntaxAnalyzer.h"

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

int main(){
    std::string source_code = file2string(in_file_path);
    LexicalAnalyzer lexical_analyzer(source_code);
    std::vector<TokenLexemePair*> token_list;

    lexical_analyzer.analyze(token_list);
    SyntaxAnalyzer syntax_analyzer(token_list);
    std::string ans;
    ans = syntax_analyzer.analyze().toString();
    for (auto tokenPair : token_list) {
        delete(tokenPair);
    }
    SemanticAnalyzer semanticAnalyzer(syntax_analyzer.ast());
    semanticAnalyzer.analyze();
    string2file(out_file_path, ans);
    return 0;
}