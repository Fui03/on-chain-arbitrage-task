# DEX Arbitrage Mini Challenge Report

## 1. Goal

The task is to detect profitable arbitrage cycles from a snapshot of Uniswap V2 pools and then provide an optional on-chain check that recomputes profitability before execution. I implemented:

- a C++20 off-chain solver for simple-cycle arbitrage detection and ranking
- a Solidity validator plus Hardhat script for optional live reserve validation

I also refactored the codebase into smaller responsibility-focused modules:

- Part 1 separates application flow, domain logic, IO, presentation, and utility helpers
- within Part 1, the domain layer is further split into `model`, `graph`, `pricing`, and `opportunity` subfolders
- Part 2 separates interfaces, math libraries, validator contracts, script helpers, and test fixtures

## 2. Dataset Interpretation

The snapshot contains:

- `38,141` pool objects in total
- no duplicate unordered token pairs
- no explicit fee field
- token metadata limited to address and decimals

Because many pools are dust-sized, I filtered the graph to pools with `reserveUSD >= 1000`. Under that filter the working graph contains:

- `5,448` pools loaded by the solver
- `4,624` unique tokens
- `30,434` unique simple cycles when run with `--max-cycle-length 6`

This makes short-cycle search both practical and well aligned with the data.

## 3. Graph Construction

Each pool is treated as two directed swap edges:

- `token0 -> token1` with reserves `(reserve0, reserve1)`
- `token1 -> token0` with reserves `(reserve1, reserve0)`

The solver builds:

- a token index map from address to integer id
- an undirected adjacency list for simple-cycle enumeration
- a pair-to-pool lookup for quick edge reconstruction
- an anchor-based USD pricing table used for more realistic ranking

## 4. AMM Pricing Model

I used the standard Uniswap V2 constant-product formula with a `0.3%` fee:

```text
amountOut = (0.997 * amountIn * reserveOut) / (reserveIn + 0.997 * amountIn)
```

For a multi-hop cycle, the output of one swap becomes the input of the next. Profit is:

```text
profit = finalAmount - initialAmount
```

## 5. Cycle Detection Logic

The search enumerates undirected simple cycles up to a configurable maximum length, with `--max-cycle-length 4` used in the checked-in run.

For each undirected cycle:

- I evaluate both traversal directions
- I rotate the start token so the same cycle can be priced from each possible starting asset
- I then de-duplicate rotation-equivalent directed cycles and keep only the best-scoring rotation for ranking/output

Before simulating trade sizes, I apply a marginal-product screen:

```text
Π(0.997 * reserveOut / reserveIn) > 1
```

If the infinitesimal product is not profitable, the candidate is skipped immediately.

## 6. Trade Size Suggestion

For each surviving cycle, I search over trade size in two phases:

1. a log-spaced coarse scan from `1e-9 * firstReserve` to `0.05 * firstReserve`
2. a local golden-section refinement around the best coarse point

This produces a practical input size without pretending to solve a global optimization problem over the full graph.

The solver also converts the chosen input and output into raw integer token units using the start token decimals. `minOutRaw` is set to:

```text
amountInRaw + floor(0.9 * expectedProfitRaw)
```

That leaves a small buffer for reserve drift when replaying the cycle on-chain.

## 7. Ranking

The initial version used a naive median USD estimate per token, but that overvalued obscure assets and made the Top-10 noisy. I replaced it with an anchor-based pricing model:

- stablecoins such as `USDC`, `USDT`, and `DAI` are seeded at `$1`
- prices are propagated outward across the pool graph for up to two hops
- the solver tracks whether a starting token has a trusted anchored valuation, how many hops that valuation took, and the supporting pool liquidity

The primary ranking metric is:

```text
estimatedProfitUsd = profitHuman * anchoredUsdPrice[startToken]
```

Tie-breakers:

1. trusted USD valuation beats unanchored valuation
2. lower valuation hop count
3. higher valuation-supporting liquidity
4. higher `profitPct`
5. higher bottleneck pool liquidity

This produces a more practical ranking than simply multiplying by arbitrary per-pool price estimates.

## 8. Snapshot Results

Running the solver with:

```bash
./bin/arb_detect --input ../v2pools.json --output results/top10_cycles.json --top 10 --min-reserve-usd 1000 --max-input-share 0.05 --max-cycle-length 6
```

