#!/usr/bin/env bash

# This script will update the current version number being tracked by git.
set -e

if [ -z "$1" ]; then
  echo "Usage: $0 vX.Y.Z"
  exit 1
fi

VERSION="$1"

# Basic sanity check
if [[ ! "$VERSION" =~ ^v[0-9]+\.[0-9]+\.[0-9]+.*$ ]]; then
  echo "Error: version must look like vX.Y.Z (optionally with suffix)"
  exit 1
fi

# Check for unpushed commits
CURRENT_BRANCH="$(git rev-parse --abbrev-ref HEAD)"
UNPUSHED_COMMITS="$(git rev-list --count origin/$CURRENT_BRANCH..HEAD 2>/dev/null || echo 0)"

if [ "$UNPUSHED_COMMITS" -gt 0 ]; then
  echo
  echo "Warning: You have $UNPUSHED_COMMITS unpushed commit(s) on $CURRENT_BRANCH."
  read -p "Do you want to push them before tagging? [y/N] " PUSH_CONFIRM

  case "$PUSH_CONFIRM" in
    y|Y|yes|YES)
      echo "Pushing commits..."
      git push origin "$CURRENT_BRANCH"
      ;;
    *)
      echo "Aborted."
      exit 0
      ;;
  esac
fi  

echo
echo "You are about to create and push the tag:"
echo "  $VERSION"
echo
read -p "Are you sure you want to update the version to $VERSION? [y/N] " CONFIRM

case "$CONFIRM" in
  y|Y|yes|YES)
    ;;
  *)
    echo "Aborted."
    exit 0
    ;;
esac

# Ensure tag doesn't already exist
if git rev-parse "$VERSION" >/dev/null 2>&1; then
  echo "Error: tag $VERSION already exists."
  exit 1
fi

echo "Creating annotated tag..."
git tag -a "$VERSION" -m "Release $VERSION"

echo "Pushing tag to origin..."
git push origin "$VERSION"

echo "Done ✔  Tagged and pushed $VERSION"
