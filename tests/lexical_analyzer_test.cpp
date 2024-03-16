#include "lexical_analyzer/thompson_constructor.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace std;

// ===== Group =====

TEST(Group, RuleAndParseSingleSameLetter)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("1", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("1");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    ASSERT_EQ(parseRes[0]->text, "1");
}

TEST(Group, RuleAndParseStrDifferent)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("1111111", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("22");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
    ASSERT_EQ(parseRes[0]->text, "");
}

TEST(Group, RuleMatchesSeveralTimes)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    const std::string rule = "12";
    lexConstructor->addRule(rule, TerminalSymbol::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(ruleDuplicated);

    EXPECT_EQ(parseRes.size(), duplicatesCnt);
    for (auto token : parseRes) {
        EXPECT_EQ(token->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(token->text, "12");
    }
}

TEST(Group, TwoRulesFirstFails)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("222", TerminalSymbol::BLANK);
    lexConstructor->addRule("11", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("11");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "11");
}

TEST(Group, TwoRulesMatchLongest)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("11", TerminalSymbol::BLANK);
    lexConstructor->addRule("1111", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("1111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1111");
}

TEST(Group, SimpleGroup)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("(12)", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, SingleRuleNested2Times)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("((12))", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, SingleRuleNested32Times)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    std::string rule = "12";
    for (size_t i = 0; i < 32; ++i) {
        rule = '(' + rule + ')';
    }
    lexConstructor->addRule(rule, TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, SingleRuleNested256Times)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    std::string rule = "12";
    for (size_t i = 0; i < 256; ++i) {
        rule = '(' + rule + ')';
    }
    lexConstructor->addRule(rule, TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, GroupOf2Groups)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("((12)(34))", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("1234");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1234");
}

TEST(Group, TwoGroupsNested32Times)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    std::string rule = "(12)(34)";
    for (size_t i = 0; i < 32; ++i) {
        rule = '(' + rule + ')';
    }
    lexConstructor->addRule(rule, TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("1234");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1234");
}

// ===== Union =====
TEST(Union, SimpleUnion)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("[12]", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("1");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1");
}

TEST(Union, SimpleUnionFail)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("[12]", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("3");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
}

TEST(Union, NestedUnionsEachSymStandalone)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("[[01][2][345]]", TerminalSymbol::ASSIGN_OP);
    for (size_t i = 0; i <= 5; ++i) {
        const auto parseRes = LexicalAnalyzer(lexConstructor).parse(to_string(i));
        ASSERT_EQ(parseRes.size(), 1);
        EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[0]->text, std::to_string(i));
    }

    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("6");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
}

TEST(Union, NestedUnionsSymbolsTogether)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("[[01][2][345][6[7][89]]]", TerminalSymbol::ASSIGN_OP);

    std::string toParse;
    const size_t cnt = 10;
    for (size_t i = 0; i < cnt; ++i) {
        toParse += to_string(i);
    }

    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), cnt);
    for (size_t i = 0; i < cnt; ++i) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[i]->text, std::to_string(i));
    }
}

// ===== Asterisk =====
TEST(Asterisk, SimpleAsterisk)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("1*", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("1111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1111");
}

TEST(Asterisk, AsteriskMatchesEmptyAndGroup)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("1*2", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("2");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "2");
}

TEST(Asterisk, AsteriskMatchesManyCnt)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("1*", TerminalSymbol::ASSIGN_OP);
    std::string toParse;
    for (size_t i = 1; i <= 100; ++i) {
        toParse += "1";
        const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
        ASSERT_EQ(parseRes.size(), 1);
        EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[0]->text, toParse);
    }
}

TEST(Asterisk, AsteriskMatchesManyAndGroup)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("1*2", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("11112");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "11112");
}

