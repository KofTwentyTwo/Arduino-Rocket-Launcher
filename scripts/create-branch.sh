#!/bin/bash

# Create Branch Script for Git Branching Model
# Usage: ./create-branch.sh [type] [name]

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Branch types and their source branches
declare -A BRANCH_TYPES=(
    ["feature"]="develop"
    ["bugfix"]="main"
    ["release"]="develop"
    ["hotfix"]="main"
)

# Function to show usage
show_usage() {
    echo -e "${BLUE}🚀 Create Branch Script for Git Branching Model${NC}"
    echo "=================================================="
    echo ""
    echo "Usage: $0 [type] [name]"
    echo ""
    echo "Branch Types:"
    echo "  feature  - New features (from develop)"
    echo "  bugfix   - Bug fixes (from main)"
    echo "  release  - Release preparation (from develop)"
    echo "  hotfix   - Urgent fixes (from main)"
    echo ""
    echo "Examples:"
    echo "  $0 feature arming-siren"
    echo "  $0 bugfix serial-monitor-timeout"
    echo "  $0 release 1.2.0"
    echo "  $0 hotfix 1.2.1-watchdog-crash"
    echo ""
    echo "Branch Naming Convention:"
    echo "  feature/arming-siren"
    echo "  bugfix/serial-monitor-timeout"
    echo "  release/1.2.0"
    echo "  hotfix/1.2.1-watchdog-crash"
}

# Check if we have the right number of arguments
if [ $# -ne 2 ]; then
    show_usage
    exit 1
fi

BRANCH_TYPE="$1"
BRANCH_NAME="$2"

# Validate branch type
if [[ ! ${BRANCH_TYPES[$BRANCH_TYPE]} ]]; then
    echo -e "${RED}❌ Invalid branch type: $BRANCH_TYPE${NC}"
    echo "Valid types: ${!BRANCH_TYPES[@]}"
    exit 1
fi

# Get source branch
SOURCE_BRANCH="${BRANCH_TYPES[$BRANCH_TYPE]}"

# Create full branch name
FULL_BRANCH_NAME="$BRANCH_TYPE/$BRANCH_NAME"

echo -e "${BLUE}🚀 Creating new branch: $FULL_BRANCH_NAME${NC}"
echo "=================================================="
echo ""

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo -e "${RED}❌ Not in a git repository${NC}"
    exit 1
fi

# Check if source branch exists locally
if ! git show-ref --verify --quiet refs/heads/$SOURCE_BRANCH; then
    echo -e "${YELLOW}⚠️  Source branch '$SOURCE_BRANCH' doesn't exist locally${NC}"
    echo "Fetching from remote..."
    git fetch origin $SOURCE_BRANCH:$SOURCE_BRANCH
fi

# Check if source branch exists remotely
if ! git show-ref --verify --quiet refs/remotes/origin/$SOURCE_BRANCH; then
    echo -e "${RED}❌ Source branch '$SOURCE_BRANCH' doesn't exist remotely${NC}"
    echo "Please ensure the source branch exists on GitHub first."
    exit 1
fi

# Check if branch already exists
if git show-ref --verify --quiet refs/heads/$FULL_BRANCH_NAME; then
    echo -e "${RED}❌ Branch '$FULL_BRANCH_NAME' already exists locally${NC}"
    exit 1
fi

if git show-ref --verify --quiet refs/remotes/origin/$FULL_BRANCH_NAME; then
    echo -e "${RED}❌ Branch '$FULL_BRANCH_NAME' already exists remotely${NC}"
    exit 1
fi

# Create and switch to new branch
echo -e "${BLUE}📋 Creating branch from $SOURCE_BRANCH...${NC}"
git checkout $SOURCE_BRANCH

echo -e "${BLUE}🔄 Updating $SOURCE_BRANCH from remote...${NC}"
git pull origin $SOURCE_BRANCH

echo -e "${BLUE}🌿 Creating new branch: $FULL_BRANCH_NAME${NC}"
git checkout -b $FULL_BRANCH_NAME

echo -e "${BLUE}📤 Pushing branch to remote...${NC}"
git push -u origin $FULL_BRANCH_NAME

echo ""
echo -e "${GREEN}✅ Successfully created branch: $FULL_BRANCH_NAME${NC}"
echo ""
echo -e "${BLUE}📚 Next steps:${NC}"
echo "   1. Make your changes"
echo "   2. Commit and push: git add . && git commit -m 'your message' && git push"
echo "   3. Create a Pull Request to $SOURCE_BRANCH"
echo ""
echo -e "${BLUE}🔄 Branch flow:${NC}"
echo "   $FULL_BRANCH_NAME → $SOURCE_BRANCH → main (via PR)"
echo ""
echo -e "${YELLOW}💡 Tip: Use conventional commit messages:${NC}"
echo "   feat: add arming siren functionality"
echo "   fix: resolve serial monitor timeout issue"
echo "   docs: update README with new features"
