# Homepage verification — 2026-10-27

Visual source: the OT1 repository's `docs/site/` and Pages workflow. OT0 retains its system font stack, 1052 px desktop container, white cards on #f0f2f7, amber controls and responsive breakpoint at 760 px. Reference and OT0 English/Chinese pages were rendered in headless Microsoft Edge at 1440×1100 and 390×844, including full-page captures.

| Check | Reference evidence | OT0 evidence and decision |
| --- | --- | --- |
| Layout and spacing | Two-column desktop, one-column mobile | Same CSS/container/card spacing; checked both widths and languages |
| Typography | System fonts and responsive title sizes | Same font stack and title sizing; primary text fits |
| Palette and controls | Gray background, white cards, amber download | Preserved colors, borders and focus styling |
| Images | Game-specific icon and full gameplay image | Uses existing OT0 PNG assets at their actual aspect ratios, without cropping |
| Compatibility copy | Short OT1 loader/version strings | Exact OT0 experimental build; loader labels do not shrink on mobile |
| Download action | OT1 prebuilt release | Intentional difference: source ZIP plus native build instructions; no nonexistent release links |
| Interaction | Language links, section anchors, FAQ, mobile contents | Links/assets/anchors resolve; FAQ and mobile contents expand |

Browser checks passed for missing images, horizontal page overflow and visible elements extending outside the viewport. Both full pages were reviewed; no clipped or overlapping primary content was found. No live-data integration exists. Local QA screenshots are temporary verification artifacts, not site assets.