TEST(Asterisk, AsteriskMatchFail)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("2*", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("111111111111111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
    EXPECT_EQ(parseRes[0]->text, "");
}

TEST(Asterisk, AsteriskMatchAndGroupFail)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("2*3", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("111111111111111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
    EXPECT_EQ(parseRes[0]->text, "");
}

TEST(Asterisk, SimpleGroupAsterisk)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    const std::string rule = "123456789";
    lexConstructor->addRule("(" + rule + ")*", TerminalSymbol::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }

    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(ruleDuplicated);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, ruleDuplicated);
}

TEST(Asterisk, GroupAsteriskMatchesAndFail)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    const std::string rule = "123456789";
    lexConstructor->addRule("(" + rule + ")*", TerminalSymbol::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }
    ruleDuplicated.pop_back();

    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(ruleDuplicated);
    ASSERT_GT(parseRes.size(), 0);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, ruleDuplicated.substr(0, rule.size() * (duplicatesCnt - 1)));
    EXPECT_EQ(parseRes[1]->symbolType, TerminalSymbol::ERROR);
    EXPECT_EQ(parseRes[1]->text, "");
}

TEST(Asterisk, TwoAsterisks)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("2*3*", TerminalSymbol::ASSIGN_OP);
    const std::string toParse = "22222333333333333";
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

TEST(Asterisk, AsteriskAndGroupMatchesLongest)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("2*", TerminalSymbol::BLANK);
    lexConstructor->addRule("2*", TerminalSymbol::CLOSED_BRACKET);
    lexConstructor->addRule("2*1", TerminalSymbol::ASSIGN_OP);
    const std::string toParse = "22222221";
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

TEST(Asterisk, TwoAsteriskRulesFirstFails)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("3*", TerminalSymbol::BLANK);
    lexConstructor->addRule("2*", TerminalSymbol::ASSIGN_OP);
    const std::string toParse = "2222222";
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

// ====== Dot ======
TEST(Dot, DotMatchesAnyChar)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule(".", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("2");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "2");
}

TEST(Dot, DotMatchesOnlySingleChar)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule(".", TerminalSymbol::ASSIGN_OP);
    const size_t toParseLength = 64;
    std::string toParse;
    for (size_t i = 0; i < toParseLength; ++i) {
        toParse += to_string(i % 10);
    }
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), toParseLength);
    for (size_t i = 0; i < toParseLength; ++i) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[i]->text, std::string(1, toParse[i]));
    }
}

// ====== Dot and Asterisk ======
TEST(DotAndAsterisk, DotMatchesAnything)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule(".*", TerminalSymbol::ASSIGN_OP);
    const size_t toParseLength = 64;
    std::string toParse;
    for (size_t i = 0; i < toParseLength; ++i) {
        toParse += to_string(i % 10);
    }
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

// ====== Asterisk and Union ======
TEST(AsteriskAndUnion, AsteriskMatchesEachCharFromUnion)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    const std::string rule = "123456789";
    lexConstructor->addRule("[" + rule + "]*", TerminalSymbol::ASSIGN_OP);

    std::string toParse;
    toParse.reserve(rule.size());
    for (size_t i = 0; i < rule.size(); ++i) {
        toParse.push_back(rule[i]);
    }

    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

// ====== Escaped symbols ======
TEST(EscapedSymbols, EscapedDotMatchesDot)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\.", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(".");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, ".");
}

TEST(EscapedSymbols, EscapedDotMatchesCharFail)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\.", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("a");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
}

TEST(EscapedSymbols, EscapedOpenParMatchesOpenPar)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\(", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("(");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "(");
}

TEST(EscapedSymbols, EscapedCloseParMatchesClosePar)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\)", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(")");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, ")");
}

TEST(EscapedSymbols, EscapedOpenBracketMatchesOpenBracket)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\[", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("[");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "[");
}

TEST(EscapedSymbols, EscapedCloseBracketMatchesCloseBracket)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\]", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("]");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "]");
}

TEST(EscapedSymbols, EscapedAsterixMatchesAsterix)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\*", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("*");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "*");
}

TEST(EscapedSymbols, EscapedPlusMatchesPlus)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\+", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("+");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "+");
}

