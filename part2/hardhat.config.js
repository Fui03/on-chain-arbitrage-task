/**
 * Hardhat configuration for local compilation, testing, and optional public
 * network / mainnet-fork validation of the cycle validator.
 *
 * Local commands such as `npm test` should remain deterministic and must not
 * accidentally fork mainnet just because a placeholder environment variable was
 * exported in the shell. Forking is therefore opt-in via ENABLE_MAINNET_FORK.
 */
require("@nomicfoundation/hardhat-toolbox");

/**
 * Returns true when an environment variable looks like a real RPC URL instead
 * of one of the placeholder values shown in the documentation.
 */
function hasUsableRpcUrl(envName) {
  const rawValue = process.env[envName];
  if (!rawValue) {
    return false;
  }

  const normalized = rawValue.trim().toLowerCase();
  if (
    !normalized ||
    normalized.includes("your-mainnet-rpc") ||
    normalized.includes("your-sepolia-rpc") ||
    normalized.includes("your_api_key") ||
    normalized.includes("your-project-id")
  ) {
    return false;
  }

  try {
    const parsedUrl = new URL(rawValue);
    return parsedUrl.protocol === "https:" || parsedUrl.protocol === "http:";
  } catch {
    return false;
  }
}

/**
 * Returns true when the configured private key is present and has the expected
 * 32-byte hex encoding. Invalid placeholders are ignored so local commands do
 * not fail during config loading.
 */
function hasUsablePrivateKey(envName) {
  const rawValue = process.env[envName];
  if (!rawValue) {
    return false;
  }

  const normalized = rawValue.trim();
  if (normalized.toLowerCase().includes("your")) {
    return false;
  }

  return /^0x[0-9a-fA-F]{64}$/.test(normalized);
}

/**
 * Mainnet forking is intentionally opt-in so local tests never depend on a
 * remote RPC endpoint unless the caller explicitly asks for it.
 */
function shouldEnableMainnetFork() {
  return process.env.ENABLE_MAINNET_FORK === "1" &&
    hasUsableRpcUrl("MAINNET_RPC_URL");
}

const hardhatNetwork = {};
const networks = {
  hardhat: hardhatNetwork
};

if (shouldEnableMainnetFork()) {
  hardhatNetwork.forking = {
    url: process.env.MAINNET_RPC_URL
  };
}

if (hasUsableRpcUrl("SEPOLIA_RPC_URL")) {
  networks.sepolia = {
    url: process.env.SEPOLIA_RPC_URL,
    accounts: hasUsablePrivateKey("TESTNET_PRIVATE_KEY")
      ? [process.env.TESTNET_PRIVATE_KEY]
      : [],
    chainId: 11155111
  };
}

module.exports = {
  solidity: {
    version: "0.8.24",
    settings: {
      optimizer: {
        enabled: true,
        runs: 200
      }
    }
  },
  networks
};
