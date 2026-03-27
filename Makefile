# Matter SDK - Project Makefile
#
# Provides convenience targets for common development tasks.
# Before using most targets, initialize the environment with:
#   source scripts/activate.sh

.PHONY: help setup bootstrap activate build test clean lint

# Default target
help:
	@echo "Matter SDK - Available targets:"
	@echo ""
	@echo "  setup      - Bootstrap the development environment (first-time setup)"
	@echo "  activate   - Print instructions for activating the environment"
	@echo "  build      - Build the default Linux example"
	@echo "  test       - Run unit tests"
	@echo "  lint       - Run linters (Python and shell)"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Before building, activate the environment:"
	@echo "  source scripts/activate.sh"

setup:
	@echo "Bootstrapping development environment..."
	bash scripts/bootstrap.sh
	@echo ""
	@echo "Bootstrap complete. To activate the environment, run:"
	@echo "  source scripts/activate.sh"

activate:
	@echo "To activate the Matter development environment, run:"
	@echo "  source scripts/activate.sh"

build:
	@echo "Building Linux standalone example..."
	./scripts/examples/gn_build_example.sh examples/all-clusters-app/linux out/all-clusters

test:
	@echo "Running unit tests..."
	./scripts/tests/gn_tests.sh

lint:
	@echo "Running Python linter..."
	python3 -m ruff check scripts src
	@echo "Running shell linter..."
	bash scripts/run_in_build_env.sh "shellcheck --shell=bash scripts/*.sh"

clean:
	@echo "Removing build artifacts..."
	rm -rf out/
