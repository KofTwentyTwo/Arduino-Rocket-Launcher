# 🚀 Luke's Rocket Launch Controller - Makefile
# A comprehensive build system for the Arduino-based rocket launcher

# Project Configuration
PROJECT_NAME = RocketLauncher
PROJECT_DIR = code/$(PROJECT_NAME)
BUILD_DIR = $(PROJECT_DIR)/.pio
OUT_DIR = $(PROJECT_DIR)/out
SIM_DIR = wiring

# PlatformIO Configuration
PIO_CMD = pio
PIO_ENV_SIM = simulide
PIO_ENV_HW = uno_hw

# Default target
.DEFAULT_GOAL := help

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
	@echo "  make build-sim    # Build for simulation"
	@echo "  make build-hw     # Build for hardware"
	@echo "  make upload       # Upload to Arduino"
	@echo "  make clean        # Clean build files"
	@echo "  make test         # Run tests"

# Build targets
.PHONY: build-sim
build-sim: ## Build project for SimulIDE simulation
	@echo "$(GREEN)🔧 Building for simulation...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) run -e $(PIO_ENV_SIM)
	@echo "$(GREEN)✅ Simulation build complete!$(NC)"
	@echo "$(BLUE)   Firmware: $(OUT_DIR)/simulide/firmware.hex$(NC)"

.PHONY: build-hw
build-hw: ## Build project for Arduino hardware
	@echo "$(GREEN)🔧 Building for hardware...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) run -e $(PIO_ENV_HW)
	@echo "$(GREEN)✅ Hardware build complete!$(NC)"
	@echo "$(BLUE)   Firmware: $(BUILD_DIR)/$(PIO_ENV_HW)/firmware.hex$(NC)"

.PHONY: build
build: build-sim build-hw ## Build for both simulation and hardware

# Upload targets
.PHONY: upload
upload: build-hw ## Upload firmware to Arduino
	@echo "$(GREEN)🚀 Uploading to Arduino...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) run -e $(PIO_ENV_HW) -t upload
	@echo "$(GREEN)✅ Upload complete!$(NC)"

.PHONY: upload-sim
upload-sim: build-sim ## Build and prepare for SimulIDE
	@echo "$(GREEN)🎮 Preparing for SimulIDE simulation...$(NC)"
	@echo "$(BLUE)   Open $(SIM_DIR)/rocker_launcher_controls.sim1 in SimulIDE$(NC)"
	@echo "$(BLUE)   Firmware will auto-load from: $(OUT_DIR)/simulide/firmware.hex$(NC)"

# Development targets
.PHONY: monitor
monitor: ## Monitor Arduino serial output
	@echo "$(GREEN)📡 Starting serial monitor...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) device monitor

.PHONY: test
test: ## Run project tests
	@echo "$(GREEN)🧪 Running tests...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) test

.PHONY: check
check: ## Check code formatting and style
	@echo "$(GREEN)🔍 Checking code style...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) check

# Cleanup targets
.PHONY: clean
clean: ## Clean build artifacts
	@echo "$(YELLOW)🧹 Cleaning build files...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) run -t clean
	@rm -rf $(OUT_DIR)
	@echo "$(GREEN)✅ Clean complete!$(NC)"

.PHONY: clean-all
clean-all: ## Clean everything including dependencies
	@echo "$(YELLOW)🧹 Deep cleaning...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) run -t clean
	@rm -rf $(BUILD_DIR)
	@rm -rf $(OUT_DIR)
	@echo "$(GREEN)✅ Deep clean complete!$(NC)"

# Dependencies
.PHONY: install-deps
install-deps: ## Install project dependencies
	@echo "$(GREEN)📦 Installing dependencies...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) lib install "arduino-libraries/LiquidCrystal"
	@cd $(PROJECT_DIR) && $(PIO_CMD) lib install "thomasfredericks/Bounce2"
	@echo "$(GREEN)✅ Dependencies installed!$(NC)"

.PHONY: update-deps
update-deps: ## Update project dependencies
	@echo "$(GREEN)🔄 Updating dependencies...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) lib update
	@echo "$(GREEN)✅ Dependencies updated!$(NC)"

# Project management
.PHONY: init
init: install-deps ## Initialize project (install dependencies)
	@echo "$(GREEN)🎯 Project initialized!$(NC)"
	@echo "$(BLUE)   Next steps:$(NC)"
	@echo "$(BLUE)   1. make build-sim    # Build for simulation$(NC)"
	@echo "$(BLUE)   2. make build-hw     # Build for hardware$(NC)"
	@echo "$(BLUE)   3. make upload       # Upload to Arduino$(NC)"

