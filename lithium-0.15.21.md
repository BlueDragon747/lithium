# Lithium 0.15.2 Update — Source of Truth

## Overview

Port Lithium (LIT) from its current 0.8.9.7 codebase to Bitcoin Core 0.15.2, following the same approach used for the Blakecoin 0.15.2 update (`Blakecoin-0.15.21`).

**Reference codebase:** `../Blakecoin-0.15.21/` — the completed Blakecoin port to 0.15.2
**Original codebase:** `../lithium/` — current 0.8.9.7 source with all coin-specific parameters

---

## QC Status

- This file is not implementation-safe without the QC corrections in this section.
- The current 0.15.2 tree builds and passes `test/functional/wallet-basic-smoke.py` on regtest with no sends, no funding, and no mining.
- Corrected from source: Lithium does have AuxPow code in `../lithium/src`, with legacy AuxPow start height `160000` on mainnet and chain ID `0x0006`.
- Corrected from source: secret-key prefixes are `128` mainnet and `239` testnet, not `TBD`.
- Corrected from source: the first low-reward range is not a simple `height < 2000` rule in prose. Legacy code flips from `0.48 * COIN` to `48 * COIN` at height `1999`, so reward boundaries must be ported from code, not from the simplified table.
- Corrected from source: Lithium already has multiple checkpoints in `../lithium/src/checkpoints.cpp`; the old "genesis only" note is not accurate.
- Follow-up completed: the legacy `height >= 150000` full-interval retarget lookback rule has now been restored in `src/pow.cpp`, so the remaining PoW gap is AuxPow rather than base retarget math.
- The shared AuxPow framework is now integrated: `src/auxpow.{h,cpp}`, `src/primitives/pureheader.{h,cpp}`, AuxPow-aware block/header serialization, disk index persistence, AuxPow-aware PoW validation, and chain-ID-aware block template versions.
- Live daemon QA correction: current mainnet bootstrap and peer replay reaches height `913` and then rejects header `d46155202c78c74f47feb862710ae8c83932d4507bd1d59f922f379f84d857bf` as `early-auxpow-block` if the port uses a modern pre-start header reject. The 0.15.2 port therefore keeps the legacy nominal AuxPow start height `160000`, but restores the 0.8.x compatibility rule that tolerates AuxPoW-bearing blocks before that height during sync.
- Testnet and regtest use chain ID `0x0006` with start height `0` and strict chain ID disabled for local QA.
- Verified after the fan-out: `make -C src -j4 blakecoind blakecoin-cli` succeeds and `test/functional/test_runner.py --jobs=1 wallet-basic-smoke.py` passes on regtest.
- Historical validation compatibility is now preserved: the legacy Lithium 0.8 tree only enforced the BIP30 overwrite rule in `!pindex->phashBlock` contexts, so the 0.15.2 port must not silently restore Bitcoin Core's broader duplicate-txid rejection path.
- Merged-mining RPC direction is now fixed for this port: primary RPCs are `createauxblock <address>` plus `submitauxblock <hash> <auxpow>`, with `getauxblock` kept only as a compatibility wrapper for older pool software. `getworkaux` is intentionally out of scope unless a real dependency is later proven.
- Keep this repo on a strict no-send / no-mine mainnet rule until the final production pool and Electrium carry-back staging are complete.

### BlakeStream Seed And AuxPoW RPC Policy

