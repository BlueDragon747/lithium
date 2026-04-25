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
    const int act = consensusParams.nSubsidyLadderActivationHeight;
    // Genesis is always the historical 5 LIT.
    BOOST_CHECK_EQUAL(GetBlockSubsidy(0, consensusParams), 5 * COIN);
    // Pre-activation: 50 LIT flat (preserves the 0.15.21 chain history mined
    // before the ladder fork). Heights inside legacy ladder ranges still pay
    // the flat amount until activation.
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1, consensusParams), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(1998, consensusParams), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(175000, consensusParams), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(975000, consensusParams), 50 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(act - 1, consensusParams), 50 * COIN);
    // At/after activation: legacy 0.8 lithium height-tier ladder. Activation
    // lives past 975000 on mainnet so the long-tail tier (1 LIT) applies.
    BOOST_CHECK_EQUAL(GetBlockSubsidy(act, consensusParams), 1 * COIN);
    BOOST_CHECK_EQUAL(GetBlockSubsidy(act + 1000, consensusParams), 1 * COIN);
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    TestLithiumSubsidySchedule(chainParams->GetConsensus());
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    const int act = chainParams->GetConsensus().nSubsidyLadderActivationHeight;
    for (int nHeight : {0, 1, 1998, 175000, 975000, act - 1, act, act + 1000, 5000000}) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, chainParams->GetConsensus());
        BOOST_CHECK(nSubsidy <= 50 * COIN);
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
