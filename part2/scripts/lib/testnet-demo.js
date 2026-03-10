/**
 * Helpers for deploying a profitable mock cycle to either a local Hardhat
 * network or a public Ethereum testnet such as Sepolia.
 */
const fs = require("fs");
const path = require("path");

const E18 = 10n ** 18n;

/**
 * Mirrors the validator math so the script can log the expected output.
 */
function getAmountOut(amountIn, reserveIn, reserveOut) {
  const amountInWithFee = amountIn * 997n;
  const numerator = amountInWithFee * reserveOut;
  const denominator = reserveIn * 1000n + amountInWithFee;
  return numerator / denominator;
}

/**
 * Deploys three mock pairs that form a profitable A -> B -> C -> A cycle.
 */
async function deployMockCycle(hre) {
  const tokenA = hre.ethers.Wallet.createRandom().address;
  const tokenB = hre.ethers.Wallet.createRandom().address;
  const tokenC = hre.ethers.Wallet.createRandom().address;
  const MockPair = await hre.ethers.getContractFactory("MockV2Pair");

  const pairAB = await MockPair.deploy(tokenA, tokenB, 1000n * E18, 1000n * E18);
  const pairBC = await MockPair.deploy(tokenB, tokenC, 1000n * E18, 1000n * E18);
  const pairCA = await MockPair.deploy(tokenC, tokenA, 800n * E18, 1000n * E18);

  await Promise.all([
    pairAB.waitForDeployment(),
    pairBC.waitForDeployment(),
    pairCA.waitForDeployment()
  ]);

  return {
    tokenA,
    tokenB,
    tokenC,
    pairAB,
    pairBC,
    pairCA
  };
}

/**
 * Builds the validator calldata and expected values for the mock cycle.
 */
async function buildDemoValidationInput(mockCycle) {
  const tokens = [
    mockCycle.tokenA,
    mockCycle.tokenB,
    mockCycle.tokenC,
    mockCycle.tokenA
  ];
  const pools = [
    await mockCycle.pairAB.getAddress(),
    await mockCycle.pairBC.getAddress(),
    await mockCycle.pairCA.getAddress()
  ];

  const amountIn = 10n * E18;
  const amountAfterAB = getAmountOut(amountIn, 1000n * E18, 1000n * E18);
  const amountAfterBC = getAmountOut(amountAfterAB, 1000n * E18, 1000n * E18);
  const expectedOut = getAmountOut(amountAfterBC, 800n * E18, 1000n * E18);
  const minOut = amountIn + ((expectedOut - amountIn) * 9n) / 10n;

  return {
    tokens,
    pools,
    amountIn,
    expectedOut,
    minOut
  };
}

/**
 * Persists the testnet or local demo result so it can be referenced later.
 */
function writeDemoReport(networkName, report) {
  const outputDir = path.resolve(__dirname, "../../deployments");
  fs.mkdirSync(outputDir, { recursive: true });
  const outputPath = path.join(outputDir, `${networkName}-demo.json`);
  fs.writeFileSync(outputPath, JSON.stringify(report, null, 2));
  return outputPath;
}

module.exports = {
  buildDemoValidationInput,
  deployMockCycle,
  getAmountOut,
  writeDemoReport
};