- BlakeStream DNS seeds (`seed.blakestream.io`, `seed.blakecoin.org`) are shared across all six coins and serve nodes for ALL coins. A single seed lookup returns peer IPs regardless of which coin is asking; coin separation happens at the wire-protocol layer via each coin's unique `pchMessageStart` and default port.
- The production direction for this repo is a modern merged-mining RPC surface: `createauxblock` to build the child-chain template and `submitauxblock` to submit the solved AuxPoW payload.
- `createauxblock` is address-driven on purpose so a pool can choose the child-chain payout script explicitly instead of depending on wallet mining state inside the daemon.
- `getauxblock` remains only as a compatibility mode for older merged-mining software. It should map onto the same block-template / block-submit flow rather than preserving a separate legacy implementation path.
- `getworkaux` is not part of the planned 0.15.2 target. We are not reviving `getwork`-era RPC unless a live pool or deployment proves it is still required.
- The same 2 DNS seeds (`seed.blakestream.io`, `seed.blakecoin.org`) are used by all six coins. This matches the Blakecoin 0.15.2 reference repo exactly.

### Wire Checksum Policy

- Lithium should preserve the legacy `Hashblake` P2P message checksum behavior for current network interoperability.
- Do not normalize Lithium to Blakecoin's temporary non-`Hashblake` handshake exception.
- Keep Blakecoin documented as the one current exception; Lithium stays on `Hashblake` before go-live unless a fresh compatibility review says otherwise.

## AuxPoW Start And Completed Work

| Network | Chain ID | Nominal AuxPoW Start | Observed Pre-Start AuxPoW Evidence In Current QA | Exact Time/Date Status | 0.15.2 Port Rule |
|---------|----------|----------------------|-----------------------------------------------|------------------------|------------------|
| Mainnet | `0x0006` | `160000` | Replay reached height `913` before hitting the old `early-auxpow-block` reject | Exact timestamp remains archival-only; compatibility rule is already proven by replay | Keep `160000` as the nominal legacy value, but tolerate earlier historical AuxPoW-bearing blocks during bootstrap / IBD |
| Testnet | `0x0006` | `0` | N/A | Local QA only | AuxPoW enabled for local QA; strict chain ID disabled |
| Regtest | `0x0006` | `0` | N/A | Local QA only | AuxPoW enabled for local QA; strict chain ID disabled |

Interpretation note:
`160000` is still the nominal legacy mainnet AuxPoW start height preserved in `chainparams.cpp`. The fix here was not to move activation earlier. The fix was to preserve historical chain acceptance by removing the blanket pre-start reject that did not match Lithium's preserved chain history.

- Completed in this repo:
  - Integrated the shared AuxPoW framework with `src/auxpow.{h,cpp}` and `src/primitives/pureheader.{h,cpp}`.
  - Ported AuxPoW-aware block/header serialization, disk index persistence, block version handling, and PoW validation.
  - Kept Lithium's nominal mainnet start height at `160000` while removing the modern `early-auxpow-block` reject that broke historical chain acceptance around height `913`.
  - Restored the legacy `height >= 150000` full-interval retarget behavior in `src/pow.cpp` so the remaining consensus focus stays on AuxPoW compatibility rather than base retarget math.
  - Implemented the modern merged-mining RPC direction: `createauxblock <address>` and `submitauxblock <hash> <auxpow>`, with `getauxblock` retained only as a compatibility wrapper.
  - Verified no-send regtest wallet smoke coverage and refreshed Ubuntu 24 daemon/Qt builds after the compatibility correction.
- Operational rule:
  - Keep the strict no-send / no-mine mainnet rule in place while final production pool and Electrium carry-back staging continue.

---

## Coin Identity

| Parameter | Value |
|-----------|-------|
| Coin Name | Lithium |
| Ticker | LIT |
| Algorithm | Blake-256 (8 rounds) |
| Merge Mining | Yes (AuxPow; legacy nominal start 160000 with pre-start compatibility preserved) |
| Base Version (current) | 0.8.9.7 (forked from Photon/Bitcoin 0.8.5) |
| Target Version | 0.15.2 |

---

## Chain Parameters to Preserve

### Network

| Parameter | Mainnet | Testnet |
|-----------|---------|---------|
| P2P Port | 12007 | 12000 |
| RPC Port | 12000 | 12004 |
| pchMessageStart | 0xf4, 0xa3, 0x29, 0xd5 | 0x0d, 0x15, 0x04, 0x0c |

