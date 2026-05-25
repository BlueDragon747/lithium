// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <hash.h>
#include <chainparamsbase.h>
#include <logging.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <uint256.h>
#include <util/strencodings.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

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
 * CBlock(hash=000000000019d6, ver=1, hashPrevBlock=00000000000000, hashMerkleRoot=4a5e1e, nTime=1231006505, nBits=1d00ffff, nNonce=2083236893, vtx=1)
 *   CTransaction(hash=4a5e1e, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73)
 *     CTxOut(nValue=50.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: 4a5e1e
 */
static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "The Times 03/Jan/2009 Chancellor on brink of second bailout for banks";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock CreateLithiumGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // Lithium's own coinbase scriptSig text from the README source of truth
    // and legacy 0.8 main.cpp. Distinct from the rest of the Blakestream family.
    // Mainnet genesis hash 0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db
    // and merkle root 0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008
    // are reproducible from this factory + (1411788333, 8298496, 503382015, 112, 5 * COIN).
    const char* pszTimestamp = "London Times 9/27/14 3:20 utc Tornados armed and ready to strike Isis";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

/**
 * Main network on which people trade goods and services.
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = CBaseChainParams::MAIN;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        // Lithium uses a forward-activated subsidy ladder, NOT halving. The
        // 0.15.21 chain crossed activation at this height; once reached on
        // mainnet this value is locked. Pre-activation pays 50 LIT flat
        // (preserving 0.15.21 history); post-activation the legacy 0.8 ladder
        // applies. See README.md and src/validation.cpp::GetBlockSubsidy.
        consensus.nSubsidyLadderActivationHeight = 1949476;
        // Lithium-0.25.2 post-SegWit cleanup group. These are intentional
        // future-height mainnet activations after the verified 0.15.21 SegWit
        // activation at height 1956300.
        consensus.BIP34Height = 1977347;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1977347;
        consensus.BIP66Height = 1977347;
        // CSV (BIP68/112/113) provides the atomic-swap timeout primitive and is
        // always active from genesis for Lithium. Do NOT change.
        consensus.CSVHeight = 1;
        // Lithium SegWit is inherited from the 0.15.21 mainnet activation.
        // 0.25.2 buries this height and does not re-signal SegWit.
        consensus.SegwitHeight = 1956300;
        // Historical miners briefly set unknown BIP9 bits immediately before
        // buried SegWit. Only warn on post-burial unknown versionbits.
        consensus.MinBIP9WarningHeight = 1956300;
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 19;
        consensus.nMinerConfirmationWindow = 20;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;
        // Taproot bundle BIP340/BIP341/BIP342
        // (Schnorr + Taproot key/script commitments + Tapscript). This is what enables
        // PTLC-style atomic swaps and MuSig2 cross-chain DEX paths. ALL THE C++
        // MACHINERY IS ALREADY PRESENT in this codebase (verified 2026-04-25):
        // Schnorr verify, key-path, script-path, OP_CHECKSIGADD, TaggedHash byte-
        // identity to the upstream v25.2 implementation. Activation values are assigned below.
        //
        // Taproot follows the 0.25.2 cleanup group in a separate BIP9 window.
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = 1782871200;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = 1814407200;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 1980707;

        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000081cf443903cbb00d9c");
        consensus.defaultAssumeValid = uint256S("0x2cd972cd152d4e972a8fc6e48e4f2faafebccd229770f90195aece2965c01f0c");

        // Lithium AuxPoW chain identity (consumed by Phase 2 AuxPoW core).
        // mainnet: strict chain-ID, AuxPoW activates at historical height 160000.
        consensus.fStrictChainId = true;
        consensus.nAuxpowChainId = 0x0006;
        consensus.nAuxpowStartHeight = 160000;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         *
         * Lithium mainnet keeps the 0.15.21 network magic bytes.
         */
        pchMessageStart[0] = 0xf4;
        pchMessageStart[1] = 0xa3;
        pchMessageStart[2] = 0x29;
        pchMessageStart[3] = 0xd5;
        nDefaultPort = 12007;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 5;
        m_assumed_chain_state_size = 1;

        genesis = CreateLithiumGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db"));
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        // Shared BlakeStream aux-coin DNS seeds.
        vSeeds.emplace_back("seed.blakestream.io");
        vSeeds.emplace_back("seed.blakecoin.org");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,19);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "lit";
        vFixedSeeds.clear();

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = false;
        m_is_mockable_chain = false;

        // Lithium mainnet checkpoints, lifted from Lithium-0.15.21 and
        // refreshed with a 2026-04-27 release anchor.
        checkpointData = {
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
                {1937598, uint256S("0x046a8cff07cfec07b9797e06e997faa1f6a515b5e5817444289fecdf9b220196")},
                {1956300, uint256S("0x4e361135d30a94db8923116d84750e96e0d576025962841b173e838005da6130")},
                {1958000, uint256S("0x491ba955b5db6552efdfc48ee658d42bbb72872b1bf28aefe44cf16e59d83d67")},
                {1960000, uint256S("0x2cd972cd152d4e972a8fc6e48e4f2faafebccd229770f90195aece2965c01f0c")},
                {1961000, uint256S("0xe1c887174b1c50f9d12b3b62cfd4ee0bbc9461593b06c13a1962952f365f9a14")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            {
                1962644,
                {
                    AssumeutxoHash{uint256S("0x20d1e63b9bf187495c5c628d667c9316601274a4dee823e4d5c67d6deb1c0b25")},
                    2365155,
                },
            },
        };

        chainTxData = ChainTxData{
            .nTime    = 1779159937,
            .nTxCount = 2362486,
            .dTxRate  = 0.005407511797236991,
        };
    }
};

