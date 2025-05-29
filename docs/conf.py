# Configuration file for the Sphinx documentation builder.

import sys
from pathlib import Path

# -- Paths -------------------------------------------------------------------

MATTER_BASE = Path(__file__).resolve().parents[1]

sys.path.insert(0, str(MATTER_BASE / "docs" / "_extensions"))

# -- Project information -----------------------------------------------------

project = "Matter"
copyright = "2020-2023, Matter Contributors"
author = "Matter Contributors"
version = "1.0.0"

# -- General configuration ---------------------------------------------------

extensions = [
    "myst_parser",
    "external_content",
]
exclude_patterns = [
    "_build",
    "examples/android/*",
    "**/nxp/linux-imx/imx8m/README.md",
    "examples/ota-requestor-app/efr32/README.md",
    "**/android/App/app/libs*",
    "examples/providers/README.md",
    "examples/thermostat/nxp/linux-se05x/README.md",
    "examples/common/m5stack-tft/repo",
    "docs/guides/README.md",
]


# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_book_theme"
html_logo = "_static/images/logo.png"
html_favicon = "_static/images/favicon.ico"
html_static_path = ["_static"]
html_theme_options = {
    "github_url": "https://github.com/project-chip/connectedhomeip",
    "repository_url": "https://github.com/project-chip/connectedhomeip",
    "use_edit_page_button": True,
    "repository_branch": "master",
    "path_to_docs": "docs",
    "icon_links": [],
}

# -- Options for MyST --------------------------------------------------------

myst_heading_anchors = 6
suppress_warnings = [
    "myst.header",
    "myst.anchor",
]
myst_enable_extensions = ["html_image"]


# -- Options for external_content --------------------------------------------

external_content_contents = [
    (MATTER_BASE / "docs", "[!_R]*"),
    (MATTER_BASE, "data_model/**/*.md"),
    (MATTER_BASE, "README.md"),
    (MATTER_BASE, "examples/**/*.md"),
    (MATTER_BASE, "examples/**/*.png"),
    (MATTER_BASE, "examples/**/*.jpg"),
    (MATTER_BASE, "examples/**/*.JPG"),
    (MATTER_BASE, "src/tools/**/*.md"),
    (MATTER_BASE, "scripts/tools/**/*.md"),
    (MATTER_BASE, "scripts/tools/**/*.png"),
]
external_content_link_prefixes = [
    "src/",
    r"\.vscode/",
    "CONTRIBUTING",  # cannot detect CONTRIBUTING.md
    "README",  # cannot detect README.md
    "scripts/",
    "examples/android/",
    "data_model/",
]
external_content_link_extensions = [".md", ".png", ".jpg", ".svg"]
