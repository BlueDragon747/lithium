// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "net.h"

#include "test/test_bitcoin.h"

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

static void TestLithiumSubsidySchedule(const Consensus::Params& consensusParams)
{
    BOOST_CHECK_EQUAL(GetBlockSubsidy(0, consensusParams), 5 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1, consensusParams), 48 * CENT);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1998, consensusParams), 48 * CENT);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1999, consensusParams), 48 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(174999, consensusParams), 48 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(175000, consensusParams), 24 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(349999, consensusParams), 24 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(350000, consensusParams), 12 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(524999, consensusParams), 12 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(525000, consensusParams), 6 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(649999, consensusParams), 6 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(650000, consensusParams), 3 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(799999, consensusParams), 3 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(800000, consensusParams), 150 * CENT);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(974999, consensusParams), 150 * CENT);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(975000, consensusParams), 1 * COIN);
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    TestLithiumSubsidySchedule(chainParams->GetConsensus());
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    for (int nHeight : {0, 1, 1998, 1999, 175000, 350000, 525000, 650000, 800000, 975000}) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, chainParams->GetConsensus());
        BOOST_CHECK(nSubsidy <= 48 * COIN);
        BOOST_CHECK(MoneyRange(nSubsidy));
    }
}

bool ReturnFalse() { return false; }
bool ReturnTrue() { return true; }

BOOST_AUTO_TEST_CASE(test_combiner_all)
{
    boost::signals2::signal<bool (), CombinerAll> Test;
    BOOST_CHECK(Test());
    Test.connect(&ReturnFalse);
    BOOST_CHECK(!Test());
    Test.connect(&ReturnTrue);
    BOOST_CHECK(!Test());
    Test.disconnect(&ReturnFalse);
    BOOST_CHECK(Test());
    Test.disconnect(&ReturnTrue);
    BOOST_CHECK(Test());
}
BOOST_AUTO_TEST_SUITE_END()
