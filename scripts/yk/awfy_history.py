#!/usr/bin/env python3
"""Chart ykmruby's AWFY performance across PRs.

Reads the haste.zip attached to the description of each merged PR of ykjit/ykmruby and
writes a standalone site/index.html (Chart.js). Each PR is one point, in merge order,
and times are compared against the first PR.

  scripts/yk/awfy_history.py
"""
import datetime
import io
import json
import os
import re
import statistics
import string
import sys
import tomllib
import urllib.request
import zipfile
from collections.abc import Iterator
from typing import Any, TypedDict

REPO = "ykjit/ykmruby"
TEMPLATE = os.path.join(os.path.dirname(__file__), "awfy_history.html")
OUT_DIR = "site"


class PageTemplate(string.Template):
    delimiter = "@"


class Pr(TypedDict):
    number: int
    title: str
    date: str
    ms: dict[str, float]  # {benchmark: median wallclock ms}


def get(url: str) -> bytes:
    req = urllib.request.Request(url)
    token = os.environ.get("GITHUB_TOKEN")
    if token and "api.github.com" in url:
        req.add_header("Authorization", "Bearer " + token)
    with urllib.request.urlopen(req) as resp:
        body: bytes = resp.read()
    return body


def get_json(url: str) -> Any:
    return json.loads(get(url))


def merged_prs(api: str) -> list[dict[str, Any]]:
    """All merged PRs, oldest merge first."""
    prs = []
    page = 1
    while True:
        batch = get_json(f"{api}/pulls?state=closed&per_page=100&page={page}")
        prs += [pr for pr in batch if pr["merged_at"]]
        if len(batch) < 100:
            break
        page += 1
    return sorted(prs, key=lambda pr: pr["merged_at"])


def median_ms(zip_bytes: bytes) -> dict[str, float] | None:
    z = zipfile.ZipFile(io.BytesIO(zip_bytes))
    if "data.toml" not in z.namelist():
        return None
    data = tomllib.loads(z.read("data.toml").decode())["data"]
    medians: dict[str, float] = {}
    for key, runs in data.items():
        # haste keys look like "<benchmark>/<executor>/<argument>".
        benchmark, executor, _ = key.split("/")
        if executor == "mruby":
            medians[benchmark] = statistics.median(runs)
    return medians


def load_prs() -> Iterator[Pr]:
    api = f"https://api.github.com/repos/{REPO}"
    for pr in merged_prs(api):
        zip_link = re.search(r"https://github\.com/user-attachments/files/\d+/haste\.zip",
                             pr["body"] or "")
        if not zip_link:
            continue
        ms = median_ms(get(zip_link[0]))
        if ms:
            yield Pr(number=pr["number"], title=pr["title"], date=pr["merged_at"], ms=ms)


def to_js(value: object) -> str:
    """JSON to embed in a <script>. PR titles are untrusted, and "<\\/" stops one from
    closing the tag early."""
    return json.dumps(value, separators=(",", ":")).replace("</", "<\\/")


def main() -> None:
    prs = list(load_prs())
    if not prs:
        sys.exit(f"no merged PR in {REPO} has a haste zip attached")
    baseline_pr = prs[0]["number"]
    baseline = prs[0]["ms"]

    # One row per PR and benchmark, for the benchmarks the baseline also ran.
    rows: list[dict[str, object]] = []
    for pr in prs:
        for benchmark, ms in pr["ms"].items():
            if benchmark in baseline:
                rows.append(dict(pr=pr["number"], date=pr["date"], title=pr["title"],
                                 benchmark=benchmark, ms=round(ms)))

    # Geomean of time / baseline time, only for PRs that ran every baseline benchmark so
    # that the points are comparable.
    summary: list[dict[str, object]] = []
    for pr in prs:
        if baseline.keys() <= pr["ms"].keys():
            ratios = [pr["ms"][b] / baseline[b] for b in baseline]
            summary.append(dict(pr=pr["number"], date=pr["date"], title=pr["title"],
                            geomean=round(statistics.geometric_mean(ratios), 3)))

    with open(TEMPLATE) as f:
        page = PageTemplate(f.read()).substitute(
            BASE=f"PR #{baseline_pr}",
            REPO=REPO,
            GENERATED=datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            SUMMARY=json.dumps(summary, separators=(",", ":")).replace("</", "<\\/"),
            ROWS=json.dumps(rows, separators=(",", ":")).replace("</", "<\\/"),
        )

    os.makedirs(OUT_DIR, exist_ok=True)
    out_file = os.path.abspath(os.path.join(OUT_DIR, "index.html"))
    with open(out_file, "w") as f:
        f.write(page)
    print(f"{len(prs)} PRs, {len(summary)} with every benchmark")
    print(f"html written: {out_file}")


if __name__ == "__main__":
    main()