produced:

- `30,434` enumerated cycles
- `99,037` profitable candidate rotations before de-duplication
- `24,970` unique directed cycles after rotation de-duplication
- `10` final ranked opportunities in the submission output

The generated machine-readable output is stored in `part1/results/top10_cycles.json`.

## 9. On-Chain Validation Design

`CycleValidator.sol` accepts:

- `tokens[]`
- `pools[]`
- `amountIn`
- `minOut`

Validation steps:

1. verify `tokens.length == pools.length + 1`
2. verify the path is a cycle by checking `tokens[0] == tokens[last]`
3. reject zero addresses and zero `amountIn`
4. for each hop, read `token0`, `token1`, and `getReserves()` from the pair
5. verify the pair actually matches the adjacent token path
6. recompute each swap using `997 / 1000` fee math
7. expose `quoteCycle(...)` for raw recomputation
8. revert in `validateCycle(...)` if the final amount is not greater than `amountIn`
9. revert if the final amount is below `minOut`

For the public-testnet demonstration, I also added a transaction-backed proof path:

- `recordValidation(...)` reuses the same validation logic
- it emits `CycleValidationRecorded`
- this gives the Sepolia demo an explorer-visible on-chain proof instead of relying only on `eth_call`

The included Hardhat tests cover:

- profitable cycle returns `amountOut`
- profitable cycle emits an on-chain validation event
- raw cycle quoting without profitability enforcement
- invalid path length
- non-cyclic token path
- zero `amountIn`
- zero address in the path
- mismatched pools
- unprofitable cycle
- over-strict `minOut`
- zero-liquidity pool

## 10. Limitations

- The brief asks for arbitrage cycle detection in general; this implementation supports simple cycles up to a configurable maximum length rather than attempting unbounded exhaustive cycle search.
- `reserveUSD` is a snapshot-derived field and may not reflect current live prices.
- Part 2 is inherently time-sensitive: a cycle found off-chain from the snapshot can stop being profitable before the on-chain validator reads live reserves.
- The dataset does not include token symbols, so the console output uses addresses.
- The Sepolia demo proves that the validator works on a public Ethereum testnet, but it uses deployed mock pairs rather than the mainnet snapshot pool addresses.

## 11. Verified Sepolia Proof

A successful public-testnet demonstration was completed on March 10, 2026.

- deployer: `0x9e61B993e4cDcEF54842B6B71D419827Bc567cb0`
- validator: `0x3A9a912C1ed0A34C3314504EaD8466615e5E967D`
- validation transaction: `0x6f43a15cf2d03ee6cc070c1f0eee6a35d00110ca17c81c1f96a7d91e43cf7745`
- block number: `10415564`
- gas used: `58164`

The on-chain proof values matched exactly across the preview and recorded validation path:

- `amountInRaw = 10000000000000000000`
- `expectedOutRaw = 12000253838105361951`
- `previewValidatedOutRaw = 12000253838105361951`
- `validatedOutRaw = 12000253838105361951`
- `cycleHash = 0x6b7c1dba6b10a2fa50c38253d90b6fb9ab329c61e9fbffe9e884a54dfb4b2786`

The result file `part2/deployments/sepolia-demo.json` records:

- the deployed validator and mock pair addresses
- the cycle inputs
- the previewed outputs
- the final event-backed validation result
- the transaction hash, block number, and gas usage

This proves that the validator logic was not only deployed to Sepolia, but also executed successfully through a real on-chain transaction that emitted `CycleValidationRecorded`.

## 12. AI Tools Used

I used Codex to accelerate:

- task decomposition and approach selection
- code scaffolding for the C++ solver and Solidity validator
- test generation
- documentation drafting

How AI helped:

- it made it faster to explore the dataset structure and iterate from a triangle-only baseline to a configurable simple-cycle search
- it accelerated repetitive implementation work such as CLI wiring, ranking output, and test scaffolding
- it helped produce a cleaner written explanation of the method

Problems and limitations:

- the original plan assumed a local C++ JSON library, but none was installed and external downloads were restricted
- Hardhat dependency installation and local contract tests were eventually completed, but live mainnet-fork validation still depends on a responsive RPC endpoint
- generated code still required manual review and correction, especially around option plumbing and build consistency
