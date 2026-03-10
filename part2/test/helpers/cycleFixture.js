/**
 * Shared test fixture builders for deterministic cycle validator scenarios.
 */
const { ethers } = require("hardhat");

const E18 = 10n ** 18n;

/**
 * Mirrors the Solidity library formula so tests can compute expected outputs.
 */
function getAmountOut(amountIn, reserveIn, reserveOut) {
  const amountInWithFee = amountIn * 997n;
  const numerator = amountInWithFee * reserveOut;
  const denominator = reserveIn * 1000n + amountInWithFee;
  return numerator / denominator;
}

/**
 * Deploys a validator plus a small set of mock pairs used across the tests.
 */
async function deployCycleFixture() {
  const tokenA = ethers.Wallet.createRandom().address;
  const tokenB = ethers.Wallet.createRandom().address;
  const tokenC = ethers.Wallet.createRandom().address;

  const MockPair = await ethers.getContractFactory("MockV2Pair");
  const Validator = await ethers.getContractFactory("CycleValidator");
  const validator = await Validator.deploy();

  const pairAB = await MockPair.deploy(tokenA, tokenB, 1000n * E18, 1000n * E18);
  const pairBC = await MockPair.deploy(tokenB, tokenC, 1000n * E18, 1000n * E18);
  const pairCA = await MockPair.deploy(tokenC, tokenA, 800n * E18, 1000n * E18);
  const flatCA = await MockPair.deploy(tokenC, tokenA, 1000n * E18, 1000n * E18);

  return { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA, flatCA };
}

/**
 * Builds the repeated token path and pool list expected by the validator.
 */
async function buildCyclePath(tokenA, tokenB, tokenC, pairAB, pairBC, pairCA) {
  return {
    tokens: [tokenA, tokenB, tokenC, tokenA],
    pools: [
      await pairAB.getAddress(),
      await pairBC.getAddress(),
      await pairCA.getAddress()
    ]
  };
}

module.exports = {
  E18,
  buildCyclePath,
  deployCycleFixture,
  getAmountOut
};
