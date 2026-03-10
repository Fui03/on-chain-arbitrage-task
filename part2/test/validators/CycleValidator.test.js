/**
 * Contract tests for path validation, swap quoting, and profitability checks.
 */
const { expect } = require("chai");
const { anyValue } = require("@nomicfoundation/hardhat-chai-matchers/withArgs");
const {
  E18,
  buildCyclePath,
  deployCycleFixture,
  getAmountOut
} = require("../helpers/cycleFixture");

describe("CycleValidator", function () {
  it("returns amountOut for a profitable cycle", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      pairCA
    );

    const amountIn = 10n * E18;
    const amountAfterAB = getAmountOut(amountIn, 1000n * E18, 1000n * E18);
    const amountAfterBC = getAmountOut(amountAfterAB, 1000n * E18, 1000n * E18);
    const expectedOut = getAmountOut(amountAfterBC, 800n * E18, 1000n * E18);
    const minOut = amountIn + ((expectedOut - amountIn) * 9n) / 10n;

    const actual = await validator.validateCycle.staticCall(
      tokens,
      pools,
      amountIn,
      minOut
    );

    expect(actual).to.equal(expectedOut);
  });

  it("records an on-chain validation event for a profitable cycle", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      pairCA
    );

    const amountIn = 10n * E18;
    const amountAfterAB = getAmountOut(amountIn, 1000n * E18, 1000n * E18);
    const amountAfterBC = getAmountOut(amountAfterAB, 1000n * E18, 1000n * E18);
    const expectedOut = getAmountOut(amountAfterBC, 800n * E18, 1000n * E18);
    const minOut = amountIn + ((expectedOut - amountIn) * 9n) / 10n;

    await expect(
      validator.recordValidation(tokens, pools, amountIn, minOut)
    )
      .to.emit(validator, "CycleValidationRecorded")
      .withArgs(anyValue, anyValue, amountIn, minOut, expectedOut);
  });

  it("quotes a cycle without profitability checks", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, flatCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      flatCA
    );

    const amountIn = 1n * E18;
    const amountAfterAB = getAmountOut(amountIn, 1000n * E18, 1000n * E18);
    const amountAfterBC = getAmountOut(amountAfterAB, 1000n * E18, 1000n * E18);
    const expectedOut = getAmountOut(amountAfterBC, 1000n * E18, 1000n * E18);

    const actual = await validator.quoteCycle.staticCall(tokens, pools, amountIn);
    expect(actual).to.equal(expectedOut);
  });

  it("reverts on invalid path length", async function () {
    const { validator, tokenA, tokenB, pairAB } = await deployCycleFixture();
    const tokens = [tokenA, tokenB, tokenA];
    const pools = [await pairAB.getAddress()];

    await expect(
      validator.quoteCycle.staticCall(tokens, pools, 1n * E18)
    ).to.be.revertedWithCustomError(validator, "InvalidPathLength");
  });

  it("reverts on a non-cyclic token path", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const tokens = [tokenA, tokenB, tokenC, tokenB];
    const pools = [
      await pairAB.getAddress(),
      await pairBC.getAddress(),
      await pairCA.getAddress()
    ];

    await expect(
      validator.quoteCycle.staticCall(tokens, pools, 1n * E18)
    ).to.be.revertedWithCustomError(validator, "InvalidCycle");
  });

  it("reverts on zero amountIn", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      pairCA
    );

    await expect(
      validator.quoteCycle.staticCall(tokens, pools, 0n)
    ).to.be.revertedWithCustomError(validator, "InvalidAmountIn");
  });

  it("reverts on zero address in path", async function () {
    const { validator, tokenA, tokenB, pairAB } = await deployCycleFixture();
    const tokens = [tokenA, tokenB, tokenA];
    const pools = [
      await pairAB.getAddress(),
      "0x0000000000000000000000000000000000000000"
    ];

    await expect(
      validator.quoteCycle.staticCall(tokens, pools, 1n * E18)
    ).to.be.revertedWithCustomError(validator, "ZeroAddressInPath");
  });

  it("reverts when pools do not match the token path", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const { tokens } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      pairCA
    );
    const pools = [
      await pairAB.getAddress(),
      await pairCA.getAddress(),
      await pairBC.getAddress()
    ];

    await expect(
      validator.validateCycle.staticCall(tokens, pools, 1n * E18, 1n * E18)
    ).to.be.revertedWithCustomError(validator, "InvalidPoolTokenPair");
  });

  it("reverts when the cycle is not profitable", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, flatCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      flatCA
    );

    await expect(
      validator.validateCycle.staticCall(tokens, pools, 1n * E18, 1n * E18)
    ).to.be.revertedWithCustomError(validator, "NotProfitable");
  });

  it("reverts when minOut is set above the recomputed output", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      pairCA
    );

    await expect(
      validator.validateCycle.staticCall(tokens, pools, 10n * E18, 1000n * E18)
    ).to.be.revertedWithCustomError(validator, "MinOutNotMet");
  });

  it("reverts when a pool has zero liquidity", async function () {
    const { validator, tokenA, tokenB, tokenC, pairAB, pairBC, pairCA } =
      await deployCycleFixture();
    const { tokens, pools } = await buildCyclePath(
      tokenA,
      tokenB,
      tokenC,
      pairAB,
      pairBC,
      pairCA
    );

    await pairBC.setReserves(0n, 0n);

    await expect(
      validator.quoteCycle.staticCall(tokens, pools, 1n * E18)
    ).to.be.revertedWithCustomError(validator, "InsufficientLiquidity");
  });
});
