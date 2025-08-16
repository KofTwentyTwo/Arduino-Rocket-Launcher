# 🚀 Luke's Rocket Launcher - Release Management Guide

## 📋 Quick Reference

### **Branch Types & Naming**
- `main` - Production-ready code, always deployable
- `develop` - Integration branch for features
- `feature/*` - New features (from `develop`)
- `bugfix/*` - Bug fixes (from `main`)
- `release/*` - Release preparation (from `develop`)
- `hotfix/*` - Urgent fixes (from `main`)

### **Release Commands**
```bash
# Start a new release
./scripts/release.sh 0.1.0 start

# Finish a release (creates tag, merges to main/develop)
./scripts/release.sh 0.1.0 finish

# Create a new branch
./scripts/create-branch.sh feature new-feature-name
```

---

## 🌿 Git Branching Model

### **Main Branches**
- **`main`** - Always contains production-ready code
- **`develop`** - Integration branch for upcoming releases

### **Supporting Branches**
- **`feature/*`** - New features (branch from `develop`, merge back to `develop`)
- **`bugfix/*`** - Bug fixes (branch from `main`, merge back to `main`)
- **`release/*`** - Release preparation (branch from `develop`, merge to both `main` and `develop`)
- **`hotfix/*`** - Critical fixes (branch from `main`, merge to both `main` and `develop`)

---

## 🚀 Release Process

### **1. Start a Release**
```bash
# Create release branch from develop
./scripts/release.sh 0.1.0 start

# This will:
# - Switch to develop branch
# - Create release/0.1.0 branch
# - Push to remote
```

### **2. Prepare the Release**
- Update version numbers in code
- Update CHANGELOG.md
- Update documentation
- Test thoroughly

### **3. Finish the Release**
```bash
# Complete the release process
./scripts/release.sh 0.1.0 finish

# This will:
# - Merge release branch to main
# - Create and push tag v0.1.0
# - Merge release branch to develop
# - Clean up release branch
# - Trigger automated workflow
```

---

## ⚡ Automated Pipeline

### **What Happens Automatically**
1. **Tag Push** triggers GitHub Actions workflow
2. **Workflow Builds** firmware (hardware + simulation)
3. **Workflow Creates** GitHub release
4. **Workflow Uploads** assets to release
5. **Workflow Publishes** to GitHub Packages

### **No Manual Steps Needed**
- ❌ Don't manually create GitHub releases
- ❌ Don't manually upload assets
- ✅ Just run the release script
- ✅ Let the workflow handle everything else

---

## 🔧 Branch Management

### **Create New Branches**
```bash
# Feature branch (from develop)
./scripts/create-branch.sh feature new-feature-name

# Bugfix branch (from main)
./scripts/create-branch.sh bugfix bug-description

# Hotfix branch (from main)
./scripts/create-branch.sh hotfix urgent-fix-description
```

### **Branch Protection Rules**
- **`main`** - Requires PR, no direct pushes
- **`develop`** - Requires PR, no direct pushes
- **Feature branches** - Can be pushed directly

---

## 📝 Pull Request Workflow

### **1. Create PR**
```bash
# From feature branch to develop
gh pr create --base develop --head feature/new-feature

# From develop to main (for releases)
gh pr create --base main --head develop
```

