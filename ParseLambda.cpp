#include <iostream>
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

class ParamVisitor : public RecursiveASTVisitor<ParamVisitor> {
   public:
    explicit ParamVisitor(const LambdaExpr* Lambda) : Lambda(Lambda) {}

    bool VisitBinaryOperator(BinaryOperator* op) {
        if (op->isAssignmentOp()) {
            if (auto declRefExpr = dyn_cast<DeclRefExpr>(
                    op->getLHS()->IgnoreParenImpCasts())) {
                if (auto parmVarDecl =
                        dyn_cast<ParmVarDecl>(declRefExpr->getDecl())) {
                    for (const auto& Param : Lambda->getLambdaClass()
                                                 ->getLambdaCallOperator()
                                                 ->parameters()) {
                        if (Param == parmVarDecl) {
                            std::cout << "Parameter "
                                      << parmVarDecl->getNameAsString()
                                      << " is modified.\n";
                            break;
                        }
                    }
                }
            }
        }
        return true;
    }

   private:
    const LambdaExpr* Lambda;
};

class LambdaPrinter : public MatchFinder::MatchCallback {
   public:
    virtual void run(const MatchFinder::MatchResult& Result) {
        if (const LambdaExpr* Lambda =
                Result.Nodes.getNodeAs<LambdaExpr>("lambda")) {
            std::cout
                << "Lambda found:"
                << Lambda->getCallOperator()->getCallResultType().getAsString()
                << std::endl;

            // 输出捕获列表
            for (const auto& Capture : Lambda->captures()) {
                std::cout << "Capture: "
                          << Capture.getCapturedVar()->getNameAsString()
                          << std::endl;
            }

            // 输出输入参数
            if (Lambda->getLambdaClass() &&
                Lambda->getLambdaClass()->getLambdaCallOperator()) {
                for (const auto& Param : Lambda->getLambdaClass()
                                             ->getLambdaCallOperator()
                                             ->parameters()) {
                    std::cout << "Parameter: " << Param->getNameAsString()
                              << std::endl;
                }
            }

            // 检查参数是否被修改
            ParamVisitor visitor(Lambda);
            visitor.TraverseStmt(const_cast<Stmt*>(Lambda->getBody()));
        }
    }
};

int main(int argc, const char** argv) {
    llvm::cl::OptionCategory MyToolCategory("my-tool options");
    auto ExpectedParser =
        CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser& OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    // 定义匹配Lambda表达式的AST Matcher
    StatementMatcher LambdaMatcher = lambdaExpr().bind("lambda");

    // 创建MatchFinder并注册回调
    MatchFinder Finder;
    LambdaPrinter Printer;
    Finder.addMatcher(LambdaMatcher, &Printer);

    // 运行Clang工具
    return Tool.run(newFrontendActionFactory(&Finder).get());
}
