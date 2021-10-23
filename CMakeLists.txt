cmake_minimum_required(VERSION 3.13)
project(BUAA_Compiler)

set(CMAKE_CXX_STANDARD 11)

add_executable(BUAA_Compiler main.cpp TestClass.h lexical_analyzer/LexicalAnalyzer.h lexical_analyzer/lexical_tools.h lexical_analyzer/exceptions.h lexical_analyzer/lexical_tools.cpp syntax_analysis/SyntaxAnalyzer.h syntax_analysis/syntax_nodes.h syntax_analysis/syntax_nodes_classes.h syntax_analysis/syntax_exceptions.h syntax_analysis/syntax_nodes.cpp semantic_analysis/SemanticAnalyzer.h semantic_analysis/SymbolTable.h semantic_analysis/MSymbolTableItem.h semantic_analysis/MSymbol.h exceptions.h tools.h)