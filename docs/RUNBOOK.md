# Runbook

This document is the operational checklist for running the project from a clean checkout.

## Prerequisites

Make sure you have:

- `g++` with C++20 support
- `make`
- `node` and `npm`
- the pool snapshot file at `./v2pools.json`
- a mainnet RPC URL if you want live Part 2 validation

## First-Time Setup

From the repository root:

```bash
cd part1
make test
make
```

Why this is first:

- `make test` verifies the C++ toolchain and regression tests
- `make` builds the production detector binary

## Generate Opportunities

From `part1/`:

```bash
./bin/arb_detect --input ../v2pools.json --output results/top10_cycles.json --top 10 --min-reserve-usd 1000 --max-input-share 0.05 --max-cycle-length 4
```

Expected outcome:

- console summary of loaded pools, tokens, enumerated cycles, and returned opportunities
- output file written to `part1/results/top10_cycles.json`

## Inspect Output

Check:

```text
part1/results/top10_cycles.json
```

You should expect:

- top-level run statistics
- an `opportunities` array
- token path, pools, amounts, and ranking metadata for each cycle

## Validate Part 2 Locally

From the repository root:

```bash
cd part2
npm install
npm test
```

Expected outcome:

- Hardhat compiles successfully
- the validator tests pass against mock pool contracts
- no mainnet RPC is required for this step
- placeholder environment variables are ignored, so local test runs stay local

## Validate A Live Cycle On A Fork

From `part2/`:

```bash
export ENABLE_MAINNET_FORK=1
export MAINNET_RPC_URL="https://your-mainnet-rpc"
npx hardhat run scripts/validate-cycle.js
```

Optional selectors:

```bash
export CYCLE_INDEX=0
export MAX_CYCLES=1
export CYCLES_PATH="../part1/results/top10_cycles.json"
```

Expected outcome:

- the script deploys `CycleValidator`
- it loads one or more cycles from the Part 1 JSON
- it recomputes live outputs from the current pair reserves
- it prints either a validated output or a revert reason

## Run A Real Ethereum Testnet Demo

If you want to demonstrate Part 2 on a public Ethereum testnet, use Sepolia:

```bash
cd part2
export SEPOLIA_RPC_URL="https://your-sepolia-rpc"
export TESTNET_PRIVATE_KEY="0xyour-funded-private-key"
npm run testnet:demo
```

Expected outcome:

- `CycleValidator` is deployed to Sepolia
- three `MockV2Pair` contracts are deployed to Sepolia
- the profitable cycle is previewed via `eth_call`
- a real Sepolia transaction calls `recordValidation(...)`
- the transaction emits `CycleValidationRecorded`
- a deployment report is written to `part2/deployments/sepolia-demo.json`

Important distinction:

- this testnet demo does not reuse Part 1 snapshot pool addresses
- the Part 1 dataset contains mainnet pool addresses, so those addresses must still be revalidated on a mainnet fork
- the strongest proof artifact is the validation transaction hash written into the deployment report

## Recommended Routine

For normal use, follow this order:

1. `cd part1 && make test`
2. `cd part1 && make`
3. `cd part1 && ./bin/arb_detect ... --max-cycle-length 4`
4. inspect `part1/results/top10_cycles.json`
5. `cd part2 && npm test`
6. optionally run the fork validation script
7. optionally run the Sepolia demo script

## Troubleshooting

### Missing `v2pools.json`

Symptom:

- Part 1 exits with `unable to open pool snapshot`

Action:

- place the snapshot at the repository root, or pass the correct path with `--input`

### No live contract code found

Symptom:

- Part 2 reports that live pool bytecode was not found

Action:

- set `ENABLE_MAINNET_FORK=1`
- confirm `MAINNET_RPC_URL` points to a working Ethereum mainnet RPC

### Missing Sepolia RPC or deployer key

Symptom:

- Hardhat cannot find the `sepolia` network, or deployment fails before broadcasting

Action:

- export `SEPOLIA_RPC_URL`
- export `TESTNET_PRIVATE_KEY`
- make sure the key belongs to a funded Sepolia account

### Live validation fails even though Part 1 found profit

Symptom:

- `validateCycle` reverts with `NotProfitable` or `MinOutNotMet`

Action:

- this means live reserves moved since the snapshot; rerun Part 1 on fresher data or try another cycle
