#!/usr/bin/env python3
"""Validate the self-contained generated Doxygen HTML site."""

import argparse
import re
import sys
from collections import Counter
from dataclasses import dataclass
from html.parser import HTMLParser
from pathlib import Path
from typing import override
from urllib.parse import unquote, urlsplit

REFERENCE_ATTRIBUTES = ("href", "src", "data", "poster")
SKIPPED_SCHEMES = frozenset({"data", "http", "https", "javascript", "mailto", "tel"})
LITERAL_MARKDOWN_LINK = re.compile(r"\[[^\]\n]+\]\([^\)\n]+\)")
CSS_URL = re.compile(r"url\(\s*(?P<quote>['\"]?)(?P<url>[^)'\"]+)(?P=quote)\s*\)")
WHITESPACE = re.compile(r"\s+")
COMPLETENESS_WARNING = re.compile(r"(?:\bis not documented[.]?$|\b(?:parameters|return type) of member .+ (?:are|is) not documented$)")
INTERNAL_SYMBOL = re.compile(r"::detail(?:::|\s)|\bcdt::experimental::")
SOURCE_ARTIFACT = re.compile(
    r"(?:Member (?:catch|main|Timer)\b.*\bis not documented[.]?$|"
    r"parameters of member (?:catch|main)\b.*\bare not documented$)"
)
PSEUDO_SOURCE_ARTIFACT = re.compile(r"^<(?:catch|main)>:\d+: warning: ")

REQUIRED_ASSETS = (
    "S3-7-27528-I1-R1.png",
    "clipboard.js",
    "cookie.js",
    "doxygen.css",
    "dynsections.js",
    "menu.js",
    "menudata.js",
    "search/search.css",
    "search/search.js",
    "search/searchdata.js",
    "tabs.css",
)
REQUIRED_TARGETS = {
    "index.html": ("developer-workflow", "documentation", "introduction"),
    "md__r_e_f_e_r_e_n_c_e_s.html": ("metropolis-hastings-algorithm",),
    "md_docs_2api-boundary.html": ("generated-reference-policy", "header-classification"),
    "md_docs_2comparison-harness.html": (),
    "md_docs_2cpp-api-quickstart.html": (
        "workflow",
        "build-and-run",
        "failure-and-ownership-behavior",
    ),
    "md_docs_2ergodic-moves.html": (),
    "md_docs_2metropolis-hastings.html": (),
    "md_docs_2reproducibility.html": (),
    "md_docs_2viewer.html": (),
}


class GeneratedSiteError(ValueError):
    """The generated site violates its archival publication contract."""


TextCache = dict[Path, str]


def _decoded_text(path: Path, text_cache: TextCache, *, errors: str = "strict") -> str:
    """Decode one generated file once for all validation passes."""
    resolved = path.resolve()
    if resolved not in text_cache:
        text_cache[resolved] = resolved.read_text(encoding="utf-8", errors=errors)
    return text_cache[resolved]


@dataclass(frozen=True)
class Reference:
    """One URL-bearing HTML or CSS attribute."""

    source: Path
    line: int
    value: str


def _normalized_text(value: str) -> str:
    """Collapse rendered whitespace for stable duplicate-label comparisons."""
    return WHITESPACE.sub(" ", value).strip()


