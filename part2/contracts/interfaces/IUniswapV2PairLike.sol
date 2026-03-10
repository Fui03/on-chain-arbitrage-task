// SPDX-License-Identifier: MIT
pragma solidity ^0.8.24;

/// @notice Minimal Uniswap V2 pair surface required by the validator.
interface IUniswapV2PairLike {
    /// @notice Returns the first token stored by the pair contract.
    function token0() external view returns (address);

    /// @notice Returns the second token stored by the pair contract.
    function token1() external view returns (address);

    /// @notice Returns the pair reserves in token0/token1 order.
    function getReserves() external view returns (uint112, uint112, uint32);
}
