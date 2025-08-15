#!/bin/bash

# Setup Branch Protection Rules for Git Branching Model
# Run this script to configure branch protection on GitHub

echo "🚀 Setting up Branch Protection Rules for Git Branching Model"
echo "=============================================================="
echo ""

# Check if gh CLI is installed
if ! command -v gh &> /dev/null; then
    echo "❌ GitHub CLI (gh) is not installed. Please install it first:"
    echo "   https://cli.github.com/"
    exit 1
fi

# Check if authenticated
if ! gh auth status &> /dev/null; then
    echo "❌ Not authenticated with GitHub. Please run: gh auth login"
    exit 1
fi

REPO="KofTwentyTwo/Lukes-Rocket-Launcher"

echo "📋 Setting up branch protection for: $REPO"
echo ""

# Main branch protection
echo "🔒 Setting up MAIN branch protection..."
gh api repos/$REPO/branches/main/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"contexts":["Build and Test Arduino Firmware"]}' \
  --field enforce_admins=true \
  --field required_pull_request_reviews='{"required_approving_review_count":1,"dismiss_stale_reviews":true,"require_code_owner_reviews":false}' \
  --field restrictions=null

if [ $? -eq 0 ]; then
    echo "✅ MAIN branch protection configured"
else
    echo "❌ Failed to configure MAIN branch protection"
fi

echo ""

# Develop branch protection (optional, but recommended)
echo "🔒 Setting up DEVELOP branch protection..."
gh api repos/$REPO/branches/develop/protection \
  --method PUT \
  --field required_status_checks='{"strict":true,"contexts":["Build and Test Arduino Firmware"]}' \
  --field enforce_admins=false \
  --field required_pull_request_reviews='{"required_approving_review_count":1,"dismiss_stale_reviews":true,"require_code_owner_reviews":false}' \
  --field restrictions=null

if [ $? -eq 0 ]; then
    echo "✅ DEVELOP branch protection configured"
else
    echo "❌ Failed to configure DEVELOP branch protection"
fi

echo ""
echo "🎯 Branch Protection Setup Complete!"
echo ""
echo "📚 Next steps:"
echo "   1. Create feature branches from develop: git checkout -b feature/your-feature develop"
echo "   2. Create bugfix branches from main: git checkout -b bugfix/your-bug main"
echo "   3. Create release branches from develop: git checkout -b release/1.0.0 develop"
echo "   4. Create hotfix branches from main: git checkout -b hotfix/1.0.1 main"
echo ""
echo "🔄 Workflow:"
echo "   Feature → Develop → Main (via PR)"
echo "   Bugfix → Main (via PR)"
echo "   Release → Develop → Main (via PR)"
echo "   Hotfix → Main (via PR)"
