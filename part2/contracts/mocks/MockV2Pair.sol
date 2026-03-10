// SPDX-License-Identifier: MIT
pragma solidity ^0.8.24;

/// @title MockV2Pair
/// @notice Mutable in-memory pair used by the validator test suite.
contract MockV2Pair {
    address public immutable token0;
    address public immutable token1;

    uint112 private reserve0;
    uint112 private reserve1;

    /// @notice Seeds the mock pair with its token ordering and initial reserves.
    constructor(
        address token0_,
        address token1_,
        uint112 reserve0_,
        uint112 reserve1_
    ) {
        token0 = token0_;
        token1 = token1_;
        reserve0 = reserve0_;
        reserve1 = reserve1_;
    }

    /// @notice Mutates the reserves so tests can model stale or empty liquidity.
    function setReserves(uint112 reserve0_, uint112 reserve1_) external {
        reserve0 = reserve0_;
        reserve1 = reserve1_;
    }

    /// @notice Returns the stored reserves using the Uniswap V2 return shape.
    function getReserves() external view returns (uint112, uint112, uint32) {
        return (reserve0, reserve1, 0);
    }
}
