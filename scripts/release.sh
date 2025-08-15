#!/bin/bash

# Release Management Script for Git Branching Model
# Usage: ./release.sh [version] [type]

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to show usage
show_usage() {
    echo -e "${BLUE}🚀 Release Management Script for Git Branching Model${NC}"
    echo "========================================================"
    echo ""
    echo "Usage: $0 [version] [type]"
    echo ""
    echo "Types:"
    echo "  start   - Start a new release (creates release branch)"
    echo "  finish  - Finish a release (merges to main and develop, creates tag)"
    echo "  hotfix  - Create a hotfix release (from main)"
    echo ""
    echo "Examples:"
    echo "  $0 1.2.0 start     # Start release 1.2.0"
    echo "  $0 1.2.0 finish    # Finish release 1.2.0"
    echo "  $0 1.2.1 hotfix    # Create hotfix 1.2.1"
    echo ""
    echo "Release Flow:"
    echo "  1. ./release.sh 1.2.0 start    # Creates release/1.2.0 from develop"
    echo "  2. Make release changes (version, docs, etc.)"
    echo "  3. ./release.sh 1.2.0 finish   # Merges to main and develop, creates tag"
}

# Function to validate version format
validate_version() {
    local version=$1
    if [[ ! $version =~ ^[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
        echo -e "${RED}❌ Invalid version format: $version${NC}"
        echo "Version must be in format: X.Y.Z (e.g., 1.2.0)"
        exit 1
    fi
}

# Function to check if we're in a git repository
check_git_repo() {
    if ! git rev-parse --git-dir > /dev/null 2>&1; then
        echo -e "${RED}❌ Not in a git repository${NC}"
        exit 1
    fi
}

# Function to check if branch exists
branch_exists() {
    local branch=$1
    git show-ref --verify --quiet refs/heads/$branch || \
    git show-ref --verify --quiet refs/remotes/origin/$branch
}

# Function to start a release
start_release() {
    local version=$1
    local release_branch="release/$version"
    
    echo -e "${BLUE}🚀 Starting release $version${NC}"
    echo "=================================="
    
    # Check if release branch already exists
    if branch_exists $release_branch; then
        echo -e "${RED}❌ Release branch $release_branch already exists${NC}"
        exit 1
    fi
    
    # Ensure we're on develop and it's up to date
    if [[ $(git branch --show-current) != "develop" ]]; then
        echo -e "${BLUE}📋 Switching to develop branch...${NC}"
        git checkout develop
    fi
    
    echo -e "${BLUE}🔄 Updating develop from remote...${NC}"
    git pull origin develop
    
    echo -e "${BLUE}🌿 Creating release branch: $release_branch${NC}"
    git checkout -b $release_branch
    
    echo -e "${BLUE}📤 Pushing release branch to remote...${NC}"
    git push -u origin $release_branch
    
    echo ""
    echo -e "${GREEN}✅ Release $version started successfully!${NC}"
    echo ""
    echo -e "${BLUE}📚 Next steps:${NC}"
    echo "   1. Update version numbers in code"
    echo "   2. Update CHANGELOG.md"
    echo "   3. Update documentation"
    echo "   4. Test thoroughly"
    echo "   5. Run: $0 $version finish"
}

# Function to finish a release
finish_release() {
    local version=$1
    local release_branch="release/$version"
    local tag="v$version"
    
    echo -e "${BLUE}🎯 Finishing release $version${NC}"
    echo "=================================="
    
    # Check if we're on the release branch
    if [[ $(git branch --show-current) != $release_branch ]]; then
        echo -e "${BLUE}📋 Switching to release branch...${NC}"
        git checkout $release_branch
    fi
    
    # Ensure release branch is up to date
    echo -e "${BLUE}🔄 Updating release branch from remote...${NC}"
    git pull origin $release_branch
    
    # Check if there are uncommitted changes
    if ! git diff-index --quiet HEAD --; then
        echo -e "${YELLOW}⚠️  You have uncommitted changes. Please commit or stash them first.${NC}"
        git status --short
        exit 1
    fi
    
    echo -e "${BLUE}🔍 Checking if tag already exists...${NC}"
    if git tag -l | grep -q "^$tag$"; then
        echo -e "${RED}❌ Tag $tag already exists${NC}"
        exit 1
    fi
    
    echo -e "${BLUE}📋 Switching to main branch...${NC}"
    git checkout main
    git pull origin main
    
    echo -e "${BLUE}🔄 Merging release branch to main...${NC}"
    git merge --no-ff $release_branch -m "Release $version"
    
    echo -e "${BLUE}🏷️  Creating tag $tag...${NC}"
    git tag -a $tag -m "Release $version"
    
    echo -e "${BLUE}📤 Pushing main and tag to remote...${NC}"
    git push origin main
    git push origin $tag
    
    echo -e "${BLUE}📋 Switching to develop branch...${NC}"
    git checkout develop
    git pull origin develop
    
    echo -e "${BLUE}🔄 Merging release branch to develop...${NC}"
    git merge --no-ff $release_branch -m "Merge release $version into develop"
    
    echo -e "${BLUE}📤 Pushing develop to remote...${NC}"
    git push origin develop
    
    echo -e "${BLUE}🧹 Cleaning up release branch...${NC}"
    git branch -d $release_branch
    git push origin --delete $release_branch
    
    echo ""
    echo -e "${GREEN}✅ Release $version completed successfully!${NC}"
    echo ""
    echo -e "${BLUE}🎉 What was accomplished:${NC}"
    echo "   ✅ Release branch merged to main"
    echo "   ✅ Tag $tag created and pushed"
    echo "   ✅ Release branch merged to develop"
    echo "   ✅ Release branch cleaned up"
    echo ""
    echo -e "${BLUE}📚 Next steps:${NC}"
    echo "   1. Create GitHub Release from tag $tag"
    echo "   2. Deploy to production"
    echo "   3. Announce the release"
}

# Function to create a hotfix
create_hotfix() {
    local version=$1
    local hotfix_branch="hotfix/$version"
    local tag="v$version"
    
    echo -e "${BLUE}🚨 Creating hotfix $version${NC}"
    echo "================================"
    
    # Check if hotfix branch already exists
    if branch_exists $hotfix_branch; then
        echo -e "${RED}❌ Hotfix branch $hotfix_branch already exists${NC}"
        exit 1
    fi
    
    # Ensure we're on main and it's up to date
    if [[ $(git branch --show-current) != "main" ]]; then
        echo -e "${BLUE}📋 Switching to main branch...${NC}"
        git checkout main
    fi
    
    echo -e "${BLUE}🔄 Updating main from remote...${NC}"
    git pull origin main
    
    echo -e "${BLUE}🌿 Creating hotfix branch: $hotfix_branch${NC}"
    git checkout -b $hotfix_branch
    
    echo -e "${BLUE}📤 Pushing hotfix branch to remote...${NC}"
    git push -u origin $hotfix_branch
    
    echo ""
    echo -e "${GREEN}✅ Hotfix $version created successfully!${NC}"
    echo ""
    echo -e "${BLUE}📚 Next steps:${NC}"
    echo "   1. Fix the urgent issue"
    echo "   2. Test thoroughly"
    echo "   3. Commit and push changes"
    echo "   4. Create PR to main"
    echo "   5. After merge, run: $0 $version finish"
}

# Main script logic
if [ $# -ne 2 ]; then
    show_usage
    exit 1
fi

VERSION=$1
TYPE=$2

# Validate version format
validate_version $VERSION

# Check if we're in a git repository
check_git_repo

# Execute based on type
case $TYPE in
    "start")
        start_release $VERSION
        ;;
    "finish")
        finish_release $VERSION
        ;;
    "hotfix")
        create_hotfix $VERSION
        ;;
    *)
        echo -e "${RED}❌ Invalid type: $TYPE${NC}"
        echo "Valid types: start, finish, hotfix"
        exit 1
        ;;
esac
