# Output Reference

This document explains the structure of `part1/results/top10_cycles.json`.

## Top-Level Fields

### `input`

The path to the snapshot file used for the run.

### `minReserveUsd`

The liquidity filter threshold applied before graph construction.

### `maxInputShare`

The maximum fraction of the first pool reserve that the optimizer is allowed to use as the input amount.

### `maxCycleLength`

The maximum simple-cycle length explored during the run.

### `topN`

The number of opportunities kept after sorting.

### `filteredPoolCount`

How many pools survived the basic reserve and liquidity filtering.

### `filteredTokenCount`

How many unique tokens remain in the filtered graph.

### `enumeratedCycleCount`

How many unique simple cycles were enumerated under the configured cycle-length cap.

### `profitableCandidates`

How many directed cycle candidates survived evaluation before rotation de-duplication.

### `uniqueDirectedCycles`

How many unique directed cycles remain after collapsing rotation-equivalent duplicates.

### `returnedOpportunities`

How many opportunities were finally written to the output file.

## Per-Opportunity Fields

### `tokens`

The cycle path expressed as token addresses, with the first token repeated at the end.

Example:

```text
[A, B, C, A]
```

### `tokenLabels`

Human-readable labels for the token path. Well-known assets use real labels such as `WETH` or `USDC`. Unknown assets fall back to shortened addresses.

### `pools`

The pair addresses used for each hop in the cycle.

### `cycleLength`

How many swap hops are included in the cycle.

### `amountInHuman`

The suggested starting amount in human-readable token units.

### `amountInRaw`

The same input amount converted to raw on-chain integer units using the starting token decimals.

### `amountOutHuman`

The final output amount in human-readable token units after simulating the full cycle.

### `amountOutRaw`

The final output converted to raw integer units.

### `minOutRaw`

The recommended minimum output for Part 2 validation. It is set slightly below the expected output so small reserve drift can be tolerated without accepting an obviously stale trade.

### `profitHuman`

The simulated net profit in the starting token.

### `profitPct`

The simulated percentage profit relative to `amountInHuman`.

### `estimatedProfitUsd`

The profit converted into an anchored USD estimate, if the starting token can be priced credibly from the stablecoin anchor logic.

### `hasTrustedUsdValuation`

Whether the starting token received a trusted USD anchor price.

### `usdValuationHops`

How many graph hops away the anchored valuation came from:

- `0`: stablecoin itself
- `1`: directly connected to an anchored token
- `2`: two hops from an anchored token
- `-1`: no trusted anchored valuation

### `usdValuationConfidenceUsd`

The supporting liquidity used when selecting the anchored price candidate.

### `bottleneckReserveUsd`

The smallest `reserveUSD` value among the pools in the cycle. This is useful as a quick liquidity sanity check, especially when comparing otherwise similar routes with different hop counts.

### `marginalProduct`

The infinitesimal product:

```text
Π(0.997 * reserveOut / reserveIn)
```

If this value is less than or equal to `1`, the cycle is not profitable for tiny trades and is skipped early by the solver.

### `startTokenDecimals`

The decimal precision used to convert between human-readable and raw token amounts for the starting token.

## How To Read The File In Practice

Use the fields in this order:

1. confirm `returnedOpportunities > 0`
2. inspect `tokenLabels` and `pools`
3. compare `amountInHuman`, `amountOutHuman`, and `profitHuman`
4. check `estimatedProfitUsd`
5. verify `hasTrustedUsdValuation`
6. use `amountInRaw` and `minOutRaw` when moving into Part 2

## Important Interpretation Warning

This file is based on a static snapshot. It is not proof that a trade is executable right now. Always treat it as a candidate list and revalidate against live reserves before execution.
