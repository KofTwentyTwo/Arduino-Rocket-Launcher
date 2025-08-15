# 🏷️ Semantic Versioning Guide

This document explains how to use the semantic versioning system for Luke's Rocket Launcher project.

## 📋 **Overview**

The project now uses **semantic versioning** to automatically manage releases, create GitHub releases, and package firmware files. This ensures consistent versioning and makes it easy for users to get the latest firmware.

## 🔢 **Version Format**

**Format**: `MAJOR.MINOR.PATCH` (e.g., `1.0.0`, `1.2.3`)

- **MAJOR** (1.0.0 → 2.0.0): Breaking changes, incompatible API changes
- **MINOR** (1.0.0 → 1.1.0): New features, backward compatible
- **PATCH** (1.0.0 → 1.0.1): Bug fixes, improvements, backward compatible

## 🚀 **Creating Releases**

### **Option 1: Using the Release Script (Recommended)**

```bash
# From project root directory
./scripts/release.sh --type minor

# Or with custom version
./scripts/release.sh --version 1.0.0

# Dry run to see what would happen
./scripts/release.sh --type patch --dry-run
```

**Script Options:**
- `--type`: `patch`, `minor`, or `major`
- `--version`: Specific version number
- `--message`: Custom release message
- `--dry-run`: Preview without making changes

### **Option 2: Using Make**

```bash
# From code/RocketLauncher directory
make release
```

### **Option 3: Manual Git Tags**

```bash
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```

### **Option 4: GitHub Actions Manual Trigger**

1. Go to [Actions](https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/actions)
2. Click "Semantic Release"
3. Click "Run workflow"
4. Enter version and release type
5. Click "Run workflow"

## 📝 **Commit Message Convention**

Use conventional commit messages to automatically determine version increments:

```bash
# Patch releases (bug fixes, improvements)
fix(arduino): resolve startup transition issue
fix(testing): fix mock interface behavior
docs: update installation instructions

# Minor releases (new features)
feat(arduino): add new safety feature
feat(testing): add comprehensive test suite
feat(build): add automated release workflow

# Major releases (breaking changes)
feat!: completely redesign state machine
BREAKING CHANGE: change pin assignments
```

## 🔄 **Release Workflow**

When you create a release:

1. **Version Determination**: Script calculates next version
2. **Git Tag**: Creates and pushes version tag
3. **GitHub Actions**: Automatically triggers build workflow
4. **Build & Test**: Runs tests and builds firmware
5. **Package Creation**: Creates downloadable firmware files
6. **GitHub Release**: Creates release with assets and notes

## 📦 **Release Assets**

Each release automatically includes:

- **Simulation Firmware**: `.hex` file for SimulIDE
- **Hardware Firmware**: `.hex` file for Arduino
- **Release Notes**: Auto-generated from commits
- **Build Information**: Version, date, commit hash

## 🎯 **Best Practices**

### **When to Release**

- **Patch**: Bug fixes, documentation updates, minor improvements
- **Minor**: New features, enhancements, backward compatible changes
- **Major**: Breaking changes, major redesigns, incompatible updates

### **Release Frequency**

- **Patch**: Weekly or as needed
- **Minor**: Monthly or when features are complete
- **Major**: Quarterly or for significant changes

### **Before Releasing**

1. **Test Everything**: Ensure all tests pass
2. **Update Documentation**: Keep README and docs current
3. **Check Dependencies**: Verify library versions are stable
4. **Clean Working Directory**: Commit or stash all changes

## 🛠️ **Troubleshooting**

### **Common Issues**

**Tag Already Exists:**
```bash
# Check existing tags
git tag -l

# Delete local tag if needed
git tag -d v1.0.0

# Delete remote tag if needed
git push origin --delete v1.0.0
```

**Workflow Fails:**
1. Check [Actions](https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/actions) for errors
2. Ensure tests pass locally: `make test`
3. Verify build works: `make build`

**Version Calculation Issues:**
```bash
# Check current version
git describe --tags --abbrev=0

# See commit history
git log --oneline --decorate
```

## 📚 **Examples**

### **Example 1: Bug Fix Release**

```bash
# Fix a bug
git commit -m "fix(arduino): resolve button debouncing issue"

# Create patch release
./scripts/release.sh --type patch --message "Bug fix release"
```

### **Example 2: Feature Release**

```bash
# Add new feature
git commit -m "feat(arduino): add current sensing circuit"

# Create minor release
./scripts/release.sh --type minor --message "Add current sensing feature"
```

### **Example 3: Major Update**

```bash
# Breaking change
git commit -m "feat!: redesign state machine architecture"

# Create major release
./scripts/release.sh --type major --message "Complete state machine redesign"
```

## 🔗 **Related Links**

- [GitHub Actions](https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/actions)
- [Releases](https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/releases)
- [Conventional Commits](https://www.conventionalcommits.org/)
- [Semantic Versioning](https://semver.org/)

---

**🚀 Ready to release? Use `./scripts/release.sh --help` for more options!**
