# Build rules for the Part 1 C++ arbitrage detector and its unit tests.

CXX ?= g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra -pedantic
SRC_DIR := src
BIN_DIR := bin
TEST_DIR := tests

SOURCES := \
	$(SRC_DIR)/app/cli.cpp \
	$(SRC_DIR)/app/detector.cpp \
	$(SRC_DIR)/domain/graph/cycle_enumerator.cpp \
	$(SRC_DIR)/domain/graph/market_graph.cpp \
	$(SRC_DIR)/domain/opportunity/cycle_evaluator.cpp \
	$(SRC_DIR)/domain/opportunity/opportunity_ranker.cpp \
	$(SRC_DIR)/domain/opportunity/trade_size_optimizer.cpp \
	$(SRC_DIR)/domain/pricing/price_estimator.cpp \
	$(SRC_DIR)/io/pool_snapshot_loader.cpp \
	$(SRC_DIR)/io/result_writer.cpp \
	$(SRC_DIR)/presentation/console_reporter.cpp \
	$(SRC_DIR)/util/number_utils.cpp \
	$(SRC_DIR)/util/token_registry.cpp

APP := $(BIN_DIR)/arb_detect
TEST_APP := $(BIN_DIR)/arb_tests

.PHONY: all clean test

all: $(APP)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(APP): $(BIN_DIR) $(SOURCES) $(SRC_DIR)/main.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -Ithird_party $(SOURCES) $(SRC_DIR)/main.cpp -o $(APP)

$(TEST_APP): $(BIN_DIR) $(SOURCES) $(TEST_DIR)/test_main.cpp
	$(CXX) $(CXXFLAGS) -I$(SRC_DIR) -Ithird_party $(SOURCES) $(TEST_DIR)/test_main.cpp -o $(TEST_APP)

test: $(TEST_APP)
	./$(TEST_APP)

clean:
	rm -rf $(BIN_DIR)
