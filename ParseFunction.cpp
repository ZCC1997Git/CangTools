/*************************************************************************
        > File Name: ParseFunction.cpp
        > Author: zhangchenchen
        > Mail: zhangchenchen1997@outlook.com
        > Created Time: Sun Mar  3 19:37:01 2024
 ************************************************************************/

/**
 * 这段代码是用于解析C++源代码并打印出函数名和参数的程序
 */
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

using namespace clang;

class FunctionVisitor : public RecursiveASTVisitor<FunctionVisitor> {
   public:
    /*用于访问抽象语法树（AST）中的每个函数声明。当访问到一个函数声明时，它会打印出函数名和参数。*/
    bool VisitFunctionDecl(FunctionDecl* func) {
        llvm::outs() << "Found function: " << func->getNameInfo().getAsString()
                     << "\n";
        for (auto param : func->parameters()) {
            llvm::outs() << "  Parameter: " << param->getNameAsString() << "\n";
        }
        return true;
    }
};

class FunctionConsumer : public ASTConsumer {
   public:
    /*用于处理一个翻译单元。翻译单元是C++程序的顶级实体，通常对应于一个源文件及其包含的头文件。
    这个函数会遍历翻译单元的所有声明*/
    virtual void HandleTranslationUnit(ASTContext& Context) {
        visitor.TraverseDecl(Context.getTranslationUnitDecl());
    }

   private:
    FunctionVisitor visitor;
};

class FunctionAction : public ASTFrontendAction {
   public:
    /*这是FunctionAction类的成员函数，它是ASTFrontendAction的一个方法，
    用于创建一个ASTConsumer。ASTConsumer是一个接口，它定义了如何处理AST的方法。*/
    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& Compiler,
        llvm::StringRef InFile) {
        return std::unique_ptr<ASTConsumer>(new FunctionConsumer);
    }
};

int main(int argc, char** argv) {
    if (argc > 1) {
        clang::tooling::runToolOnCode(
            std::unique_ptr<clang::FrontendAction>(new FunctionAction),
            argv[1]);
    }
}
