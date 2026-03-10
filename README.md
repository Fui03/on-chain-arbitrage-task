# DEX Arbitrage Mini Challenge

This repository contains a full submission for the challenge in two parts:

- `part1/`: an off-chain C++20 solver that loads a Uniswap V2 snapshot, builds a token graph, searches simple arbitrage cycles up to a configurable maximum length, optimizes trade size, and writes the Top-10 results.
- `part2/`: an optional Hardhat/Solidity validator that can either replay a Part 1 cycle against live pair reserves on a mainnet fork or deploy a mock profitable cycle to Sepolia for a real public-testnet demonstration.

## Start Here

If you are opening this project for the first time, run the steps below in order.

### 0. Check the required input file

Before running anything, make sure the pool snapshot exists at:

```text
./v2pools.json
```

If that file is missing, Part 1 cannot run because it is the dataset the solver reads.

### 1. Verify Part 1 first

Run this before anything else:

```bash
cd part1
make test
```

What this does:

- compiles the Part 1 code
- runs the unit tests for AMM math, graph logic, pricing, and opportunity de-duplication
- confirms the C++ environment is working before you try the real dataset

If this passes, build the solver binary:

```bash
make
```

### 2. Run the Part 1 solver

Still inside `part1/`, run:

```bash
./bin/arb_detect --input ../v2pools.json --output results/top10_cycles.json --top 10 --min-reserve-usd 1000 --max-input-share 0.05 --max-cycle-length 4
```

What is happening here:

- the solver loads pools from `v2pools.json`
- it filters out weak-liquidity pools using `reserveUSD >= 1000`
- it builds a token graph from the remaining pools
- it enumerates unique simple cycles up to the configured maximum length
- it simulates both directions and all start-token rotations for each cycle
- it uses Uniswap V2 math with a `0.3%` fee assumption
- it searches for a profitable input size for each candidate cycle
- it ranks the surviving opportunities and writes the Top 10 to `part1/results/top10_cycles.json`

### 3. Read the Part 1 output

After the solver runs successfully, inspect:

```text
part1/results/top10_cycles.json
```

This file is the bridge into Part 2. It contains:

- the ranked arbitrage cycles
- suggested input and output amounts
- `minOutRaw` for on-chain validation
- summary metrics such as number of enumerated cycles and number of profitable candidates

### 4. Verify Part 2

Only do this after Part 1 has already produced `part1/results/top10_cycles.json`.

```bash
cd ../part2
npm install
npm test
```

What this does:

- installs the Hardhat toolchain
- compiles the Solidity contracts
- runs the validator tests against mock Uniswap-style pairs
- confirms the validator behaves correctly before any live replay

Important note:

- `npm test` is intentionally local-only and does not need `MAINNET_RPC_URL`
- mainnet forking is opt-in and is only enabled when both `ENABLE_MAINNET_FORK=1` and a real `MAINNET_RPC_URL` are set
- placeholder values such as `https://your-mainnet-rpc` are ignored so they do not break the demo flow

### 5. Optionally run live on-chain validation

If you want to replay a ranked opportunity against live reserves, run Part 2 on a mainnet fork:

```bash
export ENABLE_MAINNET_FORK=1
export MAINNET_RPC_URL="https://your-mainnet-rpc"
npx hardhat run scripts/validate-cycle.js
```

Optional environment variables:

- `CYCLES_PATH`: override the default cycle JSON path
- `CYCLE_INDEX`: choose which ranked opportunity to validate
- `MAX_CYCLES`: validate multiple consecutive opportunities

What is happening here:

- the script loads the ranked cycle data from Part 1
- it deploys `CycleValidator.sol`
- it reads current reserves from the live pool contracts on the fork
- it recomputes the cycle output using raw on-chain math
- it reverts if the cycle is no longer profitable or if output is below `minOutRaw`

### 6. Optionally run a real Ethereum testnet demo

If you want a real public-testnet deployment, use Sepolia:

```bash
cd ../part2
export SEPOLIA_RPC_URL="https://your-sepolia-rpc"
export TESTNET_PRIVATE_KEY="0xyour-funded-private-key"
npm run testnet:demo
```

What is happening here:

- the script deploys `CycleValidator.sol` to Sepolia
- it deploys three `MockV2Pair` contracts that form a profitable cycle
- it previews the cycle output with `quoteCycle(...)` and `validateCycle(...)`
- it then sends a real transaction to `recordValidation(...)`
- that transaction emits `CycleValidationRecorded`, which gives you an explorer-visible proof that the validator logic executed on-chain
- it writes a machine-readable deployment report to `part2/deployments/sepolia-demo.json`

Important note:

- this Sepolia path is a real Ethereum testnet deployment, but it does **not** use the Part 1 snapshot pool addresses
- that is intentional, because the snapshot addresses are mainnet addresses and do not exist on Sepolia
- use the mainnet-fork flow when you want to recheck real Part 1 opportunities
- the Sepolia demo therefore proves the validator logic works on a real Ethereum testnet, but it does not prove that a mainnet snapshot opportunity is live on Sepolia

## Recommended Run Order

Use this order every time:

1. `cd part1 && make test`
2. `cd part1 && make`
3. `cd part1 && ./bin/arb_detect ... --max-cycle-length 4`
4. inspect `part1/results/top10_cycles.json`
5. `cd part2 && npm install && npm test`
6. optionally run `npx hardhat run scripts/validate-cycle.js` with `ENABLE_MAINNET_FORK=1` and `MAINNET_RPC_URL`
7. optionally run `npm run testnet:demo` with `SEPOLIA_RPC_URL` and `TESTNET_PRIVATE_KEY`

## How The Whole Project Works

The project is split into two stages:

### Part 1: Find opportunities from a static snapshot

