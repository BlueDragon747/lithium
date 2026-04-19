# Lithium 0.15.2 Update — QC Report

Fresh QC performed against source of truth `lithium-0.15.2-update.md` and the
legacy 0.8.x Lithium source in `../lithium/src/`.

---

## Coin Metrics Summary

| Parameter               | Expected                       | 0.15.2 Port Actual            | Legacy 0.8.x Actual         | Status |
|-------------------------|--------------------------------|-------------------------------|-----------------------------|--------|
| Coin Name / Ticker      | Lithium / LIT                  | Lithium / LIT (`bech32 lit`)  | Lithium / LIT               | PASS   |
| Algorithm               | BLAKE-256 8-round              | `Hashblake` hash + PoW        | BLAKE-256 8-round           | PASS   |
| Base Version            | 0.15.2                         | 0.15.2                        | 0.8.9.7                     | PASS   |
| Block Time              | 180 s                          | `nPowTargetSpacing = 180`     | 180 s                       | PASS   |
| Retarget Timespan       | 3600 s                         | `20 * 3 * 60 = 3600`          | 3600 s                      | PASS   |
| Retarget Interval       | 20 blocks                      | 20 blocks                     | 20 blocks                   | PASS   |
| Coinbase Maturity       | 120                            | `COINBASE_MATURITY = 120`     | 120                         | PASS   |
| Max Supply              | 25,228,800 LIT                 | `MAX_MONEY = 25228800 * COIN` | 25228800 * COIN             | PASS   |
| Genesis nTime           | 1411788333                     | 1411788333                    | 1411788333                  | PASS   |
| Genesis nNonce          | 8298496                        | 8298496                       | 8298496                     | PASS   |
| Genesis nVersion        | 112                            | 112                           | 112                         | PASS   |
| Genesis Hash            | `000000fc...931902db`          | matches                       | matches                     | PASS   |
| Merkle Root             | `2a1fd3e4...f4d6c008`          | matches                       | matches                     | PASS   |
| pchMessageStart (main)  | `f4 a3 29 d5`                  | `f4 a3 29 d5`                 | `f4 a3 29 d5`               | PASS   |
| pchMessageStart (test)  | `0d 15 04 0c`                  | `0d 15 04 0c`                 | `0d 15 04 0c`               | PASS   |
| P2P Port (main)         | 12007                          | `nDefaultPort = 12007`        | 12007                       | PASS   |
| P2P Port (test)         | 12000                          | `nDefaultPort = 12000`        | 12000                       | PASS   |
| RPC Port (main)         | 12000                          | `nRPCPort = 12000`            | 12000                       | PASS   |
| RPC Port (test)         | 12004                          | `nRPCPort = 12004`            | 12004                       | PASS   |
| Pubkey prefix (main)    | 19 (0x13)                      | 19                            | 19                          | PASS   |
| Script prefix (main)    | 7 (0x07)                       | 7                             | 7                           | PASS   |
| SecretKey prefix (main) | 128 (0x80)                     | 128                           | 128                         | PASS   |
| Pubkey prefix (test)    | 142                            | 142                           | 142                         | PASS   |
| Script prefix (test)    | 170                            | 170                           | 170                         | PASS   |
| SecretKey prefix (test) | 239                            | 239                           | 239                         | PASS   |
| Bech32 HRP              | `lit` / `tlit` / `rlit`        | `lit` / `tlit` / `rlit`       | n/a                         | PASS   |
| AuxPow Chain ID         | `0x0006`                       | `0x0006`                      | `0x0006`                    | PASS   |
| AuxPow Start (main)     | 160000 + pre-start tolerance   | 160000 + tolerance            | 160000                      | PASS   |
| AuxPow Start (test/reg) | 0 (strict chain ID off)        | 0 (`fStrictChainId = false`)  | n/a                         | PASS   |
| Retarget 150000 lookback| full interval                  | `height >= 150000` full-interval | matches                  | PASS   |
| Difficulty clamping     | ±15% <3500, ±3% after          | matches legacy branch logic   | matches                     | PASS   |
| BIP30 relaxation        | `!pindex->phashBlock` only     | matches                       | matches                     | PASS   |
| Wire checksum           | `Hashblake` uniform            | `Hashblake` uniform           | `Hashblake`                 | PASS   |
| Ticker                  | LIT                            | LIT                           | LIT                         | PASS   |

