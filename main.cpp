#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "lexical_analyzer/LexicalAnalyzer.h"
#include "ast_generate//ASTGenerator.h"
#include "IR_generate/IRGenerator.h"
#include "exception_management/MExceptionManager.h"
#include "IRVirtualMachine/MIRVirtualMachine.h"

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
const std::string pcode_out_file_path = "pcoderesult.txt";

const bool ifOutput = false;

int main(){

        try {
            std::string source_code = file2string(in_file_path);
            LexicalAnalyzer lexical_analyzer(source_code);
            std::vector<TokenLexemePair*> token_list;
            std::string ans;
            try {
                lexical_analyzer.analyze(token_list);
                if (ifOutput) {
                    for (auto item : token_list) {
                        std::cout << TokenLexemePair2string(item) << std::endl;
                    }
                }
            } catch (CharacterNotMatchAnythingException e) {
                for (auto item : token_list) {
                    std::cout << TokenLexemePair2string(item) << std::endl;
                }
            }



            MSymbolTable *globalTable = new MSymbolTable(nullptr);
            // MSymbolTable *currentTable = globalTable;
            IRGenerator* irGenerator = new IRGenerator(globalTable);
            ASTGenerator astGenerator(token_list, globalTable, irGenerator);
            // 语法分析、语义分析、生成中间代码
            MCompUnit* ast = astGenerator.analyze();

            if (ifOutput && MExceptionManager::empty()) {
                if (ast != nullptr) {
                    // std::cout << ast->toString();
                }
                std::cout << "\n\n\n";
                std::cout << irGenerator->toString();
                std::cout << "\n\n\n";
            }
            if (MExceptionManager::empty()) {
                // 虚拟机跑
                MIRVirtualMachine vm(irGenerator->getIRStatements());
                vm.run();
                ans = vm.output();
                if (ifOutput)
                    std::cout << ans;
                string2file(pcode_out_file_path, ans);
            } else {
                ans = MExceptionManager::toString();
                if (ifOutput)
                    std::cout << ans;
                string2file(error_file_path, MExceptionManager::toString());
            }
        } catch (char const* msg) {
            std::cout << msg;
        } catch (const std::bad_typeid& fg) {
            std::cout << fg.what() << std::endl;
        } catch (std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > msg) {
            std::cout << msg;
        }
    return 0;
}