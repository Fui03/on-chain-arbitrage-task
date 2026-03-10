/**
 * Deploys the validator and a profitable mock cycle, then validates that cycle
 * on the selected network. Use `--network sepolia` for a real testnet run.
 */
const hre = require("hardhat");
const { deployValidator, logValidationHeader } = require("./lib/validator-runner");
const {
  buildDemoValidationInput,
  deployMockCycle,
  writeDemoReport
} = require("./lib/testnet-demo");

async function main() {
  const { deployer, validator } = await deployValidator(hre);
  const mockCycle = await deployMockCycle(hre);
  const demo = await buildDemoValidationInput(mockCycle);
  const validatorAddress = await validator.getAddress();

  logValidationHeader(deployer.address, validatorAddress);
  console.log(`Network: ${hre.network.name}`);
  console.log(
    `Mode: ${
      hre.network.name === "hardhat"
        ? "local smoke test"
        : "public Ethereum testnet deployment"
    }`
  );

  const quotedOut = await validator.quoteCycle.staticCall(
    demo.tokens,
    demo.pools,
    demo.amountIn
  );
  const previewValidatedOut = await validator.validateCycle.staticCall(
    demo.tokens,
    demo.pools,
    demo.amountIn,
    demo.minOut
  );

  if (previewValidatedOut !== demo.expectedOut) {
    throw new Error(
      `Validated output ${previewValidatedOut.toString()} did not match expected output ${demo.expectedOut.toString()}`
    );
  }

  // Broadcast a real transaction so the public testnet run leaves an explorer-
  // visible proof that the validator executed successfully on-chain.
  const validationTx = await validator.recordValidation(
    demo.tokens,
    demo.pools,
    demo.amountIn,
    demo.minOut
  );
  const validationReceipt = await validationTx.wait();

  const validationEvent = validationReceipt.logs
    .map((log) => {
      try {
        return validator.interface.parseLog(log);
      } catch {
        return null;
      }
    })
    .find((parsed) => parsed && parsed.name === "CycleValidationRecorded");

  if (!validationEvent) {
    throw new Error("CycleValidationRecorded event was not found in receipt logs");
  }

  const validatedOut = validationEvent.args.amountOut;

  const reportPath = writeDemoReport(hre.network.name, {
    network: hre.network.name,
    deployer: deployer.address,
    validator: validatorAddress,
    mockPairs: {
      pairAB: await mockCycle.pairAB.getAddress(),
      pairBC: await mockCycle.pairBC.getAddress(),
      pairCA: await mockCycle.pairCA.getAddress()
    },
    cycle: {
      tokens: demo.tokens,
      pools: demo.pools,
      amountInRaw: demo.amountIn.toString(),
      minOutRaw: demo.minOut.toString(),
      expectedOutRaw: demo.expectedOut.toString(),
      quotedOutRaw: quotedOut.toString(),
      previewValidatedOutRaw: previewValidatedOut.toString(),
      validatedOutRaw: validatedOut.toString(),
      cycleHash: validationEvent.args.cycleHash
    },
    proof: {
      validationTxHash: validationReceipt.hash,
      validationBlockNumber: validationReceipt.blockNumber,
      gasUsed: validationReceipt.gasUsed.toString()
    }
  });

  console.log(`amountInRaw: ${demo.amountIn.toString()}`);
  console.log(`expectedOutRaw: ${demo.expectedOut.toString()}`);
  console.log(`previewValidatedOutRaw: ${previewValidatedOut.toString()}`);
  console.log(`validatedOutRaw: ${validatedOut.toString()}`);
  console.log(`validationTxHash: ${validationReceipt.hash}`);
  console.log(`Demo report written to: ${reportPath}`);
}

main().catch((error) => {
  console.error(error);
  process.exitCode = 1;
});
