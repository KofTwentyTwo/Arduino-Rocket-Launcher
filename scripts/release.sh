#!/bin/bash

# Luke's Rocket Launcher - Release Helper Script
# This script helps create semantic versioned releases

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_header() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -v, --version VERSION    Specify exact version (e.g., 1.0.0)"
    echo "  -t, --type TYPE          Release type: patch, minor, major (default: patch)"
    echo "  -m, --message MESSAGE    Custom release message"
    echo "  -p, --pre-release        Mark as pre-release (alpha/beta/rc)"
    echo "  -d, --dry-run            Show what would be done without doing it"
    echo "  -w, --watch              Watch the release workflow progress"
    echo "  -h, --help               Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --type minor          # Auto-increment minor version"
    echo "  $0 --version 1.0.0       # Release specific version"
    echo "  $0 --type patch --message 'Bug fix release'"
    echo "  $0 --version 0.1.0 --pre-release # Create pre-release"
    echo "  $0 --version 1.0.0 --watch       # Create release and watch progress"
    echo ""
    echo "Conventional Commit Messages:"
    echo "  feat: new feature"
    echo "  fix: bug fix"
    echo "  docs: documentation changes"
    echo "  style: code style changes"
    echo "  refactor: code refactoring"
    echo "  test: adding or updating tests"
    echo "  chore: maintenance tasks"
}

# Parse command line arguments
VERSION=""
RELEASE_TYPE="patch"
MESSAGE=""
PRE_RELEASE=false
DRY_RUN=false
WATCH=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -v|--version)
            VERSION="$2"
            shift 2
            ;;
        -t|--type)
            RELEASE_TYPE="$2"
            shift 2
            ;;
        -m|--message)
            MESSAGE="$2"
            shift 2
            ;;
        -p|--pre-release)
            PRE_RELEASE=true
            shift
            ;;
        -d|--dry-run)
            DRY_RUN=true
            shift
            ;;
        -w|--watch)
            WATCH=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Validate release type
if [[ ! "$RELEASE_TYPE" =~ ^(patch|minor|major)$ ]]; then
    print_error "Invalid release type: $RELEASE_TYPE"
    print_error "Must be one of: patch, minor, major"
    exit 1
fi

print_header "Luke's Rocket Launcher Release Helper"

# Get current version from git tags
CURRENT_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "v0.0.0")
CURRENT_VERSION=${CURRENT_TAG#v}

print_status "Current version: $CURRENT_VERSION"

# Calculate new version
if [[ -n "$VERSION" ]]; then
    NEW_VERSION="$VERSION"
    print_status "Using specified version: $NEW_VERSION"
else
    IFS='.' read -r MAJOR MINOR PATCH <<< "$CURRENT_VERSION"
    
    case "$RELEASE_TYPE" in
        "major")
            NEW_VERSION="$((MAJOR + 1)).0.0"
            print_status "Incrementing major version: $CURRENT_VERSION → $NEW_VERSION"
            ;;
        "minor")
            NEW_VERSION="$MAJOR.$((MINOR + 1)).0"
            print_status "Incrementing minor version: $CURRENT_VERSION → $NEW_VERSION"
            ;;
        "patch")
            NEW_VERSION="$MAJOR.$MINOR.$((PATCH + 1))"
            print_status "Incrementing patch version: $CURRENT_VERSION → $NEW_VERSION"
            ;;
    esac
fi

NEW_TAG="v$NEW_VERSION"

print_status "New version: $NEW_VERSION"
print_status "New tag: $NEW_TAG"

# Check if working directory is clean
if [[ -n $(git status --porcelain) ]]; then
    print_warning "Working directory is not clean. Please commit or stash changes first."
    git status --short
    exit 1
fi

# Check if tag already exists
if git tag -l | grep -q "^$NEW_TAG$"; then
    print_error "Tag $NEW_TAG already exists!"
    exit 1
fi

# Show what will be done
print_header "Release Plan"
echo "1. Create tag: $NEW_TAG"
if [[ "$PRE_RELEASE" == true ]]; then
    echo "   ⚠️  This will be marked as a PRE-RELEASE"
fi
echo "2. Push tag to remote"
echo "3. Trigger GitHub Actions release workflow"
echo "4. Create GitHub release with firmware files"
echo "5. Package firmware files for download"

if [[ "$DRY_RUN" == true ]]; then
    print_status "DRY RUN - No changes will be made"
    exit 0
fi

# Confirm release
echo ""
read -p "Proceed with release? (y/N): " -n 1 -r
echo ""
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    print_status "Release cancelled"
    exit 0
fi

# Create and push tag
print_status "Creating tag $NEW_TAG..."
if [[ "$PRE_RELEASE" == true ]]; then
    TAG_MESSAGE="Pre-release $NEW_TAG

$MESSAGE

⚠️  This is a pre-release version. Not recommended for production use."
else
    TAG_MESSAGE="Release $NEW_TAG

$MESSAGE"
fi

git tag -a "$NEW_TAG" -m "$TAG_MESSAGE"

print_status "Pushing tag to remote..."
git push origin "$NEW_TAG"

print_header "Release Complete!"
if [[ "$PRE_RELEASE" == true ]]; then
    print_warning "Tag $NEW_TAG has been created and pushed as a PRE-RELEASE"
    print_warning "Remember to mark this as a pre-release in GitHub after the workflow completes"
else
    print_status "Tag $NEW_TAG has been created and pushed"
fi
print_status "GitHub Actions will automatically:"
echo "  • Build and test the firmware"
echo "  • Create a GitHub release"
echo "  • Package firmware files for download"
echo ""
print_status "Monitor progress at: https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/actions"
print_status "Download firmware at: https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/releases"

# Watch workflow if requested
if [[ "$WATCH" == true ]]; then
    echo ""
    print_status "Watching release workflow progress..."
    print_status "Looking for workflow triggered by tag $NEW_TAG..."
    
    # Wait a moment for the workflow to start
    sleep 3
    
    # Find the workflow run for this tag
    WORKFLOW_RUN=$(gh run list --workflow="Build and Test Arduino Firmware" --limit 10 | grep "$NEW_TAG" | head -1 | awk '{print $NF}')
    
    if [[ -n "$WORKFLOW_RUN" ]]; then
        print_status "Found workflow run: $WORKFLOW_RUN"
        print_status "Starting workflow monitor..."
        echo ""
        
        # Watch the workflow
        gh run watch "$WORKFLOW_RUN"
        
        # Show final status
        echo ""
        print_status "Workflow completed! Final status:"
        gh run view "$WORKFLOW_RUN" --json status,conclusion,url --jq '.status + " (" + .conclusion + ")"'
        echo ""
        print_status "View details at: $(gh run view "$WORKFLOW_RUN" --json url --jq '.url')"
    else
        print_warning "Could not find workflow run for tag $NEW_TAG"
        print_status "You can manually check at: https://github.com/KofTwentyTwo/Lukes-Rocket-Launcher/actions"
    fi
fi
