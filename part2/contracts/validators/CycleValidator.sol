// SPDX-License-Identifier: MIT
pragma solidity ^0.8.24;

import "../interfaces/IUniswapV2PairLike.sol";
import "../libraries/CycleMath.sol";

/// @title CycleValidator
/// @notice Recomputes a candidate cycle against live pair reserves.
/// @dev This contract validates path shape and profitability but does not
/// execute swaps.
contract CycleValidator {
    error InvalidPathLength();
    error InvalidCycle();
    error InvalidAmountIn();
    error ZeroAddressInPath();
    error InvalidPoolTokenPair(address pool, address tokenIn, address tokenOut);
    error InsufficientLiquidity(address pool);
    error NotProfitable(uint256 amountIn, uint256 amountOut);
    error MinOutNotMet(uint256 minOut, uint256 amountOut);

    /// @notice Emitted when a cycle is validated through a real transaction.
    event CycleValidationRecorded(
        address indexed caller,
        bytes32 indexed cycleHash,
        uint256 amountIn,
        uint256 minOut,
        uint256 amountOut
    );

    /// @notice Quotes a cycle and enforces profitability plus a caller-supplied
    /// minimum acceptable output.
    function validateCycle(
        address[] calldata tokens,
        address[] calldata pools,
        uint256 amountIn,
        uint256 minOut
    ) external view returns (uint256 amountOut) {
        amountOut = quoteCycle(tokens, pools, amountIn);

        if (amountOut <= amountIn) {
            revert NotProfitable(amountIn, amountOut);
        }
        if (amountOut < minOut) {
            revert MinOutNotMet(minOut, amountOut);
        }
    }

    /// @notice Executes the same validation logic and emits an event so a
    /// public testnet run has an on-chain proof artifact.
    function recordValidation(
        address[] calldata tokens,
        address[] calldata pools,
        uint256 amountIn,
        uint256 minOut
    ) external returns (uint256 amountOut) {
        amountOut = quoteCycle(tokens, pools, amountIn);

        if (amountOut <= amountIn) {
            revert NotProfitable(amountIn, amountOut);
        }
        if (amountOut < minOut) {
            revert MinOutNotMet(minOut, amountOut);
        }

        emit CycleValidationRecorded(
            msg.sender,
            keccak256(abi.encode(tokens, pools)),
            amountIn,
            minOut,
            amountOut
        );
    }

    /// @notice Quotes the final output for a cyclic path using current reserves.
    function quoteCycle(
        address[] calldata tokens,
        address[] calldata pools,
        uint256 amountIn
    ) public view returns (uint256 amountOut) {
        _validatePath(tokens, pools, amountIn);

        uint256 runningAmount = amountIn;
        for (uint256 i = 0; i < pools.length; ++i) {
            runningAmount = _simulateSwap(
                pools[i],
                tokens[i],
                tokens[i + 1],
                runningAmount
            );
        }

        return runningAmount;
    }

    /// @dev Validates the basic path invariants before any external reads.
    function _validatePath(
        address[] calldata tokens,
        address[] calldata pools,
        uint256 amountIn
    ) internal pure {
        if (tokens.length != pools.length + 1 || pools.length == 0) {
            revert InvalidPathLength();
        }
        if (tokens[0] != tokens[tokens.length - 1]) {
            revert InvalidCycle();
        }
        if (amountIn == 0) {
            revert InvalidAmountIn();
        }
        for (uint256 i = 0; i < pools.length; ++i) {
            if (
                pools[i] == address(0) ||
                tokens[i] == address(0) ||
                tokens[i + 1] == address(0)
            ) {
                revert ZeroAddressInPath();
            }
        }
    }

    /// @dev Replays one swap hop after verifying that the pool matches the path.
    function _simulateSwap(
        address pool,
        address tokenIn,
        address tokenOut,
        uint256 amountIn
    ) internal view returns (uint256 amountOut) {
        IUniswapV2PairLike pair = IUniswapV2PairLike(pool);
        address poolToken0 = pair.token0();
        address poolToken1 = pair.token1();
        (uint112 reserve0, uint112 reserve1,) = pair.getReserves();

        if (reserve0 == 0 || reserve1 == 0) {
            revert InsufficientLiquidity(pool);
        }

        if (tokenIn == poolToken0 && tokenOut == poolToken1) {
            return CycleMath.getAmountOut(amountIn, reserve0, reserve1);
        }
        if (tokenIn == poolToken1 && tokenOut == poolToken0) {
            return CycleMath.getAmountOut(amountIn, reserve1, reserve0);
        }

        revert InvalidPoolTokenPair(pool, tokenIn, tokenOut);
    }
}