**WARNING: P2P and RPC ports overlap!** Mainnet RPC (12000) = Testnet P2P (12000). Verify this is correct in the original source — it may cause conflicts if running both networks on the same machine.

### Address Prefixes

| Type | Mainnet | Testnet |
|------|---------|---------|
| Pubkey Address | 19 (0x13) | 142 (0x8E) |
| Script Address | 7 (0x07) | 170 (0xAA) |
| Secret Key | 128 (0x80) | 239 (0xEF) |
| Bech32 HRP | `lit` | `tlit` |

### Block Parameters

| Parameter | Value |
|-----------|-------|
| Block Time | 180 seconds (3 minutes) |
| Target Timespan | 3,600 seconds (60 minutes) |
| Retarget Interval | 20 blocks |
| Coinbase Maturity | 120 blocks |
| Max Supply | 25,228,800 LIT |
| COIN | 100,000,000 satoshis |
| PoW Limit | ~uint256(0) >> 24 |

### Block Reward Schedule (CRITICAL — Multi-Tier Non-Standard)

Lithium uses a **complex multi-tier reward schedule**, NOT standard halving:

| Block Range | Reward | Notes |
|-------------|--------|-------|
| Genesis (block 0) | 5 LIT | |
| 1 — 1,998 | 0.48 LIT | Reduced launch phase |
| 1,999 — 174,999 | 48 LIT | Main emission |
| 175,000 — 349,999 | 24 LIT | Step-down #1 |
| 350,000 — 524,999 | 12 LIT | Step-down #2 |
| 525,000 — 649,999 | 6 LIT | Step-down #3 |
| 650,000 — 799,999 | 3 LIT | Step-down #4 |
| 800,000 — 974,999 | 1.5 LIT | Step-down #5 |
| 975,000+ | 1 LIT | Permanent tail emission |

**This reward logic has been ported exactly to `GetBlockSubsidy()` in `src/validation.cpp`.**

### Difficulty Adjustment (CUSTOM)

- Max ±15% change per retarget until block 3,500
- Max ±3% change per retarget after block 3,500
- This custom difficulty clamping must be preserved in the 0.15.2 `CalculateNextWorkRequired()` or equivalent

### Genesis Block

| Parameter | Value |
|-----------|-------|
| Hash | `0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db` |
| Merkle Root | `0x2a1fd3e405c93b2011b4189e97cd658b1f5ca859eba01723af28dd21f4d6c008` |
| nTime | 1411788333 (Sept 27, 2014) |
| nNonce | 8,298,496 |
| nVersion | 112 |
| nBits | 0x1d00ffff |
| Coinbase Message | "London Times 9/27/14 3:20 utc Tornados armed and ready to strike Isis" |

### DNS Seeds (IP-based)

- 72.23.74.166
- 62.219.234.143
- 78.26.209.208
- 77.121.61.203

Note: These are IP addresses, not DNS hostnames. In 0.15.2, these should be added as fixed seeds or converted to proper DNS seed entries.

### Checkpoints

| Block | Hash |
|-------|------|
| 0 | `0x000000fcf39055b547e94e610f1008b8046f942bbb730e8b6dfa6232931902db` |

(Only genesis checkpoint found in source — may need to add more from a running node)

---

## What Needs to Be Done

### Phase 1: Copy & Rebrand the Blakecoin 0.15.2 Base

1. **Copy** the entire `Blakecoin-0.15.21` codebase into this directory
2. **Rename** all Blakecoin references → Lithium:
   - Binary names: `lithiumd`, `lithium-qt`, `lithium-cli`, `lithium-tx`
   - Config file: `lithium.conf`, config dir `~/.lithium/`
   - URI scheme: `lithium://`
   - Desktop entry, icons, window titles
   - `configure.ac`: package name, version
   - Window title: "Lithium-Qt"
   - Organization: "Lithium" / "Lithium.org"

