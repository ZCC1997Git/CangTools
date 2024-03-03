/*************************************************************************
        > File Name: ParseFunction.cpp
        > Author: zhangchenchen
        > Mail: zhangchenchen1997@outlook.com
        > Created Time: Sun Mar  3 19:37:01 2024
 ************************************************************************/
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Tooling/Tooling.h>

using namespace clang;

class FunctionVisitor : public RecursiveASTVisitor<FunctionVisitor> {
public:
  bool VisitFunctionDecl(FunctionDecl *func) {
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
  virtual void HandleTranslationUnit(ASTContext &Context) {
    visitor.TraverseDecl(Context.getTranslationUnitDecl());
  }

private:
  FunctionVisitor visitor;
};

class FunctionAction : public ASTFrontendAction {
public:
  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &Compiler, llvm::StringRef InFile) {
    return std::unique_ptr<ASTConsumer>(new FunctionConsumer);
  }
};

int main(int argc, char **argv) {
  if (argc > 1) {
    clang::tooling::runToolOnCode(
        std::unique_ptr<clang::FrontendAction>(new FunctionAction), argv[1]);
  }
}
