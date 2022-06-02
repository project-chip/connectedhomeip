# Configuration file for the Sphinx documentation builder.

import os
from pathlib import Path
import sys

# -- Paths -------------------------------------------------------------------

MATTER_BASE = Path(__file__).resolve().parents[1]

sys.path.insert(0, str(MATTER_BASE / "docs" / "_extensions"))

# -- Project information -----------------------------------------------------

project = "Matter"
copyright = "2021, Matter Contributors"
author = "Matter Contributors"
version = "1.0.0"

# -- General configuration ---------------------------------------------------

extensions = ["myst_parser", "external_content", "doxyrunner", "breathe"]
exclude_patterns = [
    "_build",
    "**/nxp/linux-imx/imx8m/README.md",
    "examples/ota-requestor-app/efr32/README.md",
    "**/android/App/app/libs*",
    "examples/providers/README.md",
]


# -- Options for HTML output -------------------------------------------------

html_theme = "sphinx_book_theme"
html_logo = "_static/images/logo.png"
html_favicon = "_static/images/favicon.ico"
html_static_path = ["_static"]
html_theme_options = {
    "logo_only": True,
    "github_url": "https://github.com/project-chip/connectedhomeip",
    "repository_url": "https://github.com/project-chip/connectedhomeip",
    "use_edit_page_button": True,
    "repository_branch": "master",
    "path_to_docs": "docs",
}

# -- Options for MyST --------------------------------------------------------

myst_heading_anchors = 6
suppress_warnings = ["myst.header"]
myst_enable_extensions = ["html_image"]


# -- Options for external_content --------------------------------------------

external_content_contents = [
    (MATTER_BASE / "docs", "[!_]*"),
    (MATTER_BASE, "examples/**/*.md"),
    (MATTER_BASE, "examples/**/*.png"),
    (MATTER_BASE, "examples/**/*.jpg"),
    (MATTER_BASE, "examples/**/*.JPG"),
]
external_content_link_repositories = ["src", r"\.vscode"]
external_content_link_extensions = [".md", ".png", ".jpg", ".svg"]

# -- Options for zephyr.doxyrunner plugin ------------------------------------

doxyrunner_doxygen = os.environ.get("DOXYGEN_EXECUTABLE", "doxygen")
doxyrunner_doxyfile = MATTER_BASE / "docs" / "matter.doxyfile.in"
doxyrunner_outdir = MATTER_BASE / "docs" / "_build" / "doxygen"
doxyrunner_fmt = True
doxyrunner_fmt_vars = {"MATTER_BASE": str(MATTER_BASE), "MATTER_VERSION": version}

# -- Options for Breathe plugin -------------------------------------------

breathe_projects = {"Matter": str(doxyrunner_outdir / "xml")}
breathe_default_project = "Matter"
breathe_domain_by_extension = {
    "h": "cpp",
    "cpp": "cpp",
    "c": "c",
}