### Phase 2: Apply Coin-Specific Parameters

3. **`src/chainparams.cpp`** — Replace ALL chain parameters:
   - Genesis block (hash, merkle root, nTime=1411788333, nNonce=8298496, coinbase message)
   - Network ports (P2P: 12007, RPC: 12000)
   - Message start bytes (0xf4, 0xa3, 0x29, 0xd5)
   - Address prefixes (pubkey: 19, script: 7)
   - Block timing (180s block time, 60-min retarget, 20-block interval)
   - **Disable halving interval** (set to max) — rewards are height-range based
   - Checkpoints
   - Bech32 HRP
   - Fixed seed IPs instead of DNS seeds

4. **`src/amount.h`** — MAX_MONEY = 25,228,800 * COIN

5. **`src/validation.cpp`** — Block reward logic (**CUSTOM — 8 TIERS**):
   ```
   if (height == 0) return 5 * COIN;
   if (height < 1999) return 48000000;     // 0.48 LIT
   if (height < 175000) return 48 * COIN;
   if (height < 350000) return 24 * COIN;
   if (height < 525000) return 12 * COIN;
   if (height < 650000) return 6 * COIN;
   if (height < 800000) return 3 * COIN;
   if (height < 975000) return 150000000;  // 1.5 LIT
   return 1 * COIN;
   ```

6. **`src/pow.cpp`** — Custom difficulty adjustment:
   - Port the ±15% / ±3% clamping logic from 0.8.x `GetNextWorkRequired()`
   - Threshold at block 3,500

7. **`src/consensus/consensus.h`** — COINBASE_MATURITY = 120

8. **`src/qt/`** — Update all GUI branding for Lithium

### AuxPow / Merge Mining Status

- The shared AuxPow framework from the BlakeBitcoin 0.15.2 reference is now integrated in this repo.
- Mainnet uses strict chain ID `0x0006` with legacy nominal AuxPow start height `160000` in the 0.15.2 port, while preserving the 0.8.x pre-start AuxPoW compatibility rule for historical sync.
- Testnet and regtest activate AuxPow at height `0` for local QA and keep strict chain ID disabled.
- The modern merged-mining RPC path is now implemented and no-send regtest-smoke verified for `createauxblock <address>` plus compatibility `getauxblock`.
- Historical merged-mined header compatibility is now documented from preserved replay evidence; exact activation dating remains archival-only and is no longer the release blocker.

### Phase 3: Build System

9. **`build.sh`** — Update all variables:
   - COIN_NAME: "lithium"
   - DAEMON_NAME: "lithiumd"
   - QT_NAME: "lithium-qt"
   - CLI_NAME: "lithium-cli"
   - TX_NAME: "lithium-tx"
   - VERSION: "0.15.2"
   - RPC_PORT: 12000
   - P2P_PORT: 12007

10. **Docker configs** — Same Docker images as Blakecoin 0.15.2

### Phase 4: SegWit Activation

11. **Mainnet SegWit rollout** — Mainnet versionbits signaling starts on May 11, 2026 00:00:00 UTC (`1778457600`) and times out on May 11, 2027 00:00:00 UTC (`1809993600`).
12. **Activation semantics** — May 11, 2026 is the signaling start date, not guaranteed same-day activation. Actual mainnet SegWit enforcement still depends on miner signaling and BIP9 lock-in.
13. **CSV / test networks** — CSV stays `ALWAYS_ACTIVE`, and testnet/regtest keep `ALWAYS_ACTIVE` SegWit for controlled QA and wallet validation.
14. **BIP34/65/66** — Disable version checks

---

## Key Differences from Blakecoin

