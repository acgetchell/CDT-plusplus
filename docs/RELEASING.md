# Releasing CDT++

This guide documents the release flow for CDT++ release candidates and the
final `v1.0.0` release: prepare a dedicated release pull request, merge it,
create an annotated tag with reviewed release notes, and create the GitHub
release. The final stable release also includes the Zenodo archival handoff
tracked by [issue #96].

CDT++ is not published to a package registry. The GitHub release and its
automatically provided source archives are the release artifacts. The Python
project and vcpkg manifest are repository tooling and dependency metadata; do
not publish them independently as part of this process.

## Conventions

Run the commands from the repository root. Set `TAG` for the release being
made; the rest of the workflow is identical:

```bash
# The Git tag has a leading v; metadata versions do not.
# For the stable release, use TAG=v1.0.0.
TAG=v1.0.0-rc2
VERSION="${TAG#v}"

RELEASE_FLAGS=()
if [[ "$VERSION" == *-rc* ]]; then
  RELEASE_FLAGS+=(--prerelease)
fi
```

A release-candidate tag such as `v1.0.0-rc2` uses `1.0.0-rc2` for product,
vcpkg, Doxygen, and citation metadata, and the PEP 440 spelling `1.0.0rc2` for
Python metadata. A stable tag such as `v1.0.0` uses `1.0.0` everywhere else.

The changelog workflow requires the pinned `git-cliff` version declared by the
Justfile. Check or install it with:

```bash
git-cliff --version
cargo install git-cliff --version "$(just --evaluate git_cliff_version)" --locked
```

## Step 1: Prepare the release pull request

All feature and fix work intended for the release should already be on `main`.
Keep the release pull request limited to release metadata, active release
documentation, and its lockfile consequences.

Start from an up-to-date, clean `main` branch:

```bash
git remote -v
git switch main
git pull --ff-only
git status --short
git switch -c "release/$TAG"
```

The status command should produce no output before creating the branch.

### Update release metadata

Edit the following synchronized metadata:

- `CMakeLists.txt`: numeric `project(... VERSION ...)` and
  `CDT_VERSION_SUFFIX`; use an empty suffix for a stable release.
- `vcpkg.json`: `version`.
- `pyproject.toml`: `[project] version`, using the PEP 440 spelling.
- `docs/Doxyfile`: `PROJECT_NUMBER`.
- `CITATION.cff`: `version` and `date-released`.
- `README.md`, `REFERENCES.md`, and `.github/CONTRIBUTING.md`: active release
  references and release-facing prose.

For `v1.0.0-rc2`, synchronize the versions in the repository before generating
the changelog.
Set `CITATION.cff`'s `date-released` to the actual release date and review the
other fields rather than rewriting unchanged metadata:

```bash
${EDITOR:-vi} \
  CMakeLists.txt \
  vcpkg.json \
  pyproject.toml \
  docs/Doxyfile \
  CITATION.cff \
  README.md \
  REFERENCES.md \
  .github/CONTRIBUTING.md

uv lock
just changelog-unreleased "$TAG"
```

Review the citation identity fields at the same time: title, authors, ORCID,
repository, URL, license, and release date. Add or change a DOI only when the
archival policy and the correct DOI are known.

`just changelog-unreleased` validates `TAG` against all synchronized release
metadata, runs `git-cliff` offline, takes the release date from `CITATION.cff`,
and atomically replaces `CHANGELOG.md`. Review the generated release section
before committing it.

### Validate the release branch

Run the release-specific metadata check, strict documentation check, and full
cross-platform-equivalent local validation gate:

```bash
just release-check
just docs-check
just ci
```

`just release-check` verifies synchronization across CMake, vcpkg, Python,
`uv.lock`, Doxygen, citation metadata, and active release-candidate references.

Review and commit only release artifacts:

```bash
git diff --check
git diff

git add \
  CMakeLists.txt \
  vcpkg.json \
  pyproject.toml \
  uv.lock \
  docs/Doxyfile \
  CITATION.cff \
  CHANGELOG.md \
  README.md \
  REFERENCES.md \
  .github/CONTRIBUTING.md

git diff --cached --check
git diff --cached --stat
git commit -m "chore(release): release $TAG"
```

Push the branch and open the release pull request:

```bash
git push -u origin "release/$TAG"

gh pr create \
  --base main \
  --head "release/$TAG" \
  --title "chore(release): release $TAG" \
  --body "Finalize synchronized release metadata and the release date for $TAG. No feature work."

gh pr checks --watch
```

Merge the pull request only after every required check passes. Use the normal
repository merge strategy; from the command line:

```bash
gh pr merge --merge
```

## Step 2: Tag the merged release

Return to `main`, update it to the merge commit, and repeat the release gates
against the exact commit that will be tagged:

```bash
TAG=v1.0.0-rc2

git switch main
git pull --ff-only
git status --short

just release-check
just docs-check
just ci

test "$(git rev-parse HEAD)" = "$(git rev-parse origin/main)"
test -z "$(git status --porcelain)"
test -z "$(git tag -l "$TAG")"
```

Preview the annotated tag that will be created from the matching
`CHANGELOG.md` section, then create and inspect it:

```bash
just tag-check "$TAG"
just tag "$TAG"
git tag -l --format='%(contents)' "$TAG"
git push origin "$TAG"
```

Both tag commands reject malformed SemVer, missing or empty changelog sections,
existing tags, and dirty worktrees. `just tag-check` performs every preflight
without changing Git state. Do not move or replace a pushed release tag. If the
tagged commit is wrong, prepare a new version instead.

## Step 3: Publish and verify the release

Create the GitHub release from the annotated tag. `RELEASE_FLAGS` marks an RC
as a prerelease and expands to no arguments for a stable release:

Before publishing a release that Zenodo should archive, sign in to Zenodo with
the GitHub-linked account, enable this repository under **Profile → GitHub**,
and verify that GitHub has exactly one active Zenodo release webhook without
printing its token-bearing callback URL:

```bash
ZENODO_HOOK_COUNT="$(
  gh api repos/acgetchell/CDT-plusplus/hooks \
    --jq '[.[] | select(
      .active and
      (.events | index("release")) and
      (.config.url | startswith("https://zenodo.org/"))
    )] | length'
)"
test "$ZENODO_HOOK_COUNT" -eq 1
```

Stop if this preflight fails. Enabling Zenodo after publication does not
reliably import an earlier GitHub release.

```bash
TAG=v1.0.0-rc2
RELEASE_FLAGS=(--prerelease)

gh release create "$TAG" \
  --verify-tag \
  --title "$TAG" \
  --notes-from-tag \
  "${RELEASE_FLAGS[@]}"
```

Published releases may be immutable. Never delete a published release to
retrigger an integration: GitHub permanently reserves an immutable release's
tag name, so the release cannot be recreated with that tag. Verify external
integrations before publishing and create a new version if a release must be
repeated.

Always use the exact tag, including the leading `v`, as the release title.
Verify the published release:

```bash
gh release view "$TAG" \
  --json url,tagName,name,isDraft,isPrerelease \
  --jq .

ZENODO_HOOK_ID="$(
  gh api repos/acgetchell/CDT-plusplus/hooks \
    --jq '.[] | select(
      .active and
      (.events | index("release")) and
      (.config.url | startswith("https://zenodo.org/"))
    ) | .id'
)"
gh api \
  "repos/acgetchell/CDT-plusplus/hooks/$ZENODO_HOOK_ID/deliveries" \
  --jq '[.[] | select(.event == "release")][0] |
    {event, action, status, status_code, delivered_at, redelivery}'
```

For an intentionally archived release candidate, verify the Zenodo record and
DOI without treating it as the final archival handoff. For a stable release,
complete the Zenodo handoff tracked in [issue #96]: confirm that Zenodo received
the release, verify its version, date, title, authors, license, repository URL,
and files, and record the final DOI according to the archival plan. If the DOI
was unavailable before tagging, update `CITATION.cff` in a focused follow-up
pull request rather than rewriting the tag. Complete repository archival only
after the GitHub release and Zenodo record are correct.

## Step 4: Clean up

After the release is verified, remove the release branch if the pull request
did not already delete it:

```bash
git branch -d "release/$TAG"
git push origin --delete "release/$TAG"
```

If the remote branch was already deleted, the remote deletion command may be
omitted.

## Release-critical fixes

If a critical problem is found before tagging, fix it through a pull request,
regenerate `CHANGELOG.md` with `just changelog-unreleased "$TAG"` after the fix
is merged, and rerun all validation steps. Defer non-critical fixes to the next
version and document them in the changelog when appropriate.

If a problem is found after the tag is pushed or the GitHub release is
published, do not retag the release. Correct it in a new release.

[issue #96]: https://github.com/acgetchell/CDT-plusplus/issues/96