TEST(EscapedSymbols, EscapedBackslashMatchesBackslash)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("\\\\", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("\\");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "\\");
}

TEST(EscapedSymbols, EscapedBackslashWithLettersMatchesBackslashWithLetters)
{
    auto lexConstructor = std::make_shared<ThompsonConstructor>();
    lexConstructor->addRule("a\\\\b", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse("a\\b");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "a\\b");
}

// ====== Real tokens ======
class RealTokens : public ::testing::Test
{
protected:
    void SetUp() override
    {
        lexConstructor = std::make_shared<ThompsonConstructor>();
        lexConstructor->addRule(ThompsonConstructor::allDigits + "+\\." +
                                    ThompsonConstructor::allDigits + "+",
                                TerminalSymbol::NUMBER);
        lexConstructor->addRule(ThompsonConstructor::allDigits + "+", TerminalSymbol::NUMBER);
        lexConstructor->addRule(" ", TerminalSymbol::BLANK);
        lexConstructor->addRule("\n", TerminalSymbol::NEWLINE);
        lexConstructor->addRule(";.*\n", TerminalSymbol::COMMENT);
        lexConstructor->addRule("#\\\\" + LexicalAnalyzerConstructor::allLetters + "+",
                                TerminalSymbol::CHARACTER);
        lexConstructor->addRule("define", TerminalSymbol::DEFINE);
    }
    std::shared_ptr<LexicalAnalyzerConstructor> lexConstructor;
};

TEST_F(RealTokens, NumbersWithSpaces)
{
    const vector<std::string> numbers = {"0.12",  "0.1", "22", "333",   "4444.1234567",
                                         "55555", "6",   "77", "888.9", "9999.0001"};
    std::string toParse;
    for (auto integer : numbers) {
        toParse += integer + " ";
    }
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 2 * numbers.size());
    for (size_t i = 0; i < parseRes.size(); i += 2) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::NUMBER);
        EXPECT_EQ(parseRes[i]->text, numbers[i / 2]);
        EXPECT_EQ(parseRes[i + 1]->symbolType, TerminalSymbol::BLANK);
        EXPECT_EQ(parseRes[i + 1]->text, " ");
    }
}

TEST_F(RealTokens, CharsWitHSpaces)
{
    const std::string shuffledLetters = "WpXZoyVwmGdrlbCxgQnthqieRPfIkuDcMaBsJLEvAHOzSYFTjUKN";
    std::string toParse;
    for (auto ch : shuffledLetters) {
        toParse += "#\\" + std::string(1, ch) + " ";
    }
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 2 * shuffledLetters.size());
    for (size_t i = 0; i < parseRes.size(); i += 2) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::CHARACTER);
        EXPECT_EQ(parseRes[i]->text, "#\\" + std::string(1, shuffledLetters[i / 2]));
        EXPECT_EQ(parseRes[i + 1]->symbolType, TerminalSymbol::BLANK);
        EXPECT_EQ(parseRes[i + 1]->text, " ");
    }
}

TEST_F(RealTokens, Comment)
{
    const std::string toParse = "1\n;123 asdf 1234234\n2";
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 4);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::NUMBER);
    EXPECT_EQ(parseRes[1]->symbolType, TerminalSymbol::NEWLINE);
    EXPECT_EQ(parseRes[2]->symbolType, TerminalSymbol::COMMENT);
    EXPECT_EQ(parseRes[3]->symbolType, TerminalSymbol::NUMBER);
}

TEST_F(RealTokens, MatchKeyword)
{
    const std::string toParse = "1 define";
    const auto parseRes = LexicalAnalyzer(lexConstructor).parse(toParse);
    ASSERT_EQ(parseRes.size(), 3);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::NUMBER);
    EXPECT_EQ(parseRes[1]->symbolType, TerminalSymbol::BLANK);
    EXPECT_EQ(parseRes[2]->symbolType, TerminalSymbol::DEFINE);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
