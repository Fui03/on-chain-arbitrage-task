/**
 * Deploys the validator and replays one or more ranked Part 1 cycles against
 * the current forked chain state.
 */
const hre = require("hardhat");
const { loadCyclesFile, selectCycles } = require("./lib/cycle-file");
const {
  deployValidator,
  ensureForkHasLivePool,
  logCycleInput,
  logValidationHeader,
  quoteAndValidateCycle
} = require("./lib/validator-runner");

async function main() {
  const cycleIndex = Number(process.env.CYCLE_INDEX || "0");
  const maxCycles = Number(process.env.MAX_CYCLES || "1");
  const { parsed } = loadCyclesFile(process.env.CYCLES_PATH);
  const selectedCycles = selectCycles(parsed.opportunities, cycleIndex, maxCycles);
  const { deployer, validator } = await deployValidator(hre);

  logValidationHeader(deployer.address, await validator.getAddress());
  await ensureForkHasLivePool(hre.ethers.provider, selectedCycles[0].pools[0]);

  for (let offset = 0; offset < selectedCycles.length; offset += 1) {
    const currentIndex = cycleIndex + offset;
    const cycle = selectedCycles[offset];
    logCycleInput(currentIndex, cycle);

    try {
      const result = await quoteAndValidateCycle(validator, cycle);
      console.log(`quotedAmountOutRaw: ${result.quotedOut.toString()}`);
      console.log(
        `quotedMinusSnapshotRaw: ${result.quotedMinusSnapshot.toString()}`
      );
      console.log(`validatedAmountOutRaw: ${result.validatedOut.toString()}`);
    } catch (error) {
      const message =
        error.shortMessage || error.reason || error.message || String(error);
      console.error(`Validation reverted: ${message}`);
      process.exitCode = 1;
    }
  }
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