class _DocumentParser(HTMLParser):
    """Collect link targets, resource references, and rendered prose evidence."""

    def __init__(self, source: Path) -> None:
        super().__init__(convert_charrefs=True)
        self.source = source
        self.ids: list[tuple[int, str]] = []
        self.targets: set[str] = set()
        self.references: list[Reference] = []
        self.prose: list[tuple[int, str]] = []
        self.duplicate_link_labels: list[tuple[int, str]] = []
        self._element_stack: list[tuple[str, bool]] = []
        self._suppressed_depth = 0
        self._anchor_chunks: list[str] | None = None
        self._anchor_is_doxygen_link = False
        self._pending_anchor: tuple[int, str] | None = None

    def _add_reference(self, value: str) -> None:
        self.references.append(Reference(self.source, self.getpos()[0], value))

    def _collect_identifiers(self, attributes: dict[str, str | None]) -> None:
        """Collect fragment targets without treating matching id/name as duplicates."""
        identifier = attributes.get("id")
        if identifier:
            self.ids.append((self.getpos()[0], identifier))
            self.targets.add(identifier)
        name = attributes.get("name")
        if name:
            self.targets.add(name)

    def _collect_references(self, attributes: dict[str, str | None]) -> None:
        """Collect ordinary and responsive resource references."""
        for attribute in REFERENCE_ATTRIBUTES:
            value = attributes.get(attribute)
            if value:
                self._add_reference(value)
        srcset = attributes.get("srcset")
        if srcset:
            for candidate in srcset.split(","):
                value = candidate.strip().split(maxsplit=1)[0]
                if value:
                    self._add_reference(value)

    def _push_element(self, tag: str, classes: frozenset[str]) -> None:
        """Track elements whose contents are code rather than rendered prose."""
        suppressed_here = tag in {"code", "pre", "script", "style"} or bool(classes & {"fragment", "line"})
        self._element_stack.append((tag, suppressed_here))
        if suppressed_here:
            self._suppressed_depth += 1

    @override
    def handle_starttag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        """Collect identifiers and URL-bearing attributes from one element."""
        if self._pending_anchor is not None:
            self._pending_anchor = None

        attributes = dict(attrs)
        self._collect_identifiers(attributes)
        self._collect_references(attributes)
        classes = frozenset((attributes.get("class") or "").split())
        self._push_element(tag, classes)

        if tag == "a":
            self._anchor_chunks = []
            self._anchor_is_doxygen_link = "el" in classes

    @override
    def handle_startendtag(self, tag: str, attrs: list[tuple[str, str | None]]) -> None:
        """Process one self-closing element without retaining stack state."""
        self.handle_starttag(tag, attrs)
        self.handle_endtag(tag)

    @override
    def handle_endtag(self, tag: str) -> None:
        """Close parser state and retain one Doxygen link label for regression checks."""
        if tag == "a" and self._anchor_chunks is not None:
            label = _normalized_text("".join(self._anchor_chunks))
            if label and self._anchor_is_doxygen_link:
                self._pending_anchor = (self.getpos()[0], label)
            self._anchor_chunks = None
            self._anchor_is_doxygen_link = False

        for index in range(len(self._element_stack) - 1, -1, -1):
            element, _ = self._element_stack[index]
            if element != tag:
                continue
            removed = self._element_stack[index:]
            del self._element_stack[index:]
            self._suppressed_depth -= sum(suppressed for _, suppressed in removed)
            break

    @override
    def handle_data(self, data: str) -> None:
        """Collect anchor text and prose while detecting Doxygen 1.17 label duplication."""
        if self._anchor_chunks is not None:
            self._anchor_chunks.append(data)
        elif self._pending_anchor is not None:
            line, label = self._pending_anchor
            rendered = _normalized_text(data)
            if rendered:
                if rendered.startswith(label):
                    self.duplicate_link_labels.append((line, label))
                self._pending_anchor = None

        if self._suppressed_depth == 0 and data:
            self.prose.append((self.getpos()[0], data))


def _parse_document(path: Path, text_cache: TextCache) -> _DocumentParser:
    """Parse one generated HTML document with path-specific diagnostics."""
    parser = _DocumentParser(path)
    try:
        parser.feed(_decoded_text(path, text_cache))
        parser.close()
    except (OSError, UnicodeError) as error:
        message = f"{path}: cannot parse generated HTML: {error}"
        raise GeneratedSiteError(message) from error
    return parser


def _relative(path: Path, root: Path) -> str:
    """Return a stable site-relative path for diagnostics."""
    return path.relative_to(root).as_posix()


def _warning_message(line: str) -> str:
    """Return the diagnostic text following Doxygen's warning prefix."""
    _, separator, message = line.partition("warning: ")
    return message if separator else line


def _is_ignored_completeness_warning(line: str) -> bool:
    """Whether *line* documents an intentionally unsupported implementation symbol."""
    message = _warning_message(line)
    if COMPLETENESS_WARNING.search(message) is None:
        return False
    if INTERNAL_SYMBOL.search(message) is not None:
        return True
    normalized = line.replace("\\", "/")
    source_path = f"/{normalized.lstrip('/')}"
    originates_in_source = "/src/" in source_path or PSEUDO_SOURCE_ARTIFACT.search(line) is not None
    return originates_in_source and SOURCE_ARTIFACT.search(message) is not None


