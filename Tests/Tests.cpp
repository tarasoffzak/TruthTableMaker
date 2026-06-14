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
        // 1: Пустое выражение
        TEST_METHOD(EmptyExpression)
        {
            FunctManager fm;
            expectError("", fm, ErrorType::EVAL_ERROR);
        }

        // 2: Одиночная константа
        TEST_METHOD(SingleConstant)
        {
            FunctManager fm;
            expectSuccess("1", fm);
        }

        // 3: Одиночная переменная
        TEST_METHOD(SingleVariable)
        {
            FunctManager fm;
            expectSuccess("A", fm);
        }

        // 4: Бинарная операция (И)
        TEST_METHOD(BinaryOpAnd)
        {
            FunctManager fm;
            expectSuccess("A B &", fm);
        }

        // 5: Бинарная операция (ИЛИ)
        TEST_METHOD(BinaryOpOr)
        {
            FunctManager fm;
            expectSuccess("A B |", fm);
        }

        // 6: Унарная операция (НЕ)
        TEST_METHOD(UnaryOpNot)
        {
            FunctManager fm;
            expectSuccess("A !", fm);
        }

        // 7: Сложное выражение
        TEST_METHOD(ComplexExpression)
        {
            FunctManager fm;
            expectSuccess("A 1 & B C & | !", fm);
        }

        // 8: Нехватка аргументов для бинарного оператора
        TEST_METHOD(MissingArgsForBinaryOp)
        {
            FunctManager fm;
            expectError("A &", fm, ErrorType::EVAL_ERROR);
        }

        // 9: Нехватка аргументов для унарного оператора
        TEST_METHOD(MissingArgsForUnaryOp)
        {
            FunctManager fm;
            expectError("!", fm, ErrorType::EVAL_ERROR);
        }

        // 10: Избыток элементов в стеке
        TEST_METHOD(ExcessStackElements)
        {
            FunctManager fm;
            expectError("A B C &", fm, ErrorType::EVAL_ERROR);
        }

        // 11: Корректный вызов определённой функции
        TEST_METHOD(ValidUserFunction)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectSuccess("A B $myfunc", fm);
        }

        // 12: Вызов неопределённой функции
        TEST_METHOD(UndefinedFunction)
        {
            FunctManager fm;
            expectError("A B $unknownFunc", fm, ErrorType::SYNTAX_ERROR);
        }

        // 13: Нехватка аргументов для функции
        TEST_METHOD(MissingArgsForFunction)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A $myfunc", fm, ErrorType::EVAL_ERROR);
        }

        // 14: Некорректный префикс — после имени функции ("myfunc$")
        TEST_METHOD(PrefixAfterName)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A B myfunc$", fm, ErrorType::SYNTAX_ERROR);
        }

        // 15: Некорректный префикс — пробел между $ и именем ("$ myfunc")
        TEST_METHOD(StandalonePrefixWithSpace)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A B $ myfunc", fm, ErrorType::SYNTAX_ERROR);
        }

        // 16: Некорректный префикс — дублирование ("$$myfunc")
        TEST_METHOD(DuplicatedPrefix)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A B $$myfunc", fm, ErrorType::SYNTAX_ERROR);
        }

        // 17: Оператор склеен с именем переменной ("A&B")
        TEST_METHOD(OperatorGluedToVar)
        {
            FunctManager fm;
            expectError("A&B", fm, ErrorType::SYNTAX_ERROR);
        }

        // 18: Оператор внутри имени функции ("$my&func")
        TEST_METHOD(OperatorInsideFuncName)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A B $my&func", fm, ErrorType::SYNTAX_ERROR);
        }

        // 19: Недопустимый символ "%" — не оператор, не $, обрабатывается как переменная
        //     Стек в итоге содержит 3 элемента -> EVAL_ERROR
        TEST_METHOD(InvalidCharacter)
        {
            FunctManager fm;
            expectError("A B %", fm, ErrorType::EVAL_ERROR);
        }

        // 20: Многосимвольные наименования переменных и функции
        TEST_METHOD(MulticharNames)
        {
            FunctManager fm;
            setupFuntions(fm, "testfunct = A B & C &");
            expectSuccess("alpha1 alpha2 alpha3 $testfunct", fm);
        }

        // 21: Повторяющиеся имена переменных, функция вызывается дважды
        TEST_METHOD(RepeatedNames)
        {
            FunctManager fm;
            setupFuntions(fm, "func1 = A B &");
            expectSuccess("A A $func1 B $func1", fm);
        }

        // 22: Имя переменной совпадает с именем функции
        TEST_METHOD(VarNameMatchesFuncName)
        {
            FunctManager fm;
            setupFuntions(fm, "result = A B &");
            expectSuccess("result A $result", fm);
        }

        // 23: Более 10 уникальных переменных — дерево строится успешно
        //     (лимит на 10 переменных проверяется в generateTruthTable, не в parseExprRPN)
        TEST_METHOD(ManyVariables)
        {
            FunctManager fm;
            expectSuccess(
                "var1 var2 & var3 & var4 & var5 & var6 & var7 & var8 & var9 & var10 & var11 |",
                fm
            );
        }

        // 24: Функция без аргументов (тело — константа)
        TEST_METHOD(FunctionWithNoArgs)
        {
            FunctManager fm;
            setupFuntions(fm, "zero_arg_func = 1");
            expectSuccess("$zero_arg_func", fm);
        }

        // 25: Длинные имена переменных
        TEST_METHOD(LongVariableNames)
        {
            FunctManager fm;
            expectSuccess("first_logical_var_20 second_logical_var20 &", fm);
        }

        // 26: Глубокая вложенность операторов
        TEST_METHOD(DeepNesting)
        {
            FunctManager fm;
            expectSuccess("A ! B C D ! & ! | ! & !", fm);
        }

        // 27: Пограничное условие — ровно 10 переменных (рекомендуемый лимит)
        TEST_METHOD(MaxRecommendedVariables)
        {
            FunctManager fm;
            expectSuccess("A B & C & D & E & F & G & H & I & J &", fm);
        }
    };
}
