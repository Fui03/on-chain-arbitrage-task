/**
 * Runtime helpers for deploying the validator and replaying one or more ranked
 * cycle candidates.
 */

/**
 * Deploys the validator contract with the current Hardhat signer.
 */
async function deployValidator(hre) {
  const [deployer] = await hre.ethers.getSigners();
  if (!deployer) {
    throw new Error(
      "No deployer account is configured for this network. Set TESTNET_PRIVATE_KEY for public testnet runs."
    );
  }
  const factory = await hre.ethers.getContractFactory("CycleValidator");
  const validator = await factory.deploy();
  await validator.waitForDeployment();

  return { deployer, validator };
}

/**
 * Ensures the configured provider is actually backed by a fork containing the
 * requested live pool contract.
 */
async function ensureForkHasLivePool(provider, poolAddress) {
  const poolCode = await provider.getCode(poolAddress);
  if (poolCode === "0x") {
    throw new Error(
      "Live pool bytecode was not found. Run this script against a Hardhat mainnet fork by setting ENABLE_MAINNET_FORK=1 and MAINNET_RPC_URL."
    );
  }
}

/**
 * Formats a readable token path for console output.
 */
function formatTokenPath(cycle) {
  return (cycle.tokenLabels || cycle.tokens).join(" -> ");
}

/**
 * Prints the deployment context before cycle replay begins.
 */
function logValidationHeader(deployerAddress, validatorAddress) {
  console.log(`Deployer: ${deployerAddress}`);
  console.log(`Validator: ${validatorAddress}`);
}

/**
 * Prints the selected cycle inputs so validation output can be interpreted.
 */
function logCycleInput(index, cycle) {
  console.log(`\nCycle index: ${index}`);
  console.log(`Tokens: ${formatTokenPath(cycle)}`);
  console.log(`amountInRaw: ${cycle.amountInRaw}`);
  console.log(`minOutRaw: ${cycle.minOutRaw}`);
  console.log(`snapshotAmountOutRaw: ${cycle.amountOutRaw}`);
}

/**
 * Quotes and validates a cycle against the deployed validator contract.
 */
async function quoteAndValidateCycle(validator, cycle) {
  const quotedOut = await validator.quoteCycle.staticCall(
    cycle.tokens,
    cycle.pools,
    BigInt(cycle.amountInRaw)
  );
  const validatedOut = await validator.validateCycle.staticCall(
    cycle.tokens,
    cycle.pools,
    BigInt(cycle.amountInRaw),
    BigInt(cycle.minOutRaw)
  );

  return {
    quotedOut,
    validatedOut,
    quotedMinusSnapshot: quotedOut - BigInt(cycle.amountOutRaw)
  };
}

module.exports = {
  deployValidator,
  ensureForkHasLivePool,
  logCycleInput,
  logValidationHeader,
  quoteAndValidateCycle
};
