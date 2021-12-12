cmake_minimum_required(VERSION 3.13)
project(BUAA_Compiler)

set(CMAKE_CXX_STANDARD 11)

add_executable(BUAA_Compiler main.cpp lexical_analyzer/LexicalAnalyzer.h lexical_analyzer/lexical_tools.h lexical_analyzer/exceptions.h lexical_analyzer/lexical_tools.cpp ast_generate/ASTGenerator.h ast_generate/syntax_nodes.h ast_generate/syntax_nodes_classes.h ast_generate/syntax_exceptions.h ast_generate/syntax_nodes.cpp IR_generate/IRGenerator.h IR_generate/MSymbolTable.h IR_generate/MSymbolTableItem.h IR_generate/MSymbol.h exception_management/exceptions.h IR_generate/MSymbolTable.cpp exception_management/MExceptionManager.h exception_management/MExceptionManager.cpp IR_generate/MSymbolTableItem.cpp IR/IRStatements.h IR/IRVar.h IR_generate/MLabelManager.h IR_generate/MExpCalculator.h IRVirtualMachine/MIRVirtualMachine.h IRVirtualMachine/MRAM.h IRVirtualMachine/MIRSymbolTable.h IRVirtualMachine/MIRSymbolTableItem.h)