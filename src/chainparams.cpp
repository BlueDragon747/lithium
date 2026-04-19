// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Copyright (c) 2013-2026 The Blakecoin Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>
#include <limits>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database.
 *
 * Blakecoin Genesis Block:
 * CBlock(hash=0000000f14c5..., ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1372066561, nBits=1d00ffff, nNonce=421575, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d010445...)
 *     CTxOut(nValue=50.00000000, scriptPubKey=...)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // Lithium legacy genesis block parameters.
    const char* pszTimestamp = "London Times 9/27/14 3:20 utc Tornados armed and ready to strike Isis";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        // BEGIN BLAKECOIN: Blakecoin uses dynamic subsidy, not halving
        // Subsidy formula: 25 + sqrt(difficulty * height) BLC
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max(); // No halving
        // END BLAKECOIN
        // BEGIN BLAKECOIN: Set BIP heights to disable version checks for historical blocks
        // Blakecoin uses different block versioning - disable these checks
        consensus.BIP34Height = 100000000; // Disabled - far in future
        consensus.BIP34Hash = uint256S("0x000000000000024b89b42a942fe0d9fea3bb44ab7bd1b19115dd6a759c0808b8");
        consensus.BIP65Height = 100000000; // Disabled - far in future
        consensus.BIP66Height = 100000000; // Disabled - far in future
        // END BLAKECOIN
        // BEGIN BLAKECOIN: Proof of work limit: difficulty 1 target from nBits 0x1e00ffff
        // Target = 0x00ffff * 256^27 = 000000ffff000000000000000000000000000000000000000000000000000000
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        // Difficulty adjustment: every 20 blocks (1 hour with 3-minute blocks)
        consensus.nPowTargetTimespan = 20 * 3 * 60;       // 1 hour (20 blocks * 3 minutes)
        consensus.nPowTargetSpacing = 3 * 60;             // 3 minutes (Blakecoin)
        // END BLAKECOIN
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.fStrictChainId = true;
        consensus.nAuxpowChainId = 0x0006;
        // Preserve the legacy nominal AuxPow activation height from the 0.8.x
        // Lithium tree. Historical sync compatibility is handled in
        // validation.cpp by accepting pre-start AuxPow-bearing blocks, matching
        // legacy behavior during bootstrap import and IBD.
        consensus.nAuxpowStartHeight = 160000;
        // BEGIN BLAKECOIN: Rule change threshold for 20-block interval
        consensus.nRuleChangeActivationThreshold = 19; // 95% of 20
        consensus.nMinerConfirmationWindow = 20; // nPowTargetTimespan / nPowTargetSpacing
        // END BLAKECOIN
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Mainnet SegWit signaling starts on May 11, 2026 00:00:00 UTC and
        // times out on May 11, 2027 00:00:00 UTC.
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = 1778457600;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = 1809993600;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        // Lithium mainnet message start bytes (from legacy source main.cpp:3193)
        pchMessageStart[0] = 0xf4;
        pchMessageStart[1] = 0xa3;
        pchMessageStart[2] = 0x29;
        pchMessageStart[3] = 0xd5;
        nDefaultPort = 12007;
        nPruneAfterHeight = 100000;

        // Lithium legacy genesis block.
        genesis = CreateGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db");
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        // BlakeStream ecosystem DNS seeds — shared across all 6 coins
        vSeeds.emplace_back("blakestream.io", "seed.blakestream.io", false);
        vSeeds.emplace_back("blakecoin.org", "seed.blakecoin.org", false);

        // Lithium address prefixes from the legacy source.
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,19);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        bech32_hrp = "lit";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        // Lithium mainnet checkpoints (from lithium/src/checkpoints.cpp)
        checkpointData = (CCheckpointData) {
            {
                {0,       uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db")},
                {13000,   uint256S("0x09f2b9ccb8024bb4eaf3d230945333d2b9418bbb0602d6de8d0f81cc5035136f")},
                {19021,   uint256S("0x050190c7720c393171514350353ec7ac070bf721e79f053e521e0ea64b223d91")},
                {26012,   uint256S("0x0384667174cf11623d727993781681f7d47e94a99d85c410d425c2522ba2d928")},
                {30019,   uint256S("0x0b3c9b8156ecb23c0a851ceea23b5b635ea460c04bb6f86dfe40b5a7e524d242")},
                {84000,   uint256S("0x000b1002ef5d01182a42f341e1f2838dddb123b3ef5693476d882c123ee804cf")},
                {118009,  uint256S("0x75fd64c358f384ef700c1831209e3dc830a5cd9cd8a3ab7694897f950967dc20")},
                {139452,  uint256S("0x56d154ab3ddf6968c529e0a880af0caa438579fa20f30dcaa260802df830b7de")},
                {535001,  uint256S("0x34f47f7dc07805535442a7a765f9cf36baae1de657212d57e5e3ae6696eaaf7e")},
                {1250000, uint256S("0xbdd59175ea75a7a6b99d93678a74c24a0f61ffe8ebde02b8cc1665e5a3eb9e79")},
            }
        };

        chainTxData = ChainTxData{
            // Data as of block 1250000 (last checkpoint, from lithium/src/checkpoints.cpp)
            1646262870, // * UNIX timestamp of last checkpoint block
            1628063,    // * total number of transactions between genesis and last checkpoint
            2800.0 / (24 * 60 * 60) // * ~2800 tx/day expressed as tx/sec
        };
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        // Keep testnet aligned with Blakecoin mainnet instead of inherited Bitcoin defaults.
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max(); // No halving
        consensus.BIP34Height = 100000000; // Disabled - far in future
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 100000000; // Disabled - far in future
        consensus.BIP66Height = 100000000; // Disabled - far in future
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 20 * 3 * 60; // 1 hour (20 blocks * 3 minutes)
        consensus.nPowTargetSpacing = 3 * 60;             // 3 minutes (Blakecoin)
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0006;
        consensus.nAuxpowStartHeight = 0;
        consensus.nRuleChangeActivationThreshold = 15; // 75% of 20
        consensus.nMinerConfirmationWindow = 20;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // Deployment of SegWit (BIP141, BIP143, and BIP147)
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x0000000000000000000000000000000000000000000000000000000000000000");

        // Lithium testnet message start bytes (from legacy source main.cpp:2863-2866)
        pchMessageStart[0] = 0x0d;
        pchMessageStart[1] = 0x15;
        pchMessageStart[2] = 0x04;
        pchMessageStart[3] = 0x0c;
        nDefaultPort = 12000;
        nPruneAfterHeight = 1000;

        // Legacy Lithium source reused the same genesis on testnet.
        genesis = CreateGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db");
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // Testnet seeds to be added when available

        // Lithium testnet address prefixes from the legacy source.
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,170);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
        bech32_hrp = "tlit";

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        // BEGIN BLAKECOIN: Testnet checkpoints
        checkpointData = (CCheckpointData) {
            {
                {0, uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db")},
            }
        };
        // END BLAKECOIN

        chainTxData = ChainTxData{
            // Data as of testnet genesis
            1411788333,
            1,
            0.01
        };

    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max(); // No halving (Lithium uses height-based tiers)
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        // Keep regtest useful for local testing, but use Blakecoin's 3-minute cadence.
        consensus.nPowTargetTimespan = 20 * 3 * 60; // 1 hour
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0006;
        consensus.nAuxpowStartHeight = 0;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].bit = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_SEGWIT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        // Regtest reuses the legacy Lithium genesis for local-only testing.
        genesis = CreateGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db");
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        // BEGIN BLAKECOIN: Regtest checkpoint
        checkpointData = (CCheckpointData){
            {
                {0, uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db")}
            }
        };
        // END BLAKECOIN

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        // Regtest uses Lithium's mainnet address prefix for local testing.
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,19);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        bech32_hrp = "rlit";
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