Part 1 is an offline analysis step. It does not read live chain state. In this implementation, it looks at the supplied JSON snapshot and asks:

> If these reserves are accurate, which simple cycles up to the configured maximum length would return more of the starting token than they consume?

The original brief asks for arbitrage cycle detection in general. This repository now supports simple-cycle search up to a configurable maximum length via `--max-cycle-length`, with `4` used as the practical default in the checked-in example run. That is why Part 1 is still fast and deterministic, but also why its results can become stale.

### Part 2: Recheck the same opportunity against live state

Part 2 has two modes:

- a mainnet-fork replay mode for real Part 1 opportunities
- a Sepolia demo mode for proving the validator works on a public Ethereum testnet

In mainnet-fork replay mode, Part 2 does not search the whole graph again. Instead, it takes a cycle already found by Part 1 and asks:

> Is this exact cycle still profitable right now, using the current reserves on-chain?

In Sepolia demo mode, Part 2 asks:

> Can the validator and cycle logic be deployed and exercised successfully on a real Ethereum testnet?

That is why Part 2 is optional but important. It protects against stale snapshot opportunities on a fork and also gives you a concrete public-testnet demo path.

## Repository Layout

- `README.md`: first-run guide and project overview
- `REPORT.md`: submission report
- `docs/RUNBOOK.md`: step-by-step operational guide
- `docs/ARCHITECTURE.md`: system and module explanation
- `docs/OUTPUT_REFERENCE.md`: explanation of the result JSON fields
- `docs/COMPLIANCE_CHECKLIST.md`: requirement-to-evidence checklist
- `part1/`: C++ solver
- `part2/`: Hardhat validator

### Part 1 layout

- `part1/src/app/`: CLI parsing and top-level orchestration
- `part1/src/domain/model/`: shared domain data structures and metadata
- `part1/src/domain/graph/`: graph construction and simple-cycle enumeration
- `part1/src/domain/pricing/`: AMM math and anchored USD pricing
- `part1/src/domain/opportunity/`: trade-size optimization, evaluation, and ranking
- `part1/src/io/`: dataset loading and JSON output
- `part1/src/presentation/`: console reporting
- `part1/src/util/`: helper utilities
- `part1/tests/`: unit tests

### Part 2 layout

- `part2/contracts/interfaces/`: pair interfaces
- `part2/contracts/libraries/`: reusable Solidity math
- `part2/contracts/mocks/`: mock pair contracts for tests and the Sepolia demo
- `part2/contracts/validators/`: production validator logic
- `part2/scripts/lib/`: reusable script helpers
- `part2/test/helpers/`: shared test fixtures
- `part2/test/validators/`: contract test suite

## Assumptions

- The dataset has no explicit fee field, so the solver assumes the standard Uniswap V2 fee of `0.3%` implemented as `997 / 1000`.
- Pools with `reserveUSD < 1000` are filtered by default to remove dust liquidity.
- The search scope is configurable through `--max-cycle-length`, with `4` used as the default example setting in this repository.
- Token symbols are not included in the dataset, so unknown assets are shown as shortened addresses.

## Common Failure Cases

### `unable to open pool snapshot`

Cause:

- `v2pools.json` is missing or the `--input` path is wrong

Fix:

- place `v2pools.json` at the repository root, or pass the correct path to `--input`

### `Live pool bytecode was not found`

Cause:

- Part 2 is being run without a working mainnet fork

Fix:

- set `ENABLE_MAINNET_FORK=1`
- set `MAINNET_RPC_URL` to a working Ethereum mainnet RPC
- rerun the Hardhat script

### Missing `sepolia` network or deployer account

Cause:

- `SEPOLIA_RPC_URL` is not set, or no funded `TESTNET_PRIVATE_KEY` is configured

Fix:

- export both variables before running `npm run testnet:demo`

### `Validation reverted`

Cause:

- the opportunity from the snapshot is no longer profitable on live state

Fix:

- this is expected behavior for stale opportunities; choose another cycle or rerun Part 1 on fresher data

## Documentation Map

Use these documents depending on what you need:

- read `README.md` first if you want to know what to run
- read `docs/RUNBOOK.md` if you want a checklist-style execution guide
- read `docs/ARCHITECTURE.md` if you want to understand the design
- read `docs/OUTPUT_REFERENCE.md` if you want help interpreting `top10_cycles.json`
- read `docs/COMPLIANCE_CHECKLIST.md` if you want to audit the submission against the implemented requirements
- read `REPORT.md` if you want the short write-up for submission

## Latest Verified Sepolia Demo

The latest confirmed public-testnet validation run completed on March 10, 2026 and produced:

- deployer: `0x9e61B993e4cDcEF54842B6B71D419827Bc567cb0`
- validator: `0x3A9a912C1ed0A34C3314504EaD8466615e5E967D`
- validation transaction: `0x6f43a15cf2d03ee6cc070c1f0eee6a35d00110ca17c81c1f96a7d91e43cf7745`
- validation block: `10415564`
- gas used: `58164`

Output consistency from the recorded run:

- `amountInRaw = 10000000000000000000`
- `expectedOutRaw = 12000253838105361951`
- `previewValidatedOutRaw = 12000253838105361951`
- `validatedOutRaw = 12000253838105361951`

Explorer links:

- [Validator Contract](https://sepolia.etherscan.io/address/0x3A9a912C1ed0A34C3314504EaD8466615e5E967D)
- [Validation Transaction](https://sepolia.etherscan.io/tx/0x6f43a15cf2d03ee6cc070c1f0eee6a35d00110ca17c81c1f96a7d91e43cf7745)

The corresponding machine-readable proof artifact is:

- `part2/deployments/sepolia-demo.json`
