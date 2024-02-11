#include "lexical_analyzer.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace std;

// ===== Group =====

TEST(Group, RuleAndParseSingleSameLetter)
{
    LexicalAnalyzer lex;
    lex.addRule("1", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("1");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    ASSERT_EQ(parseRes[0]->text, "1");
}

TEST(Group, RuleAndParseStrDifferent)
{
    LexicalAnalyzer lex;
    lex.addRule("1111111", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("22");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
    ASSERT_EQ(parseRes[0]->text, "");
}

TEST(Group, RuleMatchesSeveralTimes)
{
    LexicalAnalyzer lex;
    const std::string rule = "12";
    lex.addRule(rule, TerminalSymbol::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }
    const auto parseRes = lex.parse(ruleDuplicated);

    EXPECT_EQ(parseRes.size(), duplicatesCnt);
    for (auto token : parseRes) {
        EXPECT_EQ(token->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(token->text, "12");
    }
}

TEST(Group, TwoRulesFirstFails)
{
    LexicalAnalyzer lex;
    lex.addRule("222", TerminalSymbol::BLANK);
    lex.addRule("11", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("11");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "11");
}

TEST(Group, TwoRulesMatchLongest)
{
    LexicalAnalyzer lex;
    lex.addRule("11", TerminalSymbol::BLANK);
    lex.addRule("1111", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("1111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1111");
}

TEST(Group, SimpleGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("(12)", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, SingleRuleNested2Times)
{
    LexicalAnalyzer lex;
    lex.addRule("((12))", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, SingleRuleNested32Times)
{
    LexicalAnalyzer lex;
    std::string rule = "12";
    for (size_t i = 0; i < 32; ++i) {
        rule = '(' + rule + ')';
    }
    lex.addRule(rule, TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, SingleRuleNested256Times)
{
    LexicalAnalyzer lex;
    std::string rule = "12";
    for (size_t i = 0; i < 256; ++i) {
        rule = '(' + rule + ')';
    }
    lex.addRule(rule, TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "12");
}

TEST(Group, GroupOf2Groups)
{
    LexicalAnalyzer lex;
    lex.addRule("((12)(34))", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("1234");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1234");
}

TEST(Group, TwoGroupsNested32Times)
{
    LexicalAnalyzer lex;
    std::string rule = "(12)(34)";
    for (size_t i = 0; i < 32; ++i) {
        rule = '(' + rule + ')';
    }
    lex.addRule(rule, TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("1234");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1234");
}

// ===== Union =====
TEST(Union, SimpleUnion)
{
    LexicalAnalyzer lex;
    lex.addRule("[12]", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("1");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1");
}

TEST(Union, SimpleUnionFail)
{
    LexicalAnalyzer lex;
    lex.addRule("[12]", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("3");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
}

TEST(Union, NestedUnionsEachSymStandalone)
{
    LexicalAnalyzer lex;
    lex.addRule("[[01][2][345]]", TerminalSymbol::ASSIGN_OP);
    for (size_t i = 0; i <= 5; ++i) {
        const auto parseRes = lex.parse(to_string(i));
        ASSERT_EQ(parseRes.size(), 1);
        EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[0]->text, std::to_string(i));
    }

    const auto parseRes = lex.parse("6");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
}

TEST(Union, NestedUnionsSymbolsTogether)
{
    LexicalAnalyzer lex;
    lex.addRule("[[01][2][345][6[7][89]]]", TerminalSymbol::ASSIGN_OP);

    std::string toParse;
    const size_t cnt = 10;
    for (size_t i = 0; i < cnt; ++i) {
        toParse += to_string(i);
    }

    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), cnt);
    for (size_t i = 0; i < cnt; ++i) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[i]->text, std::to_string(i));
    }
}

// ===== Asterisk =====
TEST(Asterisk, SimpleAsterisk)
{
    LexicalAnalyzer lex;
    lex.addRule("1*", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("1111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "1111");
}

TEST(Asterisk, AsteriskMatchesEmptyAndGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("1*2", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("2");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "2");
}

TEST(Asterisk, AsteriskMatchesManyCnt)
{
    LexicalAnalyzer lex;
    lex.addRule("1*", TerminalSymbol::ASSIGN_OP);
    std::string toParse;
    for (size_t i = 1; i <= 100; ++i) {
        toParse += "1";
        const auto parseRes = lex.parse(toParse);
        ASSERT_EQ(parseRes.size(), 1);
        EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[0]->text, toParse);
    }
}

TEST(Asterisk, AsteriskMatchesManyAndGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("1*2", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("11112");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "11112");
}

TEST(Asterisk, AsteriskMatchFail)
{
    LexicalAnalyzer lex;
    lex.addRule("2*", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("111111111111111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
    EXPECT_EQ(parseRes[0]->text, "");
}

TEST(Asterisk, AsteriskMatchAndGroupFail)
{
    LexicalAnalyzer lex;
    lex.addRule("2*3", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("111111111111111");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ERROR);
    EXPECT_EQ(parseRes[0]->text, "");
}

TEST(Asterisk, SimpleGroupAsterisk)
{
    LexicalAnalyzer lex;
    const std::string rule = "123456789";
    lex.addRule("(" + rule + ")*", TerminalSymbol::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }

    const auto parseRes = lex.parse(ruleDuplicated);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, ruleDuplicated);
}

TEST(Asterisk, GroupAsteriskMatchesAndFail)
{
    LexicalAnalyzer lex;
    const std::string rule = "123456789";
    lex.addRule("(" + rule + ")*", TerminalSymbol::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }
    ruleDuplicated.pop_back();

    const auto parseRes = lex.parse(ruleDuplicated);
    ASSERT_GT(parseRes.size(), 0);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, ruleDuplicated.substr(0, rule.size() * (duplicatesCnt - 1)));
    EXPECT_EQ(parseRes[1]->symbolType, TerminalSymbol::ERROR);
    EXPECT_EQ(parseRes[1]->text, "");
}

TEST(Asterisk, TwoAsterisks)
{
    LexicalAnalyzer lex;
    lex.addRule("2*3*", TerminalSymbol::ASSIGN_OP);
    const std::string toParse = "22222333333333333";
    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

TEST(Asterisk, AsteriskAndGroupMatchesLongest)
{
    LexicalAnalyzer lex;
    lex.addRule("2*", TerminalSymbol::BLANK);
    lex.addRule("2*", TerminalSymbol::CLOSED_BRACKET);
    lex.addRule("2*1", TerminalSymbol::ASSIGN_OP);
    const std::string toParse = "22222221";
    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

TEST(Asterisk, TwoAsteriskRulesFirstFails)
{
    LexicalAnalyzer lex;
    lex.addRule("3*", TerminalSymbol::BLANK);
    lex.addRule("2*", TerminalSymbol::ASSIGN_OP);
    const std::string toParse = "2222222";
    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

// ====== Dot ======
TEST(Dot, DotMatchesAnyChar)
{
    LexicalAnalyzer lex;
    lex.addRule(".", TerminalSymbol::ASSIGN_OP);
    const auto parseRes = lex.parse("2");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, "2");
}

TEST(Dot, DotMatchesOnlySingleChar)
{
    LexicalAnalyzer lex;
    lex.addRule(".", TerminalSymbol::ASSIGN_OP);
    const size_t toParseLength = 64;
    std::string toParse;
    for (size_t i = 0; i < toParseLength; ++i) {
        toParse += to_string(i % 10);
    }
    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), toParseLength);
    for (size_t i = 0; i < toParseLength; ++i) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::ASSIGN_OP);
        EXPECT_EQ(parseRes[i]->text, std::string(1, toParse[i]));
    }
}

// ====== Dot and Asterisk ======
TEST(DotAndAsterisk, DotMatchesAnything)
{
    LexicalAnalyzer lex;
    lex.addRule(".*", TerminalSymbol::ASSIGN_OP);
    const size_t toParseLength = 64;
    std::string toParse;
    for (size_t i = 0; i < toParseLength; ++i) {
        toParse += to_string(i % 10);
    }
    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

// ====== Asterisk and Union ======
TEST(AsteriskAndUnion, AsteriskMatchesEachCharFromUnion)
{
    LexicalAnalyzer lex;
    const std::string rule = "123456789";
    lex.addRule("[" + rule + "]*", TerminalSymbol::ASSIGN_OP);

    std::string toParse;
    toParse.reserve(rule.size());
    for (size_t i = 0; i < rule.size(); ++i) {
        toParse.push_back(rule[i]);
    }

    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0]->symbolType, TerminalSymbol::ASSIGN_OP);
    EXPECT_EQ(parseRes[0]->text, toParse);
}

// ====== Real tokens ======
class RealTokens : public ::testing::Test
{
protected:
    void SetUp() override
    {
        lex.addRule("[0123456789]+", TerminalSymbol::NUM_LIT);
        lex.addRule(";", TerminalSymbol::SEMICOLON);
    }
    LexicalAnalyzer lex;
};

TEST_F(RealTokens, IntegersWithSemicolons)
{
    const vector<std::string> integers = {"0",     "1", "22", "333", "4444",
                                          "55555", "6", "77", "888", "9999"};
    std::string toParse;
    for (auto integer : integers) {
        toParse += integer + ";";
    }
    const auto parseRes = lex.parse(toParse);
    ASSERT_EQ(parseRes.size() % 2, 0);
    for (size_t i = 0; i < parseRes.size(); i += 2) {
        EXPECT_EQ(parseRes[i]->symbolType, TerminalSymbol::NUM_LIT);
        EXPECT_EQ(parseRes[i]->text, integers[i / 2]);
        EXPECT_EQ(parseRes[i + 1]->symbolType, TerminalSymbol::SEMICOLON);
        EXPECT_EQ(parseRes[i + 1]->text, ";");
    }
}

int main(int argc, char **argv
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
