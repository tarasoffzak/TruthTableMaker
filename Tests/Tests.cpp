#include "pch.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
    TEST_CLASS(ParseExprRPNTests)
    {
    private:
        static void setupFuntions(FunctManager& fm, const std::string& def) {
            Config cfg;
            ErrorManager::clear();
            fm.registerFunction(fm.parseUserFunction(def, cfg));
        }

        static void expectSuccess(const std::string& expr, FunctManager& fm) {
            Config cfg;
            ErrorManager::clear();
            auto result = parseExprRPN(expr, cfg, fm);
            Assert::IsNotNull(result.get(), L"Expected non-null tree");
            Assert::IsFalse(ErrorManager::hasErrors(), L"Expected no errors");
        }

        static void expectError(const std::string& expr, FunctManager& fm, ErrorType expectedType) {
            Config cfg;
            ErrorManager::clear();
            auto result = parseExprRPN(expr, cfg, fm);
            Assert::IsNull(result.get(), L"Expected null (error) result");
            Assert::IsTrue(ErrorManager::hasErrors(), L"Expected errors in history");
            Assert::IsTrue(
                ErrorManager::getHistory()[0].getType() == expectedType,
                L"Unexpected error type"
            );
        }

    public:
        
    };
}
