---
paths: /.github/workflows/release.yml, /.github/workflows/pkg-pr-new.yml, /release-please-config.json
---

# Release Process Guidelines

Publishing to npm using release-please with conventional commits. Uses npm trusted publishers with OIDC for secure, tokenless publishing with provenance.

## Release Triggers

**Manual Releases**: Triggered manually via GitHub Actions workflow_dispatch. Two-step process:
1. Create GitHub release and update changelog
2. Optionally publish to npm registry

**Preview Packages (Pull Requests)**: Automatic via pkg.pr.new on pull requests. Creates temporary preview packages for testing without publishing to npm registry.

## Version Bumping

Conventional commit types determine version bump:
- `fix:` = PATCH (1.0.x)
- `feat:` = MINOR (1.x.0)
- `feat!:` or `BREAKING CHANGE:` = MAJOR (x.0.0)

SEE docs/agents/git.md for commit message format.

## How Release-Please Works

1. Analyzes commits since last release using conventional commits
2. Determines next version based on commit types
3. Generates/updates CHANGELOG.md from commit messages
4. Updates package.json version
5. Creates GitHub release with release notes
6. Optionally triggers npm publish job

## Triggering Releases

**Manual Release**:
1. Go to Actions → Release workflow
2. Click "Run workflow"
3. Select options:
   - "Create GitHub release" - generates changelog, tags version, creates GitHub release
   - "Publish to npm" - builds and publishes to npm (requires npm environment approval)

**Preview Packages**: Automatic on pull requests via pkg.pr.new. Preview URLs posted as PR comments.

## Configuration

**release-please-config.json**: Main configuration for release-please behavior
**.release-please-manifest.json**: Tracks current version (updated automatically)

## Requirements

- Conventional commit messages
- npm environment configured with OIDC trusted publishing
- Appropriate permissions in workflow (contents: write, pull-requests: write, id-token: write)

## References

- `.github/workflows/release.yml` - Manual release workflow
- `.github/workflows/pkg-pr-new.yml` - Preview packages workflow
- `release-please-config.json` - Release-please configuration
- `docs/agents/git.md` - Commit message format and conventional commits
- https://github.com/googleapis/release-please - Official release-please documentation
- https://github.com/googleapis/release-please-action - GitHub Action documentation
- https://github.com/stackblitz-labs/pkg.pr.new - Preview packages service