---

## Source-of-Truth Claim Verification

Against the QC Status bullets and other prescriptive sections of
`lithium-0.15.2-update.md`:

| Claim                                                                                                 | Status |
|-------------------------------------------------------------------------------------------------------|--------|
| 0.15.2 tree builds; regtest `wallet-basic-smoke.py` passes with no sends/funding/mining               | PASS (per verified snapshot in source-of-truth; not re-run here) |
| Lithium has AuxPow in legacy source with start 160000 and chain ID 0x0006                             | PASS — confirmed in `../lithium/src/main.cpp:2065-2076` |
| Secret key prefixes are 128 / 239                                                                     | PASS — confirmed in both trees |
| Reward 0.48 -> 48 LIT flip happens AT height 1999 (boundary must come from code, not prose)           | PASS — `validation.cpp:1082` uses `nHeight < 1999 -> 48 CENT`, next tier `nHeight < 175000 -> 48 COIN`, so height 1999 returns 48 COIN. Exactly matches legacy `nHeight > 1998 && nHeight < 175000` branch. |
| Checkpoint set must match legacy (not genesis-only, not Blakecoin's)                                  | PASS — all 10 Lithium checkpoints ported verbatim (heights 0, 13000, 19021, 26012, 30019, 84000, 118009, 139452, 535001, 1250000) |
| `height >= 150000` full-interval retarget lookback restored in `pow.cpp`                              | PASS — see `pow.cpp:53-57` matching legacy `main.cpp:1204-1206` |
| Shared AuxPow framework integrated (`auxpow.{h,cpp}`, `primitives/pureheader.{h,cpp}`, block/header serialization, disk index, PoW validation, chain-ID-aware templates) | PASS — all files present and `block.h` serializes `CPureBlockHeader` + conditional AuxPow payload |
| Keep nominal mainnet AuxPow start 160000 but tolerate pre-start AuxPow-bearing blocks (removes `early-auxpow-block` reject) | PASS — `pow.cpp:147-168` `CheckAuxPowProofOfWork`: when `!auxpowActive` AND `block.auxpow` is set, it simply returns `true` instead of rejecting, matching the 0.8.x legacy tolerance |
| Testnet/regtest AuxPoW at 0 with strict chain ID disabled                                             | PASS — `fStrictChainId = false`, `nAuxpowStartHeight = 0` on both |
| BIP30 relaxation preserved (`!pindex->phashBlock` only)                                               | PASS — `validation.cpp:1769` `bool fEnforceBIP30 = !pindex->phashBlock;` with legacy commentary |
| Modern merged-mining RPC surface: `createauxblock <address>` + `submitauxblock <hash> <auxpow>`, `getauxblock` only as compatibility wrapper, no `getworkaux` | PASS — `rpc/mining.cpp` defines all three in the command table; `getauxblock` routes to the same `AuxMiningCreateBlock` / `AuxMiningSubmitBlock` helpers; no `getworkaux` present |
| Wire Checksum Policy: Lithium preserves legacy `Hashblake` on all P2P, no handshake exception         | PASS — `net.cpp:832` and `net.cpp:2931` both use `Hashblake` uniformly; no version/verack bypass path found |
| BlakeStream DNS seed policy: 2 shared seeds serve all 6 coins                                         | PASS — `vSeeds` uses the 2 shared ecosystem seeds `seed.blakestream.io` and `seed.blakecoin.org`. Per policy, all 6 coins use the SAME 2 seeds and they serve nodes for ALL coins; coin separation happens at the wire-protocol layer via `pchMessageStart` and port. Matches the Blakecoin 0.15.2 reference repo exactly. |
| SegWit mainnet signaling starts 2026-05-11 UTC (`1778457600`), timeout 2027-05-11 UTC (`1809993600`)  | PASS — exact values in `chainparams.cpp:124-125` |
| CSV ALWAYS_ACTIVE; testnet/regtest SegWit ALWAYS_ACTIVE                                               | PASS |
| BIP34/65/66 disabled (version checks off)                                                             | PASS — all set to height 100000000 on mainnet/testnet (regtest keeps BIP65/66 at 1351/1251 for rpc activation tests, consistent with Blakecoin port) |

---

## AuxPoW Framework Integration

| Component                                          | Status |
|----------------------------------------------------|--------|
| `src/auxpow.{h,cpp}`                               | PRESENT |
| `src/primitives/pureheader.{h,cpp}`                | PRESENT |
| `CBlockHeader : public CPureBlockHeader`           | PASS   |
| Conditional AuxPow serialization (read/write)      | PASS — `block.h:40-52` reads/writes `auxpow` when `IsAuxpow()` |
| Chain-ID-aware version modifiers (`VERSION_AUXPOW`, `VERSION_CHAIN_START`, `SetBaseVersion(chainId)`, `GetChainId`) | PASS — `pureheader.h:22-108` |
| `CheckAuxPowProofOfWork` honours `nAuxpowStartHeight`, `fStrictChainId`, `nAuxpowChainId` | PASS — `pow.cpp:147-177` |
| Pre-start tolerance (accept AuxPow-bearing blocks before nominal start) | PASS — `!auxpowActive` path returns `true` without rejecting, satisfying the 0.8.x historical-sync rule |
| Chain ID on mainnet / testnet / regtest            | `0x0006` / `0x0006` / `0x0006` — PASS |
| `nAuxpowStartHeight`                               | mainnet 160000, testnet 0, regtest 0 — PASS |
| `fStrictChainId`                                   | mainnet `true`, testnet/regtest `false` — PASS |

---

## RPC Surface

| RPC                     | Present | Wired | Notes |
|-------------------------|---------|-------|-------|
| `createauxblock <addr>` | YES     | YES   | Address-driven; decodes to `GetScriptForDestination` and calls shared `AuxMiningCreateBlock` helper. Matches the BlakeStream Seed And AuxPoW RPC Policy. |
| `submitauxblock <hash> <auxpow>` | YES | YES | Decodes hex AuxPow via `DecodeAuxPow`, submits via `AuxMiningSubmitBlock` -> `ProcessNewBlock`. `BIP22ValidationResult(sc.state)` is `NullUniValue` on accept, so `response.isNull()` returns `true` for accepted blocks (semantically correct). |
| `getauxblock (hash auxpow)` | YES | YES | Compatibility wrapper: no-arg path uses wallet `CReserveScript`, with-args path routes through the same `AuxMiningSubmitBlock`. No duplicate legacy implementation path — confirms source-of-truth guidance. |
| `getworkaux`            | ABSENT  | n/a   | Correctly out of scope. |
| AuxPoW template `chainid` | returns `6` | — | Matches `consensus.nAuxpowChainId = 0x0006` per source-of-truth verified snapshot. |

---

## Previously Fixed Critical Issues (Re-confirmation)

All three previously reported critical issues remain corrected in the current tree:

1. **`pchMessageStart` (mainnet)** — `chainparams.cpp:139-142` sets `f4 a3 29 d5`, matching legacy `main.cpp:3193`. Testnet bytes `0d 15 04 0c` match `main.cpp:2863-2866`. FIXED / STILL CORRECT.
2. **Checkpoints** — All 10 Lithium checkpoints (0 / 13000 / 19021 / 26012 / 30019 / 84000 / 118009 / 139452 / 535001 / 1250000) with the correct hashes are present in `chainparams.cpp:171-184`. No residual Blakecoin checkpoint hashes. `chainTxData` also ported (timestamp 1646262870, 1628063 tx). FIXED / STILL CORRECT.
3. **Seeds** — `vSeeds` carries the 2 shared BlakeStream ecosystem seeds (`seed.blakestream.io`, `seed.blakecoin.org`), not leftover Blakecoin IPs or Lithium legacy static IPs. `pnSeed6_main` fixed seeds wired into `vFixedSeeds`. Per policy, all 6 coins use the SAME 2 seeds and they serve nodes for ALL coins; coin separation happens at the wire-protocol layer. Matches the Blakecoin 0.15.2 reference repo exactly. FIXED / STILL CORRECT.

Additional previously-identified corrections re-verified:

- `GetBlockSubsidy` height boundaries match legacy byte-for-byte including the fractional `48 CENT` (0.48 LIT) and `150 CENT` (1.5 LIT) tiers (`validation.cpp:1075-1097`).
- `MAX_MONEY = 25228800 * COIN` (`amount.h:26`).
- `COINBASE_MATURITY = 120` (`consensus/consensus.h:19`).
- Custom difficulty clamping ±15% / ±3% with height-3500 threshold plus `height >= 150000` full-interval lookback both present in `pow.cpp:53-101`.
- Build metadata: `build.sh` has `COIN_NAME=lithium`, `DAEMON_NAME=lithiumd`, `QT_NAME=lithium-qt`, `CLI_NAME=lithium-cli`, `TX_NAME=lithium-tx`, `VERSION=0.15.2`, `RPC_PORT=12000`, `P2P_PORT=12007`. `configure.ac` has `AC_INIT([Lithium Core], ...)` and all four `BITCOIN_*_NAME` variables set correctly.

---

## Action Items

Nothing consensus-blocking was found in this QC pass. All previously corrected
items remain in place and the source-of-truth claims are faithfully reflected
in the code. Residual follow-on work (carried from the source-of-truth, not
new findings):

1. **Historical merged-mined header replay** — Full mainnet IBD replay still
   needs to be executed end-to-end to confirm the pre-start AuxPow tolerance
   fully clears the legacy chain past the previously-failing
   `early-auxpow-block` reject around height 913. Source-of-truth marks this
   as still outstanding.
2. **Isolated `submitauxblock` acceptance pass** — A solved AuxPoW payload
   should be round-tripped through `createauxblock` / `submitauxblock` on
   testnet or regtest to confirm acceptance end-to-end; current snapshot only
   verifies template creation and chain-ID reporting.
3. **Full mainnet BIP30 replay** — The relaxed BIP30 rule is intentionally
   preserved, but a historical replay should confirm no modern 0.15.2 path
   silently re-enforces the stricter Bitcoin Core duplicate-txid check.
4. **DNS seed operational check** — Confirm that `seed.blakestream.io`
   returns Lithium-network peers only (policy is enforced operationally, not
   in-source). This cannot be validated from the repo alone.
5. **Mainnet no-send / no-mine rule** — Keep in place until items 1-3 are
   cleared, per source-of-truth.
6. **Minor cosmetic** — Several comments in `chainparams.cpp`, `pow.cpp`, and
   `validation.cpp` still carry `BEGIN BLAKECOIN` / `Blakecoin` prose
   attribution even though the logic is Lithium-specific. No functional
   impact; can be cleaned up opportunistically.

**Overall QC verdict:** Lithium 0.15.2 update repo is internally consistent
with its source-of-truth and with the legacy 0.8.x Lithium source for all
consensus-critical, network, AuxPoW, and RPC surface items reviewed in this
pass. No PASS above is contingent on code changes; the remaining risk items
are runtime QA tasks (historical replay, solved-block round-trip, DNS
operator check), not source fixes.