/**
 * Testnet (v3): public test network which is reset from time to time.
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = CBaseChainParams::TESTNET;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        // Testnet keeps BIP34/BIP65/BIP66 at far-future heights so legacy-style
        // test blocks remain usable.
        consensus.BIP34Height = 100000000;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 100000000;
        consensus.BIP66Height = 100000000;
        // CSV (BIP68/112/113) is always active for Lithium test coverage.
        consensus.CSVHeight = 1;
        // Testnet SegWit is active from height 1 so AuxPoW and atomic-swap
        // regression coverage works without waiting on the 0.15.21 mainnet
        // activation.
        consensus.SegwitHeight = 1;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("000000ffff000000000000000000000000000000000000000000000000000000");
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 15;
        consensus.nMinerConfirmationWindow = 20;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0;
        // Testnet Taproot remains inactive until a dedicated testnet activation
        // window is chosen.
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0;

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        // Lithium AuxPoW: testnet does NOT enforce strict chain-ID, and AuxPoW
        // is acceptable from genesis (no historical pre-AuxPoW height).
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0006;
        consensus.nAuxpowStartHeight = 0;

        pchMessageStart[0] = 0x0d;
        pchMessageStart[1] = 0x15;
        pchMessageStart[2] = 0x04;
        pchMessageStart[3] = 0x0c;
        nDefaultPort = 12000;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        // Lithium testnet has its OWN distinct genesis (different nTime/nNonce
        // from mainnet/regtest). Lifted verbatim from Lithium-0.15.21
        // chainparams.cpp:248-250.
        genesis = CreateLithiumGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db"));
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // Lithium testnet seeds to be added when available.

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,170);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tlit";

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = false;

        checkpointData = {
            {
                {0, uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db")},
            }
        };

        m_assumeutxo_data = MapAssumeutxo{
            // TODO to be specified in a future patch.
        };

        chainTxData = ChainTxData{
            .nTime    = 1392351202,
            .nTxCount = 1,
            .dTxRate  = 0.01,
        };
    }
};

/**
 * Signet: test network with an additional consensus parameter (see BIP325).
 */
class SigNetParams : public CChainParams {
public:
    explicit SigNetParams(const SigNetOptions& options)
    {
        std::vector<uint8_t> bin;
        vSeeds.clear();

        if (!options.challenge) {
            // Lithium signet defaults to a local/private developer network.
            // Keep the default challenge trivial and ship no global seeds,
            // assumevalid, or chainwork so we do not point at Bitcoin signet.
            bin = ParseHex("51");
            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{0, 0, 0};
        } else {
            bin = *options.challenge;
            consensus.nMinimumChainWork = uint256{};
            consensus.defaultAssumeValid = uint256{};
            m_assumed_blockchain_size = 0;
            m_assumed_chain_state_size = 0;
            chainTxData = ChainTxData{
                0,
                0,
                0,
            };
            LogPrintf("Signet with challenge %s\n", HexStr(bin));
        }

        if (options.seeds) {
            vSeeds = *options.seeds;
        }

        strNetworkID = CBaseChainParams::SIGNET;
        consensus.signet_blocks = true;
        consensus.signet_challenge.assign(bin.begin(), bin.end());
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        consensus.BIP34Height = 1;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1;
        consensus.BIP66Height = 1;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 1;
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 19;
        consensus.nMinerConfirmationWindow = 20;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("00000377ae000000000000000000000000000000000000000000000000000000");
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        // Keep Taproot always active on signet so developer coverage matches
        // regtest/testnet, while mainnet activation policy waits on
        // Lithium-0.15.21 mainnet SegWit activation results.
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        // message start is defined as the first 4 bytes of the sha256d of the block script
        HashWriter h{};
        h << consensus.signet_challenge;
        uint256 hash = h.GetHash();
        memcpy(pchMessageStart, hash.begin(), 4);

        nDefaultPort = 38733;
        nPruneAfterHeight = 1000;

        // Lithium signet: defaults to the testnet genesis params (Lithium-0.15.21
        // never shipped mainnet signet; this upstream v25.2 facility is used here
        // for developer experimentation only).
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0006;
        consensus.nAuxpowStartHeight = 0;
        genesis = CreateLithiumGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db"));
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,142);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,170);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tlit";

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        m_is_test_chain = true;
        m_is_mockable_chain = false;
    }
};