### **2. Review & Merge**
- Self-review is allowed (you're the only person)
- Merge when ready
- Delete feature branch after merge

---

## 🏷️ Version Management

### **Semantic Versioning**
- **Major.Minor.Patch** (e.g., 1.2.3)
- **Major** - Breaking changes
- **Minor** - New features, backward compatible
- **Patch** - Bug fixes, backward compatible

### **Pre-releases**
- Use for development/testing versions
- Example: 0.1.0 (pre-release)
- Mark as pre-release in GitHub

---

## 🚨 Troubleshooting

### **Common Issues**

#### **"already_exists" Error**
- **Cause**: Manual GitHub release conflicts with workflow
- **Solution**: Don't manually create releases, let workflow handle it

#### **Workflow Not Running**
- **Cause**: Branch protection rules
- **Solution**: Create PR and merge to main/develop

#### **Asset Upload Failures**
- **Cause**: Missing upload_url
- **Solution**: Ensure workflow creates release (don't skip creation)

### **Reset Release**
```bash
# Delete local tag
git tag -d v0.1.0

# Delete remote tag
git push origin --delete v0.1.0

# Delete GitHub release manually on GitHub
# Start over with release script
```

---

## 📚 File Structure

```
Lukes-Rocket-Launcher/
├── .github/
│   ├── workflows/c-cpp.yml          # Main CI/CD workflow
│   ├── CODEOWNERS                   # Code ownership rules
│   └── pull_request_template.md     # PR template
├── scripts/
│   ├── release.sh                   # Release management
│   ├── create-branch.sh            # Branch creation
│   ├── setup-branch-protection.sh  # Branch protection setup
│   ├── dev-sim.sh                  # Development simulator launcher
│   └── sim                         # Quick simulator alias
├── code/RocketLauncher/            # Main firmware code
├── wiring/                         # SimulIDE wiring files
└── RELEASE.md                      # This file
```

## 🖥️ Development Environment

### **Local Development Tools**
- **PlatformIO** - Arduino framework and build system
- **SimulIDE** - Arduino simulator for testing
- **CMake** - IDE integration and native testing
- **Unity** - Unit testing framework

### **Quick Development Commands**
```bash
# Launch simulator (builds + opens SimulIDE)
./scripts/sim

# Clean rebuild
./scripts/sim --clean

# Run tests
cd code/RocketLauncher && pio test -e native

# Build hardware firmware
cd code/RocketLauncher && pio run -e uno_hw
```

---

## 🎯 Best Practices

### **Do's**
- ✅ Use semantic versioning
- ✅ Test before releasing
- ✅ Update documentation
- ✅ Let automation handle releases
- ✅ Use descriptive branch names

### **Don'ts**
- ❌ Don't manually create GitHub releases
- ❌ Don't push directly to main/develop
- ❌ Don't skip testing
- ❌ Don't forget to update version numbers

---

## 🔄 Daily Workflow

### **Normal Development**
1. Create feature branch: `./scripts/create-branch.sh feature new-feature`
2. Make changes and commit
3. **Test locally**: `./scripts/sim` (builds and launches simulator)
4. Push and create PR to develop
5. Merge when ready

### **Local Development Cycle**
```bash
# Quick simulator launch (builds + launches SimulIDE)
./scripts/sim

# Force clean rebuild
./scripts/sim --clean

# Run unit tests
cd code/RocketLauncher && pio test -e native

# Build hardware firmware
cd code/RocketLauncher && pio run -e uno_hw
```

### **Release Day**
1. Start release: `./scripts/release.sh 1.0.0 start`
2. Prepare release (version, docs, testing)
3. Finish release: `./scripts/release.sh 1.0.0 finish`
4. Wait for automation to complete
5. Deploy and announce

---

## 📞 Quick Commands Reference

```bash
# Branch Management
./scripts/create-branch.sh feature new-feature
./scripts/create-branch.sh bugfix bug-description
./scripts/create-branch.sh hotfix urgent-fix

# Release Management
./scripts/release.sh 1.0.0 start
./scripts/release.sh 1.0.0 finish

# Git Operations
git checkout develop
git pull origin develop
git checkout main
git pull origin main

# Workflow Monitoring
gh run list --limit 5
gh run view <run-id>
```

---

## 🎉 Success Indicators

### **Release Completed Successfully When:**
- ✅ All workflow jobs show green checkmarks
- ✅ GitHub release exists with assets
- ✅ Firmware files are uploaded
- ✅ No "already_exists" errors
- ✅ Asset uploads complete

### **If Something Goes Wrong:**
1. Check workflow logs: `gh run view <run-id>`
2. Look for error messages
3. Check if release already exists on GitHub
4. Reset and try again if needed

---

*This guide covers the complete release pipeline. When in doubt, run the scripts and let automation handle the complexity! 🚀*