def validate_warning_log(path: Path) -> int:
    """Reject every Doxygen warning except scoped implementation-completeness noise."""
    try:
        lines = path.read_text(encoding="utf-8").splitlines()
    except (OSError, UnicodeError) as error:
        message = f"cannot read Doxygen warning log {path}: {error}"
        raise GeneratedSiteError(message) from error

    warnings = [line for line in lines if line.strip()]
    ignored_count = 0
    actionable = []
    for line in warnings:
        if _is_ignored_completeness_warning(line):
            ignored_count += 1
        else:
            actionable.append(line)
    if actionable:
        message = "Doxygen public documentation validation failed:\n" + "\n".join(f"- {line}" for line in actionable)
        raise GeneratedSiteError(message)
    return ignored_count


def _resolve_local_target(reference: Reference, root: Path) -> tuple[Path, str] | None:
    """Resolve one local reference, returning its target path and fragment."""
    parsed = urlsplit(reference.value)
    if parsed.scheme.lower() in SKIPPED_SCHEMES or reference.value.startswith("//"):
        return None
    if parsed.scheme:
        return None

    path_text = unquote(parsed.path)
    if path_text.startswith("/"):
        target = root / path_text.lstrip("/")
    elif path_text:
        target = reference.source.parent / path_text
    else:
        target = reference.source
    return target.resolve(), unquote(parsed.fragment)


def _validate_required_pages(root: Path, documents: dict[Path, _DocumentParser]) -> list[str]:
    """Validate the archival pages and stable GitHub-style fragments."""
    errors: list[str] = []
    for relative, fragments in REQUIRED_TARGETS.items():
        target = (root / relative).resolve()
        document = documents.get(target)
        if document is None:
            errors.append(f"missing required page: {relative}")
            continue
        errors.extend(f"missing required fragment: {relative}#{fragment}" for fragment in fragments if fragment not in document.targets)
    return errors


def _validate_required_features(
    root: Path,
    documents: dict[Path, _DocumentParser],
    text_cache: TextCache,
) -> list[str]:
    """Validate the generated assets and presentation features retained for v1."""
    errors = [f"missing required asset: {relative}" for relative in REQUIRED_ASSETS if not (root / relative).is_file()]

    index_path = (root / "index.html").resolve()
    if index_path in documents:
        index_text = _decoded_text(index_path, text_cache)
        for marker, feature in (
            ('name="viewport"', "responsive viewport"),
            ("codefold.init", "code folding"),
            ('src="clipboard.js"', "copy controls"),
        ):
            if marker not in index_text:
                errors.append(f"index.html is missing required {feature} markup")

    stylesheet = root / "doxygen.css"
    if stylesheet.is_file() and "prefers-color-scheme: dark" not in _decoded_text(stylesheet, text_cache):
        errors.append("doxygen.css is missing automatic dark-mode styling")

    graphviz_outputs = sorted(root.rglob("*.svg"))
    if not any("Generated by graphviz" in _decoded_text(item, text_cache, errors="ignore") for item in graphviz_outputs):
        errors.append("no generated Graphviz SVG was found")
    return errors


def _document_content_errors(source_name: str, document: _DocumentParser) -> list[str]:
    """Return duplicate-identifier, duplicate-label, and literal-Markdown errors."""
    errors: list[str] = []
    counts = Counter(identifier for _, identifier in document.ids)
    errors.extend(f"{source_name}: duplicate id {identifier!r} ({count} occurrences)" for identifier, count in sorted(counts.items()) if count > 1)
    errors.extend(f"{source_name}:{line}: duplicated rendered link label {label!r}" for line, label in document.duplicate_link_labels)
    for line, prose in document.prose:
        match = LITERAL_MARKDOWN_LINK.search(prose)
        if match is not None:
            errors.append(f"{source_name}:{line}: literal Markdown link {match.group(0)!r}")
    return errors