| Aspect | Blakecoin | Lithium |
|--------|-----------|---------|
| Block Time | 180s (3 min) | 180s (3 min) — SAME |
| Retarget Interval | 20 blocks | 20 blocks — SAME |
| Retarget Timespan | 1 hour | 1 hour — SAME |
| Reward Model | Dynamic formula | 8-tier height-based |
| Max Supply | 21M | 25,228,800 |
| Coinbase Maturity | ??? | 120 blocks |
| P2P Port | 8773 | 12007 |
| RPC Port | 8772 | 12000 |
| Pubkey Address | 26 | 19 |
| Merge Mining | No (in donor Blakecoin 0.15.2) | Yes (AuxPow at block 160,000 on mainnet) |
| Difficulty Clamping | Standard | ±15% then ±3% |
| Genesis Date | Oct 2013 | Sept 2014 |

Note: Block time and retarget are identical to Blakecoin, which simplifies the port. The main differences are the reward schedule and difficulty clamping.

---

## Potential Issues & Gotchas

1. **Port conflict** — Mainnet RPC port (12000) = Testnet P2P port (12000). This is how the original works, but it's worth noting. Users can't run mainnet and testnet simultaneously without overriding ports.
2. **8-tier reward schedule** — Complex reward function with specific height boundaries. Must match exactly or supply will diverge.
3. **0.48 LIT early blocks** — The early-phase reduced reward (blocks 1-1998) uses a fractional value. Ensure precision is maintained (0.48 * COIN = 48,000,000 satoshis).
4. **1.5 LIT tier** — Another fractional reward. 1.5 * COIN = 150,000,000 satoshis.
5. **Custom difficulty clamping** — The ±15%/±3% logic is non-standard and must be ported to 0.15.2's `pow.cpp`. This is the consensus-critical piece beyond the reward schedule.
6. **IP-based seeds** — No DNS seeds, only hardcoded IPs. These may be stale. Consider setting up proper DNS seed infrastructure.
7. **AuxPow follow-on work** — The shared AuxPow framework is now ported and the modern merged-mining RPC surface is in place. The former production pool / Electrium carry-back staging gate is now green.
8. **Historical BIP30 rule** — The original Lithium chain relaxed BIP30 during normal block connection (`!pindex->phashBlock` only). Keep that legacy behavior unless a full historical replay proves the broader 0.15.2 rule is safe.
9. **Checkpoint follow-up** — The legacy repo already has multiple checkpoints; if more are needed later, extract them from a fully synced historical node rather than assuming genesis-only.

---

## Build & Test Plan

1. Rebuild native Linux after any consensus change and keep `blakecoind` / `blakecoin-cli` green
2. Re-run `test/functional/test_runner.py --jobs=1 wallet-basic-smoke.py` on regtest after consensus-touching edits
3. Verify genesis block hash, address generation, and RPC defaults on port `12000`
4. Re-check reward boundaries at heights `0`, `1`, `1999`, `175000`, `350000`, `525000`, `650000`, `800000`, and `975000`
5. Re-check difficulty clamping and the restored `height >= 150000` full-interval lookback rule
6. Preserve the archived historical replay notes and do not reintroduce the rejected `early-auxpow-block` rule before any mainnet activity
7. Build AppImage, Windows, and macOS artifacts after consensus validation is stable

---

## Verified Snapshot

- Native Linux rebuild succeeded for `blakecoind` and `blakecoin-cli`.
- Fresh regtest no-send smoke passed for `getnewaddress`, `createauxblock <address>`, and compatibility `getauxblock`.
- Verified AuxPow template `chainid` returned as `6` on fresh regtest, matching `consensus.nAuxpowChainId`.
- Verified from `../lithium/src/main.cpp`: legacy Lithium keeps BIP30 relaxed with `bool fEnforceBIP30 = !pindex->phashBlock;`, and the 0.15.2 port now preserves that historical validation behavior while full mainnet replay QA remains pending.
- Direct `createauxblock` plus `submitauxblock` acceptance is now proven in isolated QA. The production carry-back staging that used to block release is now green.

---

## File Reference