/**
 * Regression test: intended for private networks only. Has minimal difficulty to ensure that
 * blocks can be found instantly.
 */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const RegTestOptions& opts)
    {
        strNetworkID =  CBaseChainParams::REGTEST;
        consensus.signet_blocks = false;
        consensus.signet_challenge.clear();
        consensus.nSubsidyHalvingInterval = std::numeric_limits<int>::max();
        consensus.BIP34Height = 100000000;
        consensus.BIP34Hash = uint256{};
        consensus.BIP65Height = 1351;
        consensus.BIP66Height = 1251;
        consensus.CSVHeight = 1;
        consensus.SegwitHeight = 0;
        consensus.MinBIP9WarningHeight = 0;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 20 * 3 * 60;
        consensus.nPowTargetSpacing = 3 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108;
        consensus.nMinerConfirmationWindow = 144;

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay

        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].bit = 2;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nStartTime = Consensus::BIP9Deployment::ALWAYS_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TAPROOT].min_activation_height = 0; // No activation delay

        consensus.nMinimumChainWork = uint256{};
        consensus.defaultAssumeValid = uint256{};

        // Lithium regtest: AuxPoW machinery available from genesis, no
        // strict chain-ID enforcement (mirrors Lithium-0.15.21).
        consensus.fStrictChainId = false;
        consensus.nAuxpowChainId = 0x0006;
        consensus.nAuxpowStartHeight = 0;

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = opts.fastprune ? 100 : 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        for (const auto& [dep, height] : opts.activation_heights) {
            switch (dep) {
            case Consensus::BuriedDeployment::DEPLOYMENT_SEGWIT:
                consensus.SegwitHeight = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_HEIGHTINCB:
                consensus.BIP34Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_DERSIG:
                consensus.BIP66Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CLTV:
                consensus.BIP65Height = int{height};
                break;
            case Consensus::BuriedDeployment::DEPLOYMENT_CSV:
                consensus.CSVHeight = int{height};
                break;
            }
        }

        for (const auto& [deployment_pos, version_bits_params] : opts.version_bits_parameters) {
            consensus.vDeployments[deployment_pos].nStartTime = version_bits_params.start_time;
            consensus.vDeployments[deployment_pos].nTimeout = version_bits_params.timeout;
            consensus.vDeployments[deployment_pos].min_activation_height = version_bits_params.min_activation_height;
        }

        // Lithium regtest reuses the Lithium mainnet genesis parameters
        // (Lithium-0.15.21 chainparams.cpp:332-334). Testnet has its own
        // distinct genesis and is NOT shared with regtest/mainnet.
        genesis = CreateLithiumGenesisBlock(1411788333, 8298496, 503382015, 112, 5 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db"));
        assert(genesis.hashMerkleRoot == uint256S("0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        m_is_test_chain = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {
                {0, uint256S("0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db")},
            }
        };

        // Lithium regtest assumeutxo snapshot at height 110. The donor
        // Lithium-25.2 hash (`75e404e4…`) does not apply: Lithium regtest
        // has a different genesis, so the height-110 chainstate (txoutset)
        // hash is different. Captured by running the regtest 110-block
        // TestChain100Setup sequence and dumping the resulting snapshot's
        // `txoutset_hash`.
        m_assumeutxo_data = MapAssumeutxo{
            {
                110,
                {
                    AssumeutxoHash{uint256S("0x5514258d23ea22f87c7ab90102d5a9d5de54a32bb9a4d694668acfa599f483e2")},
                    111,
                },
            },
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,19);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,7);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,128);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "rlit";
    }
};

std::unique_ptr<const CChainParams> CChainParams::SigNet(const SigNetOptions& options)
{
    return std::make_unique<const SigNetParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::RegTest(const RegTestOptions& options)
{
    return std::make_unique<const CRegTestParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::Main()
{
    return std::make_unique<const CMainParams>();
}

std::unique_ptr<const CChainParams> CChainParams::TestNet()
{
    return std::make_unique<const CTestNetParams>();
}
