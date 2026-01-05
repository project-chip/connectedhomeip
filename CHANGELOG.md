# Changelog

All notable changes to this repository will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Initial Commit] - 2025-11-02

### Added

This represents the initial setup of the Matter SDK (formerly Project CHIP) repository fork.

#### Repository Structure
- **src/**: Core Matter SDK source code including protocol implementation, platform abstractions, and cluster implementations
- **examples/**: Example applications demonstrating Matter device implementations across various platforms
- **scripts/**: Build scripts, testing utilities, and development tools
- **docs/**: Comprehensive documentation including guides, API references, and development workflows
- **build/**: Build system configuration and tooling
- **config/**: Platform-specific configuration files
- **data_model/**: Matter Data Model XML definitions for clusters and device types
- **third_party/**: Third-party dependencies and libraries
- **credentials/**: Test credentials and certification materials
- **integrations/**: Integration code for various ecosystems

#### Key Features
- Matter 1.2.0 specification implementation
- Support for multiple platforms including:
  - Linux (ARM and x86)
  - Darwin (macOS/iOS)
  - ESP32
  - Nordic nRF Connect SDK
  - Silicon Labs EFR32
  - NXP platforms
  - Texas Instruments CC13XX/CC26XX
  - Realtek
  - Ameba
  - Telink
  - Tizen
  - Android
  - And many more

#### Development Infrastructure
- GitHub Actions CI/CD workflows for automated building and testing
- Code quality tools (clang-format, clang-tidy, spell check)
- Development container configuration
- Pre-commit hooks for code validation
- Comprehensive test suite
- Documentation build system using Sphinx and Doxygen

#### GitHub Configuration
- Issue templates for bug reports, feature requests, and various issue types
- Pull request template
- Dependabot configuration for dependency updates
- Code owners and reviewers configuration
- GitHub Actions for building examples, running tests, and code analysis

### Reference
- Commit: 7cee904ce25de3cda41688c685b1c7e10f55ddd4
- Date: November 2, 2025
- Total files: 22,115
- Matter Specification Version: 1.2.0

### Additional Resources
- [Matter SDK Documentation](https://project-chip.github.io/connectedhomeip-doc/index.html)
- [Build with Matter](https://buildwithmatter.com)
- [CSA Matter Specifications](https://csa-iot.org/developer-resource/specifications-download-request/)
