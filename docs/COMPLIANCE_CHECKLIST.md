# Compliance Checklist

This checklist maps the repository deliverables to the challenge requirements that the project was built to satisfy.

## Part 1 Requirements

### 1. Build an off-chain arbitrage detector from the supplied pool snapshot

Status:

- Complete

Evidence:

- `part1/src/`
- `part1/Makefile`
- `part1/bin/arb_detect`

### 2. Use AMM swap math to evaluate cycle profitability

Status:

- Complete

Evidence:

- `part1/src/domain/pricing/amm.hpp`
- `part1/src/domain/opportunity/cycle_evaluator.cpp`
- `part1/src/domain/opportunity/trade_size_optimizer.cpp`

### 3. Search for profitable cycles and rank the best opportunities

Status:

- Complete

Evidence:

- `part1/src/domain/graph/cycle_enumerator.cpp`
- `part1/src/domain/opportunity/opportunity_ranker.cpp`
- `part1/results/top10_cycles.json`

### 4. Output machine-readable results with amounts and metadata

Status:

- Complete

Evidence:

- `part1/src/io/result_writer.cpp`
- `part1/results/top10_cycles.json`
- `docs/OUTPUT_REFERENCE.md`

### 5. Include tests for the off-chain logic

Status:

- Complete

Evidence:

- `part1/tests/test_main.cpp`

## Part 2 Requirements

### 6. Provide an on-chain validator that recomputes a cycle from reserves

Status:

- Complete

Evidence:

- `part2/contracts/validators/CycleValidator.sol`
- `part2/contracts/libraries/CycleMath.sol`

### 7. Validate cycles using live reserves on a fork

Status:

- Complete

Evidence:

- `part2/scripts/validate-cycle.js`
- `part2/scripts/lib/validator-runner.js`
- `README.md`

### 8. Demonstrate the validator on a public Ethereum testnet

Status:

- Complete

Evidence:

- `part2/scripts/run-testnet-demo.js`
- `part2/deployments/sepolia-demo.json`
- `README.md`
- `REPORT.md`

### 9. Include tests for the on-chain validator

Status:

- Complete

Evidence:

- `part2/test/validators/CycleValidator.test.js`

## Documentation Requirements

### 10. Explain what to run first and how the project works

Status:

- Complete

Evidence:

- `README.md`
- `docs/RUNBOOK.md`
- `docs/ARCHITECTURE.md`

### 11. Explain the output and limitations

Status:

- Complete

Evidence:

- `docs/OUTPUT_REFERENCE.md`
- `REPORT.md`

### 12. Disclose AI usage and development limitations

Status:

- Complete

Evidence:

- `REPORT.md`

## What Is Proven

The repository now proves all of the following:

- Part 1 can detect and rank profitable candidate cycles from the supplied snapshot
- Part 2 can recompute cycle profitability from reserves
- the validator logic runs on a public Ethereum testnet
- the public testnet demo produces an explorer-visible validation transaction

## What Is Not Proven

The repository does not claim any of the following:

- that a Part 1 snapshot opportunity is still profitable on Sepolia
- that the validator performs real swap execution
- that snapshot profitability guarantees present-day mainnet profitability

Those are outside the scope of the current design and are explicitly documented as limitations.