def _reference_error(
    reference: Reference,
    source_name: str,
    root: Path,
    documents: dict[Path, _DocumentParser],
) -> str | None:
    """Return one local-reference diagnostic, or None when it resolves."""
    resolved = _resolve_local_target(reference, root)
    if resolved is None:
        return None
    target, fragment = resolved
    try:
        target.relative_to(root)
    except ValueError:
        return f"{source_name}:{reference.line}: local reference escapes the site: {reference.value!r}"
    if target.is_dir():
        target = target / "index.html"
    if not target.is_file():
        return f"{source_name}:{reference.line}: missing local target {reference.value!r}"
    if not fragment or target.suffix.lower() not in {".htm", ".html"}:
        return None
    target_document = documents.get(target.resolve())
    if target_document is None or fragment not in target_document.targets:
        return f"{source_name}:{reference.line}: missing fragment {reference.value!r}"
    return None


def _validate_documents(root: Path, documents: dict[Path, _DocumentParser]) -> list[str]:
    """Validate identifiers, prose, and local HTML/resource references."""
    errors: list[str] = []
    for source, document in sorted(documents.items()):
        source_name = _relative(source, root)
        errors.extend(_document_content_errors(source_name, document))
        errors.extend(error for reference in document.references if (error := _reference_error(reference, source_name, root, documents)) is not None)
    return errors


def _validate_css(root: Path, text_cache: TextCache) -> list[str]:
    """Validate local assets referenced from generated stylesheets."""
    errors: list[str] = []
    for stylesheet in sorted(root.rglob("*.css")):
        for line_number, line in enumerate(_decoded_text(stylesheet, text_cache).splitlines(), start=1):
            for match in CSS_URL.finditer(line):
                value = match.group("url").strip()
                reference = Reference(stylesheet.resolve(), line_number, value)
                resolved = _resolve_local_target(reference, root)
                if resolved is None:
                    continue
                target, _ = resolved
                try:
                    target.relative_to(root)
                except ValueError:
                    errors.append(f"{_relative(stylesheet.resolve(), root)}:{line_number}: stylesheet reference escapes the site: {value!r}")
                    continue
                if not target.is_file():
                    errors.append(f"{_relative(stylesheet.resolve(), root)}:{line_number}: missing stylesheet asset {value!r}")
    return errors


def validate(site: Path) -> int:
    """Validate *site* and return its generated HTML page count."""
    root = site.resolve()
    if not root.is_dir():
        message = f"generated site directory does not exist: {root}"
        raise GeneratedSiteError(message)

    text_cache: TextCache = {}
    documents = {item.resolve(): _parse_document(item.resolve(), text_cache) for item in sorted(root.rglob("*.html"))}
    errors = _validate_required_pages(root, documents)
    errors.extend(_validate_required_features(root, documents, text_cache))
    errors.extend(_validate_documents(root, documents))
    errors.extend(_validate_css(root, text_cache))

    for item in sorted(root.rglob("*")):
        if not item.is_file() or item.suffix.lower() not in {".css", ".html", ".js"}:
            continue
        text = _decoded_text(item, text_cache)
        if "mermaid.esm" in text or "cdn.jsdelivr.net/npm/mermaid" in text:
            errors.append(f"{_relative(item.resolve(), root)}: unused Mermaid client dependency")

    if errors:
        message = "generated site validation failed:\n" + "\n".join(f"- {error}" for error in sorted(set(errors)))
        raise GeneratedSiteError(message)
    return len(documents)


def main(argv: list[str] | None = None) -> int:
    """Run the generated-site validator CLI."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--warning-log", type=Path, help="Doxygen warning log to classify before site validation")
    parser.add_argument("site", nargs="?", type=Path, default=Path("docs/html"), help="generated HTML root")
    arguments = parser.parse_args(argv)
    try:
        ignored_warnings = validate_warning_log(arguments.warning_log) if arguments.warning_log else 0
        page_count = validate(arguments.site)
    except (GeneratedSiteError, OSError, UnicodeError) as error:
        print(error, file=sys.stderr)
        return 1
    print(f"Generated site validation complete ({page_count} HTML pages, {ignored_warnings} scoped implementation warnings).")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
