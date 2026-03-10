# Architecture

This document explains how the project is structured and what each stage is doing.

## End-To-End Flow

The repository has a two-stage pipeline:

1. Part 1 searches the static snapshot for candidate arbitrage cycles.
2. Part 2 either replays one of those cycles against live pair reserves on a fork or deploys a mock profitable cycle to Sepolia.

The two stages are intentionally separated because they solve different problems:

- Part 1 is a search problem over many pools.
- Part 2 is a validation problem for one already-selected cycle.

## Part 1: Off-Chain Search

Part 1 works as follows:

1. Load `v2pools.json`
2. Filter out invalid or dust pools
3. Build an undirected token graph
4. Enumerate unique simple cycles up to the configured maximum length
5. Expand each cycle into directed cycle candidates
6. Reject obviously bad candidates with a marginal-product screen
7. Optimize trade size for the remaining candidates
8. Rank the surviving opportunities
9. Write `top10_cycles.json`

### Why bounded simple cycles

The project intentionally searches simple cycles up to a configurable maximum length rather than unbounded cycle lengths because:

- the supplied dataset contains no duplicate unordered token pairs, so same-pair two-pool arbitrage is not available
- the filtered graph remains sparse enough that short simple cycles already expose many candidate opportunities
- bounded cycle enumeration is much more practical for a mini challenge than unbounded exhaustive search

This is an implementation choice, not a literal restriction from the brief. The brief asks for arbitrage cycle detection in general and gives `A -> B -> C -> A` as an example.

### Part 1 module layout

`part1/src/app/`

- CLI parsing
- top-level orchestration

`part1/src/domain/model/`

- shared structs like `PoolRecord`, `Graph`, `CyclePattern`, and `RankedOpportunity`

`part1/src/domain/graph/`

- graph construction
- pair lookup generation
- simple-cycle enumeration

`part1/src/domain/pricing/`

- Uniswap V2 swap math
- anchored USD price propagation for ranking

`part1/src/domain/opportunity/`

- trade-size optimization
- cycle evaluation
- opportunity de-duplication and ranking

`part1/src/io/`

- JSON snapshot loading
- JSON result writing

`part1/src/presentation/`

- console summaries and Top-N printing

`part1/src/util/`

- numeric formatting
- address labeling

## Part 2: On-Chain Validation

Part 2 supports two distinct operating modes.

### Mode A: mainnet-fork replay of real Part 1 output

In this mode, Part 2 does not discover new cycles. It assumes Part 1 already produced a cycle list and then verifies whether a chosen cycle is still valid.

The process is:

1. Load one or more cycles from `part1/results/top10_cycles.json`
2. Deploy `CycleValidator.sol`
3. Reconstruct each cycle from `tokens[]`, `pools[]`, `amountIn`, and `minOut`
4. Read live reserves from the pair contracts on a fork
5. Recompute the output hop by hop
6. Revert if the cycle is stale or unprofitable

### Mode B: Sepolia public-testnet demonstration

In this mode, Part 2 deploys its own profitable mock cycle because the Part 1 dataset references mainnet pair addresses that do not exist on Sepolia.

The process is:

1. Deploy `CycleValidator.sol`
2. Deploy three `MockV2Pair` contracts with reserves that form a profitable cycle
3. Build the token path and pools array
4. Preview the result with `quoteCycle(...)` and `validateCycle(...)`
5. Send a real transaction to `recordValidation(...)`
6. Persist the deployment result, transaction hash, and event metadata to `part2/deployments/sepolia-demo.json`

### Part 2 module layout

`part2/contracts/interfaces/`

- external pair interfaces used by the validator

`part2/contracts/libraries/`

- reusable pure math helpers

`part2/contracts/mocks/`

- mock pair contracts used by the test suite and the Sepolia demo flow

`part2/contracts/validators/`

- production validation logic

`part2/scripts/lib/`

- shared script helpers for loading cycle data and running validation

`part2/test/helpers/`

- shared Hardhat fixtures

`part2/test/validators/`

- contract test cases

## Why Part 2 Is Optional

Part 2 is optional because the assignment can be completed with a strong off-chain search alone. It exists to show that the Part 1 results can be translated into an execution-safe validation step.

## Main Design Assumptions

- fee model: Uniswap V2 `0.3%`
- search scope: simple cycles up to `--max-cycle-length`
- liquidity filter: default `reserveUSD >= 1000`
- pricing for ranking: stablecoin-anchored propagation up to two hops
- real opportunity revalidation environment: Hardhat mainnet fork
- public testnet demonstration environment: Sepolia with deployed mock pairs

## Key Limitation

Part 1 uses static snapshot data while Part 2 reads live state. A cycle that looks profitable in Part 1 may fail in Part 2 because the market moved. That is expected and is exactly why the validation stage exists.

In the Sepolia demo mode, the proof is different: it proves the validator logic executed correctly on a public Ethereum testnet, but it does not prove that a mainnet snapshot opportunity is executable on Sepolia.
