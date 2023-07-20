#include "lexical_analyzer.hpp"
#include <gtest/gtest.h>
#include <string>

using namespace std;

// ===== Group =====

TEST(Group, RuleAndParseStrSame)
{
    LexicalAnalyzer lex;
    lex.addRule("1", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, RuleAndParseStrDifferent)
{
    LexicalAnalyzer lex;
    lex.addRule("1111111", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("22");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ERROR);
}

TEST(Group, RuleMatchesSeveralTimes)
{
    LexicalAnalyzer lex;
    const std::string rule = "12";
    lex.addRule(rule, Token::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }
    const auto parseRes = lex.parse(ruleDuplicated);

    EXPECT_EQ(parseRes.size(), duplicatesCnt);
    for (auto token : parseRes) {
        EXPECT_EQ(token, Token::ASSIGN_OP);
    }
}

TEST(Group, TwoRulesFirstFails)
{
    LexicalAnalyzer lex;
    lex.addRule("222", Token::BLANK);
    lex.addRule("11", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("11");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, TwoRulesMatchLongest)
{
    LexicalAnalyzer lex;
    lex.addRule("11", Token::BLANK);
    lex.addRule("1111", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1111");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, SimpleGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("(12)", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, SingleRuleNested2Times)
{
    LexicalAnalyzer lex;
    lex.addRule("((12))", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, SingleRuleNested32Times)
{
    LexicalAnalyzer lex;
    std::string rule = "12";
    for(size_t i = 0; i < 32; ++i) {
        rule = '(' + rule + ')';
    }
    lex.addRule(rule, Token::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, SingleRuleNested256Times)
{
    LexicalAnalyzer lex;
    std::string rule = "12";
    for(size_t i = 0; i < 256; ++i) {
        rule = '(' + rule + ')';
    }
    lex.addRule(rule, Token::ASSIGN_OP);
    const auto parseRes = lex.parse("12");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, GroupOf2Groups)
{
    LexicalAnalyzer lex;
    lex.addRule("((12)(34))", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1234");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Group, TwoGroupsNested32Times)
{
    LexicalAnalyzer lex;
    std::string rule = "(12)(34)";
    for(size_t i = 0; i < 32; ++i) {
        rule = '(' + rule + ')';
    }
    lex.addRule(rule, Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1234");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

// ===== Union =====
TEST(Union, SimpleUnion)
{
    LexicalAnalyzer lex;
    lex.addRule("[12]", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Union, SimpleUnionFail)
{
    LexicalAnalyzer lex;
    lex.addRule("[12]", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("3");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ERROR);
}

TEST(Union, NestedUnions)
{
    LexicalAnalyzer lex;
    lex.addRule("[[01][2][345]]", Token::ASSIGN_OP);
    for (size_t i = 0; i <= 5; ++i) {
        const auto parseRes = lex.parse(to_string(i));
        ASSERT_EQ(parseRes.size(), 1);
        EXPECT_EQ(parseRes[0], Token::ASSIGN_OP);
    }

    const auto parseRes = lex.parse("6");
    ASSERT_EQ(parseRes.size(), 1);
    EXPECT_EQ(parseRes[0], Token::ERROR);
}

// ===== Star =====
TEST(Star, SimpleStar)
{
    LexicalAnalyzer lex;
    lex.addRule("*1", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1111");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, StarMatchesEmptyAndGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("*12", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("2");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, StarMatchesMany)
{
    LexicalAnalyzer lex;
    lex.addRule("*1", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1111111");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, StarMatchesManyAndGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("*12", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("1111112");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, StarMatchFail)
{
    LexicalAnalyzer lex;
    lex.addRule("*2", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("111111111111111");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ERROR);
}

TEST(Star, StarMatchFailAndGroup)
{
    LexicalAnalyzer lex;
    lex.addRule("*23", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("111111111111111");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ERROR);
}

TEST(Star, GroupStar)
{
    LexicalAnalyzer lex;
    const std::string rule = "123456789";
    lex.addRule("*(" + rule + ")", Token::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }

    const auto parseRes = lex.parse(ruleDuplicated);
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, GroupStarMatchesAndFail)
{
    LexicalAnalyzer lex;
    const std::string rule = "123456789";
    lex.addRule("*(" + rule + ")", Token::ASSIGN_OP);

    std::string ruleDuplicated;
    const size_t duplicatesCnt = 10;
    ruleDuplicated.reserve(duplicatesCnt * rule.size());
    for (size_t i = 0; i < duplicatesCnt; ++i) {
        ruleDuplicated += rule;
    }
    ruleDuplicated.pop_back();

    const auto parseRes = lex.parse(ruleDuplicated);
    ASSERT_GT(parseRes.size(), 0);
    for (size_t i = 0; i < parseRes.size() - 1; ++i) {
        EXPECT_EQ(parseRes[i], Token::ASSIGN_OP);
    }
    ASSERT_EQ(parseRes.back(), Token::ERROR);
}

TEST(Star, TwoStars)
{
    LexicalAnalyzer lex;
    lex.addRule("*2*3", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("22222333333333333");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, StarAndGroupMatchesLongest)
{
    LexicalAnalyzer lex;
    lex.addRule("*2", Token::BLANK);
    lex.addRule("*2", Token::CLOSED_BRACKET);
    lex.addRule("*21", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("22222221");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

TEST(Star, TwoStarRulesFirstFails)
{
    LexicalAnalyzer lex;
    lex.addRule("*3", Token::BLANK);
    lex.addRule("*2", Token::ASSIGN_OP);
    const auto parseRes = lex.parse("2222222");
    ASSERT_EQ(parseRes.size(), 1);
    ASSERT_EQ(parseRes[0], Token::ASSIGN_OP);
}

// ====== Star and Union ======
TEST(StarAndUnion, StarMatchesEachCharFromUnion)
{
    LexicalAnalyzer lex;
    const std::string rule = "123456789";
    lex.addRule("*[" + rule + "]", Token::ASSIGN_OP);

    std::string toParse;
    toParse.reserve(rule.size());
    for (size_t i = 0; i < rule.size(); ++i) {
        toParse.push_back(rule[i]);
    }

    const auto parseRes = lex.parse(toParse);
    EXPECT_EQ(parseRes.size(), 1);
    for (auto token : parseRes) {
        EXPECT_EQ(token, Token::ASSIGN_OP);
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
