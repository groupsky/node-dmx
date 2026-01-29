---
paths: /.github/workflows/release.yml, /.github/workflows/pkg-pr-new.yml
---

# Release Process Guidelines

Automated release management using release-please. Creates GitHub releases with generated changelogs from conventional commits.

## Workflow Behavior

On every push to main:
- Analyzes commits since last release
- Creates/updates "Release PR" with CHANGELOG.md and version bump
- Merging Release PR creates GitHub release automatically

## Version Bumping

Conventional commit types determine version bump:
- `fix:` = PATCH (1.0.x)
- `feat:` = MINOR (1.x.0)
- `feat!:` or `BREAKING CHANGE:` = MAJOR (x.0.0)

See: `docs/agents/git.md` for commit message format

## Release Process

1. Merge PRs to main with conventional commits
2. Review auto-generated Release PR (changelog, version)
3. Merge Release PR when ready to publish
4. GitHub release created automatically

## Preview Packages

Automatic on pull requests via pkg.pr.new. Preview URLs posted as PR comments.

## Configuration

Inline configuration in workflow file using `release-type: node` parameter. No additional files needed.

See: `.github/workflows/release.yml` for workflow implementation
See: `.github/workflows/pkg-pr-new.yml` for preview packages

## References

- https://github.com/googleapis/release-please - Official documentation
- https://github.com/googleapis/release-please-action - GitHub Action