| What | Where |
|------|-------|
| Reference (completed) | `../Blakecoin-0.15.21/` |
| Original coin source | `../lithium/` |
| Original params | `../lithium/src/main.cpp`, `../lithium/src/main.h` |
| Original build script | `../lithium/build.sh` |
| Qt project file | `../lithium/lithium-qt.pro` |
| Address prefixes | `../lithium/src/base58.h` |
| RPC ports | `../lithium/src/bitcoinrpc.cpp` |

## SegWit Activation Test

- Functional test: `test/functional/segwit-activation-smoke.py`
- Build-server wrapper: `/home/sid/Blakestream-Installer/qa/runtime/run-segwit-activation-suite.sh`
- Direct command used by the wrapper:

```bash
BITCOIND=/path/to/lithiumd BITCOINCLI=/path/to/lithium-cli \
python3 ./test/functional/segwit-activation-smoke.py \
  --srcdir="$(pwd)/src" \
  --tmpdir="<artifact_root>/lithium/<timestamp>/tmpdir" \
  --nocleanup \
  --loglevel=DEBUG \
  --tracerpc
```

- Expected regtest Bech32 prefix: `rlit1`
- Review artifacts:
  `summary.json`, `state-defined.json`, `state-started.json`, `state-locked_in.json`, `state-active.json`, `address-sanity.json`, `combined-regtest.log`, `tmpdir/test_framework.log`, `tmpdir/node*/regtest/debug.log`
- Successful all-six build-server run:
  `/home/sid/Blakestream-Installer/outputs/segwit-activation/20260412T083423Z/run-summary.md`
- Coin artifact directory:
  `/home/sid/Blakestream-Installer/outputs/segwit-activation/20260412T083423Z/lithium`
- Harness note:
  the final witness proposal builder now takes the coinbase amount directly from `getblocktemplate()["coinbasevalue"]`, which keeps the activation proof aligned with each chain's real subsidy rules.
- Safety rule:
  regtest only for activation validation; do not mine or send transactions on mainnet while rollout QA is still in progress.

## AuxPoW Testnet Merged-Mining Verification

- Final successful container-built run:
  `/home/sid/Blakestream-Installer/outputs/auxpow-testnet/20260413T003341Z/run-summary.md`
- Wrapper command:
  `bash /home/sid/Blakestream-Installer/qa/auxpow-testnet/run-auxpow-testnet-suite.sh`
- Parent chain:
  Blakecoin testnet only, fully isolated from public peers.
- Live proof result:
  lithium accepted `2` merged-mined child blocks in the 4-child batch and `1` in the 5-child full run.
- Direct RPC cross-check:
  `createauxblock` plus `submitauxblock` accepted on a fresh lithium testnet pair. Artifact:
  `/home/sid/Blakestream-Installer/outputs/auxpow-testnet/20260413T003341Z/lithium/rpc-crosscheck.json`
- QC note:
  lithium's chain ID is one of the values that made `merkle-size = 4` impossible for the 4-child phase, so the harness now automatically resolves to the smallest collision-free power-of-two size.
- Safety rule:
  testnet only for merged-mining QA; do not mine or send transactions on mainnet while AuxPoW rollout validation is still in progress.

## Devnet/Testnet Validation Outcomes

- SegWit activation validation passed on isolated regtest. See:
  `/home/sid/Blakestream-Installer/outputs/segwit-activation/20260412T083423Z/lithium`
- AuxPoW merged-mining validation passed on isolated testnet, including direct `createauxblock` plus `submitauxblock` acceptance. See:
  `/home/sid/Blakestream-Installer/outputs/auxpow-testnet/20260413T003341Z/lithium`
- Mainnet carry-back audit for the devnet copy lives in:
  `mainnet-carryback-audit-2026-04-18.md`
- Audit result:
  the diff between this repo and the devnet `coins/lithium` copy stayed limited to devnet `chainparams*` and build cleanup. No new lithium mainnet wallet, consensus, or RPC carry-back was identified from the devnet copy itself.

