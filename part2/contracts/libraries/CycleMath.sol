// SPDX-License-Identifier: MIT
pragma solidity ^0.8.24;

/// @title CycleMath
/// @notice Pure fee-adjusted swap quoting helpers for Uniswap V2 style pairs.
library CycleMath {
    uint256 internal constant FEE_NUMERATOR = 997;
    uint256 internal constant FEE_DENOMINATOR = 1000;

    /// @notice Quotes the output amount for one swap hop.
    function getAmountOut(
        uint256 amountIn,
        uint256 reserveIn,
        uint256 reserveOut
    ) internal pure returns (uint256) {
        uint256 amountInWithFee = amountIn * FEE_NUMERATOR;
        uint256 numerator = amountInWithFee * reserveOut;
        uint256 denominator = reserveIn * FEE_DENOMINATOR + amountInWithFee;
        return numerator / denominator;
    }
}
