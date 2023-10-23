# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'Matter'
copyright = 'Copyright © 2023 Silicon Laboratories. All rights reserved.'
author = 'Silicon Labs'
release = '2.2.0-1.2'
version = '2.2.0-1.2'


# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = ['myst_parser',
              "sphinx_rtd_theme",
              "sphinx_tabs.tabs"]


source_suffix = ['.rst', '.md']
templates_path = ['_templates']
exclude_patterns = ['_build', 
                    'Thumbs.db', 
                    '.DS_Store', 
                    'README.md', 
                    'DOC_STANDARD.md', 
                    'unify/matter-bridge/readme_chip_tool_testing.md']



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
html_theme_options = {
    'analytics_id': 'G-XXXXXXXXXX',  #  Provided by Google in your dashboard
    'analytics_anonymize_ip': False,
    'logo_only': False,
    'display_version': True,
    'prev_next_buttons_location': 'bottom',
    'style_external_links': False,
    'vcs_pageview_mode': '',
    'style_nav_header_background': '#3b82f6',
    # Toc options
    'collapse_navigation': True,
    'sticky_navigation': True,
    'navigation_depth': 3,
    'includehidden': True,
    'titles_only': False
}
html_static_path = ['_static']
html_favicon = 'favicon.ico'
html_logo = 'logo.png'
html_css_files = ['custom_styles.css']
myst_enable_extensions = [
    "amsmath",
    "attrs_inline",
    "colon_fence",
    "deflist",
    "dollarmath",
    "fieldlist",
    "html_admonition",
    "html_image",
    "linkify",
    "replacements",
    "smartquotes",
    "strikethrough",
    "substitution",
    "tasklist",
]
myst_heading_anchors = 4

# Setup the breathe extension
breathe_projects = {
    "My Project": "./_doxygen/xml"
}
breathe_default_project = "My Project"

# Setup the exhale extension
exhale_args = {
    # These arguments are required
    "containmentFolder":     "./dox",
    "rootFileName":          "index.rst",
    "doxygenStripFromPath":  "..",
    # Heavily encouraged optional argument (see docs)
    "rootFileTitle":         "Library API",
    # Suggested optional arguments
    "createTreeView":        True,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # "treeViewIsBootstrap": True,
    "exhaleExecutesDoxygen": True,
    "exhaleDoxygenStdin":    "INPUT = ./src"
}

# Tell sphinx what the primary language being documented is.
primary_domain = 'cpp'

# Tell sphinx what the pygments highlight language should be.
highlight_language = 'cpp'