## Mainnet Carry-Back Decisions

- SegWit rollout remains scheduled, not forced active.
- Mainnet AuxPoW start height remains `160000` as the chain source of truth.
- Do not port devnet network identity, datadir, test shortcuts, or activation shortcuts back into this repo.
- Pool/runtime carry-back work is tracked in the mainnet Eloipool repo.
- Electrium sync and signing carry-back work is tracked in the Electrium repo.
- Mainnet pool integration now depends on the proven multi-miner aux-child payout path in Eloipool, not the old single active mining-key QA shortcut.

## Staging Hygiene

- Keep the intentional autotools and build-system layer in staging for this repo:
  `Makefile.am`, `Makefile.in`, `aclocal.m4`, `autogen.sh`, `configure*`, `build-aux/*`, and `depends/*`.
- Trim generated build junk before review or promotion:
  `.libs/`, `.deps/`, `autom4te.cache/`, `*.o`, `*.lo`, `*.la`, `config.log`, `config.status`, and similar transient outputs.
- April 19, 2026 staging pass explicitly removed staged libtool and univalue build artifacts while preserving the intentional autotools carry-back set.

## Not Carried Back From Devnet

- `src/chainparams.cpp`, `src/chainparamsbase.cpp`, `src/chainparamsbase.h`
- Any private-testnet `BIP65Height = 1`, `ALWAYS_ACTIVE`, devnet ports, message starts, datadirs, or local-only harness shortcuts
- Pool UI, merged-mine proxy, Electrium, ElectrumX, and builder/runtime scripts

## Pool / Electrium Dependencies

- Mainnet merged-mining now depends on the modern `createauxblock` plus `submitauxblock` direction and the proven multi-miner aux payout model in Eloipool.
- Electrium compatibility now depends on full AuxPoW header support and Blake-family single-SHA signing compatibility.
- Per-coin overlays and branding stay in the Electrium repo and are not folded back into this C++ core tree.

## Safety Rule

- Do not mine on mainnet while carry-back staging is in progress.
- Do not send transactions on mainnet while carry-back staging is in progress.
- Use isolated regtest, testnet, or staging environments until rollout QA is complete.

## April 18, 2026 Devnet Validation Snapshot

- Shared BlakeStream devnet run `20260418T195508Z` proved concurrent multi-miner AuxPoW against the live pool with two mining keys active in the same session.
- Live pooled merged-mined lithium child block proof is green:
  height `416` accepted with `tx_count = 2`.
- This means the live pool/proxy path is no longer limited to coinbase-only lithium child blocks once mempool transactions are present.

## Mainnet Carry-Back Snapshot

- Keep lithium chain identity, Hashblake requirement, AuxPoW start rules, and scheduled SegWit rollout exactly as already documented in this repo.
- Promote only the proven external dependencies:
  mainnet pool multi-miner mining-key payout plumbing and Electrium full AuxPoW-header plus single-SHA signing compatibility.
- Do not carry back any devnet ports, datadirs, private-testnet activation shortcuts, or runtime wrapper behavior into mainnet chain params.

## April 19, 2026 Broader Electrium Staging Closure

- Broader staged packaged-client proof is now green at:
  `/home/sid/Blakestream-Devnet/outputs/electrium-staging/20260419T053030Z/run-summary.md`
- Lithium's packaged Electrium client connected successfully against the staged
  local ElectrumX backend on `127.0.0.1:54001`.
- This run also exposed and closed a shared aux-core startup bug in
  `src/validation.cpp`: height-less disk rereads could treat genesis-like
  headers as regular AuxPoW blocks and fail with a false
  `non-AUX proof of work failed` reject.
- The fix now treats
  `block.GetHash() == consensusParams.hashGenesisBlock || block.hashPrevBlock.IsNull()`
  as genesis-like in the disk-reread path, which keeps standalone staged
  backends honest without relaxing real chained AuxPoW validation.
