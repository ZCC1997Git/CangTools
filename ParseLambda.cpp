#include <iostream>
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;
using namespace clang::ast_matchers;

class MyPPCallbacks : public PPCallbacks {
   public:
    virtual void InclusionDirective(SourceLocation HashLoc,
                                    const Token& IncludeTok,
                                    StringRef FileName,
                                    bool IsAngled,
                                    CharSourceRange FilenameRange,
                                    const FileEntry* File,
                                    StringRef SearchPath,
                                    StringRef RelativePath,
                                    const Module* Imported) {
        std::cout << "Include file: " << FileName.str() << std::endl;
    }

   private:
};

class ParamVisitor : public RecursiveASTVisitor<ParamVisitor> {
   public:
    explicit ParamVisitor(const LambdaExpr* Lambda) : Lambda(Lambda) {}

    bool VisitBinaryOperator(BinaryOperator* op) {
        if (op->isAssignmentOp()) {
            Expr* lhs = op->getLHS()->IgnoreParenImpCasts();
            if (auto declRefExpr = dyn_cast<DeclRefExpr>(lhs)) {
                // 处理参数变量的赋值
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
            } else if (auto memberExpr = dyn_cast<MemberExpr>(lhs)) {
                // 处理类成员的赋值
                if (auto declRefExpr = dyn_cast<DeclRefExpr>(
                        memberExpr->getBase()->IgnoreParenImpCasts())) {
                    if (auto parmVarDecl =
                            dyn_cast<ParmVarDecl>(declRefExpr->getDecl())) {
                        for (const auto& Param : Lambda->getLambdaClass()
                                                     ->getLambdaCallOperator()
                                                     ->parameters()) {
                            if (Param == parmVarDecl) {
                                std::cout << "Member "
                                          << memberExpr->getMemberDecl()
                                                 ->getNameAsString()
                                          << " of parameter "
                                          << parmVarDecl->getNameAsString()
                                          << " is modified.\n";
                                break;
                            }
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
                Result.Nodes.getNodeAs<LambdaExpr>("lambda"))
            if (const CXXMethodDecl* CallOperator = Lambda->getCallOperator()) {
                if (CallOperator->hasAttr<AnnotateAttr>() &&
                    CallOperator->getAttr<AnnotateAttr>()->getAnnotation() ==
                        "kernel") {
                    std::cout << "Lambda found:"
                              << Lambda->getCallOperator()
                                     ->getCallResultType()
                                     .getAsString()
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
                            std::cout
                                << "Parameter: " << Param->getNameAsString()
                                << std::endl;
                        }
                    }

                    // 检查参数是否被修改
                    ParamVisitor visitor(Lambda);
                    visitor.TraverseStmt(const_cast<Stmt*>(Lambda->getBody()));
                }
            }
    }
};

class MyFrontendAction : public ASTFrontendAction {
   public:
    MyFrontendAction(MatchFinder* Finder) : Finder(Finder) {}

    virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI,
                                                           StringRef file) {
        Finder->addMatcher(lambdaExpr().bind("lambda"), new LambdaPrinter());
        return Finder->newASTConsumer();
    }

   private:
    MatchFinder* Finder;
};

class MyFrontendActionFactory : public FrontendActionFactory {
   public:
    MyFrontendActionFactory(MatchFinder* Finder) : Finder(Finder) {}

    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<MyFrontendAction>(Finder);
    }

   private:
    MatchFinder* Finder;
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

    // 创建包含搜索路径的向量
    // 创建头文件文件夹路径
    std::string HeaderFolderPath = "./include";

    // 注册头文件包含指令的回调函数
    // MyPPCallbacks Callbacks;

    // // 设置头文件搜索路径
    // HeaderSearchOptions HeaderSearchOpts;
    // HeaderSearchOpts.AddPath(HeaderFolderPath, frontend::Quoted, false,
    // false);

    // // 创建Clang编译器实例
    // CompilerInstance Compiler;
    // Compiler.createDiagnostics();
    // Compiler.getLangOpts().CPlusPlus = 1;  // 设置为1表示启用C++
    // Compiler.setTarget(TargetInfo::CreateTargetInfo(
    //     Compiler.getDiagnostics(),
    //     std::make_shared<clang::TargetOptions>()));

    // Compiler.createPreprocessor(TU_Complete);  // 自动创建Preprocessor对象
    // Compiler.getPreprocessorOpts().UsePredefines =
    //     false;  // 不使用默认的预定义宏
    // Compiler.getPreprocessor().addPPCallbacks(
    //     std::make_unique<MyPPCallbacks>(Callbacks));

    // 定义匹配Lambda表达式的AST Matcher
    StatementMatcher LambdaMatcher = lambdaExpr().bind("lambda");

    // 创建MatchFinder实例
    MatchFinder Finder;

    // 创建MyFrontendActionFactory实例
    auto Factory = std::make_unique<MyFrontendActionFactory>(&Finder);

    // 运行Clang工具
    return Tool.run(Factory.get());
}