.PHONY: status
status: ## Show project status
	@echo "$(GREEN)📊 Project Status$(NC)"
	@echo "$(BLUE)   Project: $(PROJECT_NAME)$(NC)"
	@echo "$(BLUE)   Directory: $(PROJECT_DIR)$(NC)"
	@echo "$(BLUE)   Build dir: $(BUILD_DIR)$(NC)"
	@echo "$(BLUE)   Output dir: $(OUT_DIR)$(NC)"
	@echo "$(BLUE)   Simulation: $(SIM_DIR)$(NC)"
	@if [ -d "$(BUILD_DIR)" ]; then echo "$(GREEN)   ✅ Build directory exists$(NC)"; else echo "$(RED)   ❌ Build directory missing$(NC)"; fi
	@if [ -d "$(OUT_DIR)" ]; then echo "$(GREEN)   ✅ Output directory exists$(NC)"; else echo "$(RED)   ❌ Output directory missing$(NC)"; fi

# Simulation helpers
.PHONY: open-sim
open-sim: ## Open SimulIDE simulation (macOS)
	@echo "$(GREEN)🎮 Opening SimulIDE...$(NC)"
	@open -a SimulIDE "$(SIM_DIR)/rocker_launcher_controls.sim1"

.PHONY: convert-video
convert-video: ## Convert MP4 to GIF for GitHub (requires ffmpeg)
	@echo "$(GREEN)🎬 Converting video for GitHub...$(NC)"
	@if command -v ffmpeg >/dev/null 2>&1; then \
		ffmpeg -i docs/clip-1.mp4 -vf "fps=10,scale=480:-1" docs/clip-1.gif; \
		echo "$(GREEN)✅ Video converted to docs/clip-1.gif$(NC)"; \
	else \
		echo "$(RED)❌ ffmpeg not found. Install ffmpeg to convert videos.$(NC)"; \
		echo "$(BLUE)   macOS: brew install ffmpeg$(NC)"; \
		echo "$(BLUE)   Ubuntu: sudo apt install ffmpeg$(NC)"; \
	fi

# Documentation
.PHONY: docs
docs: ## Generate documentation
	@echo "$(GREEN)📚 Generating documentation...$(NC)"
	@echo "$(BLUE)   README.md is the main documentation$(NC)"
	@echo "$(BLUE)   Circuit diagrams are in $(SIM_DIR)/$(NC)"
	@echo "$(BLUE)   Source code is in $(PROJECT_DIR)/src/$(NC)"

# Development workflow
.PHONY: dev
dev: ## Development workflow: build, test, and check
	@echo "$(GREEN)🔄 Running development workflow...$(NC)"
	@make build
	@make test
	@make check
	@echo "$(GREEN)✅ Development workflow complete!$(NC)"

.PHONY: release
release: build-hw ## Prepare release build
	@echo "$(GREEN)🚀 Preparing release...$(NC)"
	@echo "$(BLUE)   Release firmware: $(BUILD_DIR)/$(PIO_ENV_HW)/firmware.hex$(NC)"
	@echo "$(BLUE)   Build timestamp: $(shell date)$(NC)"
	@echo "$(GREEN)✅ Release ready!$(NC)"

# Utility targets
.PHONY: size
size: build-hw ## Show firmware size
	@echo "$(GREEN)📏 Firmware size analysis...$(NC)"
	@cd $(PROJECT_DIR) && $(PIO_CMD) run -e $(PIO_ENV_HW) -t size

.PHONY: list-targets
list-targets: ## List all available targets
	@echo "$(GREEN)📋 Available targets:$(NC)"
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "  %-20s %s\n", $$1, $$2}' $(MAKEFILE_LIST)

# Error handling
.PHONY: check-pio
check-pio: ## Check if PlatformIO is installed
	@if ! command -v $(PIO_CMD) >/dev/null 2>&1; then \
		echo "$(RED)❌ PlatformIO not found!$(NC)"; \
		echo "$(BLUE)   Install with: pip install platformio$(NC)"; \
		echo "$(BLUE)   Or visit: https://platformio.org/install$(NC)"; \
		exit 1; \
	fi

# Main targets with dependencies
build-sim build-hw: check-pio
upload: check-pio
test: check-pio
check: check-pio
