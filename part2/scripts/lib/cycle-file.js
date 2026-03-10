/**
 * Helpers for locating and parsing the Part 1 JSON output that feeds the
 * on-chain validator replay scripts.
 */
const fs = require("fs");
const path = require("path");

/**
 * Resolves the cycle JSON path, falling back to the default generated result.
 */
function resolveCyclesPath(customPath) {
  return (
    customPath ||
    path.resolve(__dirname, "../../../part1/results/top10_cycles.json")
  );
}

/**
 * Loads and validates the presence of ranked opportunities in the JSON file.
 */
function loadCyclesFile(cyclesPath) {
  const resolvedPath = resolveCyclesPath(cyclesPath);
  const raw = fs.readFileSync(resolvedPath, "utf8");
  const parsed = JSON.parse(raw);

  if (!parsed.opportunities || parsed.opportunities.length === 0) {
    throw new Error(`No opportunities found in ${resolvedPath}`);
  }

  return { resolvedPath, parsed };
}

/**
 * Selects a contiguous slice of ranked opportunities for replay.
 */
function selectCycles(opportunities, cycleIndex, maxCycles) {
  if (cycleIndex < 0 || cycleIndex >= opportunities.length || maxCycles <= 0) {
    throw new Error(`CYCLE_INDEX ${cycleIndex} is out of bounds`);
  }

  return opportunities.slice(cycleIndex, cycleIndex + maxCycles);
}

module.exports = {
  loadCyclesFile,
  resolveCyclesPath,
  selectCycles
};
