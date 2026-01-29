---
paths: /.github/workflows/release.yml, /.github/workflows/pkg-pr-new.yml, /release-please-config.json
---

# Release Process Guidelines

Automated release management using release-please with conventional commits. Creates GitHub releases with generated changelogs.

## Release Triggers

**Automatic on push to main**: release-please runs on every push to main and creates/updates a "Release PR" with changelog and version bumps. Merge the Release PR when ready to create the GitHub release.

**Preview Packages (Pull Requests)**: Automatic via pkg.pr.new on pull requests. Creates temporary preview packages for testing without publishing to npm registry.

## Version Bumping

Conventional commit types determine version bump:
- `fix:` = PATCH (1.0.x)
- `feat:` = MINOR (1.x.0)
- `feat!:` or `BREAKING CHANGE:` = MAJOR (x.0.0)

SEE docs/agents/git.md for commit message format.

## How Release-Please Works

1. On every push to main, analyzes commits since last release using conventional commits
2. Creates/updates a "Release PR" with:
   - Updated CHANGELOG.md from commit messages
   - Updated package.json version
   - Determined next version based on commit types
3. When you merge the Release PR, creates GitHub release with release notes
4. Manual npm publish as needed (not automated)

## Triggering Releases

**Production Release**:
1. Push commits to main (or merge PRs)
2. release-please automatically creates/updates a "Release PR"
3. Review the Release PR changelog and version bump
4. Merge the Release PR when ready to publish
5. GitHub release is automatically created
6. Manually publish to npm if needed

**Preview Packages**: Automatic on pull requests via pkg.pr.new. Preview URLs posted as PR comments.

## Configuration

**release-please-config.json**: Main configuration for release-please behavior
**.release-please-manifest.json**: Tracks current version (updated automatically)

## Requirements

- Conventional commit messages for version bump detection
- Workflow permissions: contents: write, pull-requests: write

## References

- `.github/workflows/release.yml` - Automatic release workflow
- `.github/workflows/pkg-pr-new.yml` - Preview packages workflow
- `release-please-config.json` - Release-please configuration
- `docs/agents/git.md` - Commit message format and conventional commits
- https://github.com/googleapis/release-please - Official release-please documentation
- https://github.com/googleapis/release-please-action - GitHub Action documentation
- https://github.com/stackblitz-labs/pkg.pr.new - Preview packages service
