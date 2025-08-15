# 🚀 Luke's Rocket Launch Controller - Makefile
# Simple build system for Arduino-based rocket launcher

# Project Configuration
PROJECT_NAME = RocketLauncher
BUILD_DIR = .pio
OUT_DIR = out

# PlatformIO Configuration
PIO_CMD = pio
PIO_ENV_SIM = simulide
PIO_ENV_HW = uno_hw

# Default target
.DEFAULT_GOAL := all

# Colors for pretty output
GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
BLUE = \033[0;34m
NC = \033[0m # No Color

# Help target
.PHONY: help
help: ## Show this help message
	@echo "$(GREEN)🚀 Luke's Rocket Launch Controller - Build System$(NC)"
	@echo ""
	@echo "$(BLUE)Available targets:$(NC)"
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  $(YELLOW)%-15s$(NC) %s\n", $$1, $$2}' $(MAKEFILE_LIST)
	@echo ""
	@echo "$(BLUE)Quick start:$(NC)"
	@echo "  make              # Run tests and build everything (default)"
	@echo "  make build-only   # Build everything (skip tests)"
	@echo "  make test         # Run unit tests"
	@echo "  make build-sim    # Build for simulation"
	@echo "  make build-hw     # Build for hardware"
	@echo "  make clean        # Clean build files"

# Build targets
.PHONY: all
all: test build ## Run tests and build for both simulation and hardware

.PHONY: build-only
build-only: build ## Build for both simulation and hardware (skip tests)

.PHONY: build-sim
build-sim: ## Build project for SimulIDE simulation
	@echo "$(GREEN)🔧 Building for simulation...$(NC)"
	@$(PIO_CMD) run -e $(PIO_ENV_SIM)
	@echo "$(GREEN)✅ Simulation build complete!$(NC)"
	@echo "$(BLUE)   Firmware: $(OUT_DIR)/simulide/firmware.hex$(NC)"

.PHONY: build-hw
build-hw: ## Build project for Arduino hardware
	@echo "$(GREEN)🔧 Building for hardware...$(NC)"
	@$(PIO_CMD) run -e $(PIO_ENV_HW)
	@echo "$(GREEN)✅ Hardware build complete!$(NC)"
	@echo "$(BLUE)   Firmware: $(BUILD_DIR)/$(PIO_ENV_HW)/firmware.hex$(NC)"

.PHONY: build
build: build-sim build-hw ## Build for both simulation and hardware

# Test targets
.PHONY: test
test: ## Run unit tests
	@echo "$(GREEN)🧪 Running unit tests...$(NC)"
	@$(PIO_CMD) test -e native
	@echo "$(GREEN)✅ Tests completed!$(NC)"

.PHONY: test-verbose
test-verbose: ## Run tests with verbose output
	@echo "$(GREEN)🧪 Running verbose tests...$(NC)"
	@$(PIO_CMD) test --verbose
	@echo "$(GREEN)✅ Verbose tests completed!$(NC)"

# Cleanup targets
.PHONY: clean
clean: ## Clean build artifacts
	@echo "$(YELLOW)🧹 Cleaning build files...$(NC)"
	@$(PIO_CMD) run -t clean
	@rm -rf $(OUT_DIR)
	@echo "$(GREEN)✅ Clean complete!$(NC)"

.PHONY: clean-all
clean-all: ## Clean everything including dependencies
	@echo "$(YELLOW)🧹 Deep cleaning...$(NC)"
	@$(PIO_CMD) run -t clean
	@rm -rf $(BUILD_DIR)
	@rm -rf $(OUT_DIR)
	@echo "$(GREEN)✅ Deep clean complete!$(NC)"

# Upload targets
.PHONY: upload
upload: build-hw ## Upload firmware to Arduino
	@echo "$(GREEN)🚀 Uploading to Arduino...$(NC)"
	@$(PIO_CMD) run -e $(PIO_ENV_HW) -t upload
	@echo "$(GREEN)✅ Upload complete!$(NC)"

.PHONY: upload-sim
upload-sim: build-sim ## Build and prepare for SimulIDE
	@echo "$(GREEN)🎮 Preparing for SimulIDE simulation...$(NC)"
	@echo "$(BLUE)   Open wiring/rocker_launcher_controls.sim1 in SimulIDE$(NC)"
	@echo "$(BLUE)   Firmware will auto-load from: $(OUT_DIR)/simulide/firmware.hex$(NC)"

# Development targets
.PHONY: monitor
monitor: ## Monitor Arduino serial output
	@echo "$(GREEN)📡 Starting serial monitor...$(NC)"
	@$(PIO_CMD) device monitor -b 115200

.PHONY: format
format: ## Format source code with clang-format
	@echo "$(GREEN)🎨 Formatting source code...$(NC)"
	@if command -v clang-format >/dev/null 2>&1; then \
		clang-format -i src/**/*.cpp src/**/*.h lib/**/*.cpp lib/**/*.h 2>/dev/null || true; \
		echo "$(GREEN)✅ Code formatting complete!$(NC)"; \
	else \
		echo "$(YELLOW)⚠️  clang-format not found. Install clang-format to format code.$(NC)"; \
		echo "$(BLUE)   macOS: brew install clang-format$(NC)"; \
		echo "$(BLUE)   Ubuntu: sudo apt install clang-format$(NC)"; \
	fi

# Status
.PHONY: status
status: ## Show project status
	@echo "$(GREEN)📊 Project Status$(NC)"
	@echo "$(BLUE)   Project: $(PROJECT_NAME)$(NC)"
	@echo "$(BLUE)   Build dir: $(BUILD_DIR)$(NC)"
	@echo "$(BLUE)   Output dir: $(OUT_DIR)$(NC)"
	@if [ -d "$(BUILD_DIR)" ]; then echo "$(GREEN)   ✅ Build directory exists$(NC)"; else echo "$(RED)   ❌ Build directory missing$(NC)"; fi
	@if [ -d "$(OUT_DIR)" ]; then echo "$(GREEN)   ✅ Output directory exists$(NC)"; else echo "$(RED)   ❌ Output directory missing$(NC)"; fi
