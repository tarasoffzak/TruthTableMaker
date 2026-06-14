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

        // 14: Некорректный префикс - после имени функции ("myfunc$")
        TEST_METHOD(PrefixAfterName)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A B myfunc$", fm, ErrorType::SYNTAX_ERROR);
        }

        // 15: Некорректный префикс - пробел между $ и именем ("$ myfunc")
        TEST_METHOD(StandalonePrefixWithSpace)
        {
            FunctManager fm;
            setupFuntions(fm, "myfunc = A B &");
            expectError("A B $ myfunc", fm, ErrorType::SYNTAX_ERROR);
        }

        // 16: Некорректный префикс - дублирование ("$$myfunc")
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

        // 19: Недопустимый символ "%" - не оператор, не $, обрабатывается как переменная
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

        // 23: Более 10 уникальных переменных - дерево строится успешно
        //     (лимит на 10 переменных проверяется в generateTruthTable, не в parseExprRPN)
        TEST_METHOD(ManyVariables)
        {
            FunctManager fm;
            expectSuccess(
                "var1 var2 & var3 & var4 & var5 & var6 & var7 & var8 & var9 & var10 & var11 |",
                fm
            );
        }

        // 24: Функция без аргументов (тело - константа)
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

        // 27: Пограничное условие - ровно 10 переменных (рекомендуемый лимит)
        TEST_METHOD(MaxRecommendedVariables)
        {
            FunctManager fm;
            expectSuccess("A B & C & D & E & F & G & H & I & J &", fm);
        }
    };

    // ========================================================================
    // ConstNode::evaluate
    // ========================================================================

    TEST_CLASS(ConstNodeTests)
    {
    public:
        // 1: Значение константы = 0
        TEST_METHOD(ConstFalse)
        {
            ConstNode node(false);
            Assert::IsFalse(node.evaluate(0), L"ConstNode(false) should evaluate to false");
        }

        // 2: Значение константы = 1
        TEST_METHOD(ConstTrue)
        {
            ConstNode node(true);
            Assert::IsTrue(node.evaluate(0), L"ConstNode(true) should evaluate to true");
        }
    };

    // ========================================================================
    // VarNode::evaluate
    // ========================================================================

    TEST_CLASS(VarNodeTests)
    {
    public:
        // 1: Контекст с 0 на позиции переменной: A=0
        TEST_METHOD(VarZeroContext)
        {
            VarNode node(0); // переменная с индексом 0 (бит 0)
            Assert::IsFalse(node.evaluate(0), L"VarNode(0) with bit 0 = 0 should be false");
        }

        // 2: Контекст с 1 на позиции переменной: A=1
        TEST_METHOD(VarOneContext)
        {
            VarNode node(0); // переменная с индексом 0 (бит 0)
            Assert::IsTrue(node.evaluate(1), L"VarNode(0) with bit 0 = 1 should be true");
        }
    };

    // ========================================================================
    // OperNode::evaluate
    // ========================================================================

    TEST_CLASS(OperNodeTests)
    {
    private:
        static void setupFunction(FunctManager& fm, const std::string& def) {
            Config cfg;
            ErrorManager::clear();
            fm.registerFunction(fm.parseUserFunction(def, cfg));
        }

        static std::shared_ptr<ExprTree> buildTree(const std::string& expr, FunctManager& fm) {
            Config cfg;
            ErrorManager::clear();
            return parseExprRPN(expr, cfg, fm);
        }

    public:
        // 1: Операция NOT: "1 !" = 0
        TEST_METHOD(OperNot)
        {
            FunctManager fm;
            auto tree = buildTree("1 !", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsFalse(tree->evaluate(0), L"NOT 1 should be false");
        }

        // 2: Операция AND: "1 0 &" = 0
        TEST_METHOD(OperAnd)
        {
            FunctManager fm;
            auto tree = buildTree("1 0 &", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsFalse(tree->evaluate(0), L"1 AND 0 should be false");
        }

        // 3: Операция OR: "0 1 |" = 1
        TEST_METHOD(OperOr)
        {
            FunctManager fm;
            auto tree = buildTree("0 1 |", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsTrue(tree->evaluate(0), L"0 OR 1 should be true");
        }

        // 4: Аргумент константа: "1 1 &" = 1
        TEST_METHOD(OperArgConst)
        {
            FunctManager fm;
            auto tree = buildTree("1 1 &", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsTrue(tree->evaluate(0), L"1 AND 1 should be true");
        }

        // 5: Аргумент переменная: "A B |", A=0, B=0 = 0
        TEST_METHOD(OperArgVar)
        {
            FunctManager fm;
            auto tree = buildTree("A B |", fm);
            Assert::IsNotNull(tree.get());
            // A - бит 0, B - бит 1; A=0, B=0 - ctx=0
            Assert::IsFalse(tree->evaluate(0), L"A(0) OR B(0) should be false");
        }

        // 6: Аргумент узел-операции: "A B & !", A=1, B=1 = 0
        TEST_METHOD(OperArgOperNode)
        {
            FunctManager fm;
            auto tree = buildTree("A B & !", fm);
            Assert::IsNotNull(tree.get());
            // A - бит 0, B - бит 1; A=1, B=1 - ctx=3
            Assert::IsFalse(tree->evaluate(3), L"NOT(A(1) AND B(1)) should be false");
        }

        // 7: Аргумент узел-функции: "A B $myfunc 1 &", A=1, B=0 = 1
        //    myfunc = A B | - с A=1, B=0 вернёт 1
        TEST_METHOD(OperArgFunctNode)
        {
            FunctManager fm;
            setupFunction(fm, "myfunc = A B |");
            auto tree = buildTree("A B $myfunc 1 &", fm);
            Assert::IsNotNull(tree.get());
            // A - бит 0, B - бит 1; A=1, B=0 - ctx=1
            Assert::IsTrue(tree->evaluate(1), L"myfunc(A(1),B(0)) AND 1 should be true");
        }
    };

    // ========================================================================
    // FunctNode::evaluate
    // ========================================================================

    TEST_CLASS(FunctNodeTests)
    {
    private:
        static void setupFunction(FunctManager& fm, const std::string& def) {
            Config cfg;
            ErrorManager::clear();
            fm.registerFunction(fm.parseUserFunction(def, cfg));
        }

        static std::shared_ptr<ExprTree> buildTree(const std::string& expr, FunctManager& fm) {
            Config cfg;
            ErrorManager::clear();
            return parseExprRPN(expr, cfg, fm);
        }

    public:
        // 1: Функция из базисных операций: impl = A ! B |, "0 1 $impl" = 1
        TEST_METHOD(FuncBasicOps)
        {
            FunctManager fm;
            setupFunction(fm, "impl = A ! B |");
            auto tree = buildTree("0 1 $impl", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsTrue(tree->evaluate(0), L"impl(0,1): !0 OR 1 should be true");
        }

        // 2: Унарная функция: not_f = A !, "1 $not_f" = 0
        TEST_METHOD(FuncUnary)
        {
            FunctManager fm;
            setupFunction(fm, "not_f = A !");
            auto tree = buildTree("1 $not_f", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsFalse(tree->evaluate(0), L"not_f(1): !1 should be false");
        }

        // 3: N-арная функция (3 аргумента): func3 = A B & C |, "1 0 1 $func3" = 1
        TEST_METHOD(FuncNAry)
        {
            FunctManager fm;
            setupFunction(fm, "func3 = A B & C |");
            auto tree = buildTree("1 0 1 $func3", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsTrue(tree->evaluate(0), L"func3(1,0,1): (1 AND 0) OR 1 should be true");
        }

        // 4: Аргумент константа: f = A B &, "1 0 $f" = 0
        TEST_METHOD(FuncArgConst)
        {
            FunctManager fm;
            setupFunction(fm, "f = A B &");
            auto tree = buildTree("1 0 $f", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsFalse(tree->evaluate(0), L"f(1,0): 1 AND 0 should be false");
        }

        // 5: Аргумент переменная: f = A B |, "X Y $f", X=1, Y=0 = 1
        TEST_METHOD(FuncArgVar)
        {
            FunctManager fm;
            setupFunction(fm, "f = A B |");
            auto tree = buildTree("X Y $f", fm);
            Assert::IsNotNull(tree.get());
            // X - бит 0, Y - бит 1; X=1, Y=0 - ctx=1
            Assert::IsTrue(tree->evaluate(1), L"f(X(1),Y(0)): 1 OR 0 should be true");
        }

        // 6: Аргумент узел-операции: f = A B &, "X Y | 1 $f", X=0, Y=1 = 1
        TEST_METHOD(FuncArgOperNode)
        {
            FunctManager fm;
            setupFunction(fm, "f = A B &");
            auto tree = buildTree("X Y | 1 $f", fm);
            Assert::IsNotNull(tree.get());
            // X - бит 0, Y - бит 1; X=0, Y=1 - ctx=2
            Assert::IsTrue(tree->evaluate(2), L"f(X(0) OR Y(1), 1): 1 AND 1 should be true");
        }

        // 7: Аргумент узел-функции: g = A !, f = A B &, "1 $g 1 $f" = 0
        TEST_METHOD(FuncArgFunctNode)
        {
            FunctManager fm;
            setupFunction(fm, "g = A !");
            setupFunction(fm, "f = A B &");
            auto tree = buildTree("1 $g 1 $f", fm);
            Assert::IsNotNull(tree.get());
            // g(1) = !1 = 0; f(g(1), 1) = f(0, 1) = 0 AND 1 = 0
            Assert::IsFalse(tree->evaluate(0), L"f(g(1),1): g(1)=0, 0 AND 1 should be false");
        }

        // 8: Функция содержит подфункцию: sub = A !, main = A B $sub &, "1 0 $main" = 1
        TEST_METHOD(FuncContainsSubFunc)
        {
            FunctManager fm;
            setupFunction(fm, "sub = A !");
            setupFunction(fm, "main = A B $sub &");
            auto tree = buildTree("1 0 $main", fm);
            Assert::IsNotNull(tree.get());
            // main(1, 0): A=1, sub(B=0)=!0=1; A AND sub(B) = 1 AND 1 = 1
            Assert::IsTrue(tree->evaluate(0), L"main(1,0): 1 AND sub(0)=!0=1 should be true");
        }

        // 9: Функция без аргументов: zero_arg_func = 1, "$zero_arg_func" = 1
        TEST_METHOD(FuncNoArgs)
        {
            FunctManager fm;
            setupFunction(fm, "zero_arg_func = 1");
            auto tree = buildTree("$zero_arg_func", fm);
            Assert::IsNotNull(tree.get());
            Assert::IsTrue(tree->evaluate(0), L"zero_arg_func should return true");
        }
    };
}
