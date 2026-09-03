# Documentation Summary

CrossPP is a lightweight, open-source e-reader firmware for ESP32-C3/S3 e-paper devices (Xteink X3/X4 and adjacent hardware) with fast EPUB rendering, custom fonts, offline dictionaries, wireless transfers, and built-in reading statistics.

## Agent Context Guide

Before planning or implementing, read this `docs/SUMMARY.md` file first. Load only the detail docs relevant to the current task, and prioritize `Code Standard` docs for implementation conventions. If docs conflict with code or user intent, use the available question tool before making broad changes.

## Architecture

System design, component interactions, data flows, and hardware integration.

| File | Description |
| ---- | ----------- |
| [components.md](architecture/components.md) | High-level system architecture, HAL abstractions, graphic rendering pipeline, and activity lifecycle |

## Codebase

Directory structure, entry points, and key module responsibilities.

| File | Description |
| ---- | ----------- |
| [structure.md](codebase/structure.md) | File organization, directory map, entry points, and key module responsibilities |

## Code Standard

Embedded conventions, memory rules, and development workflows.

| File | Description |
| ---- | ----------- |
| [conventions.md](code-standard/conventions.md) | 380KB RAM constraints, memory safety, RAII rules, formatting commands, and ESP32-C3 platform pitfalls |

## Project PDR

Product goals, features, and core constraints.

| File | Description |
| ---- | ----------- |
| [product-goals.md](project-pdr/product-goals.md) | Product vision, reading statistics features, wireless workflows, and explicit non-goals |

## Other

Existing reference documentation and hardware troubleshooting guides.

| File | Description |
| ---- | ----------- |
| [activity-manager.md](activity-manager.md) | Navigation patterns and activity stack management |
| [file-formats.md](file-formats.md) | SD cache file binary formats (`book.bin`, `section.bin`, `reading_stats.json`) |
| [dictionary.md](dictionary.md) | StarDict format specifications and integration details |
| [sd-card-fonts.md](sd-card-fonts.md) | Custom SD-card `.cpfont` architecture and build workflow |
| [focus-reading.md](focus-reading.md) | Focus reading / bionic reading typography mode |
| [hyphenation-trie-format.md](hyphenation-trie-format.md) | Binary layout of hyphenation trie dictionaries |
| [i18n.md](i18n.md) | Internationalization system, string table generation, and RTL support |
| [webserver.md](webserver.md) | Embedded Web server architecture and transfer protocols |
| [webserver-endpoints.md](webserver-endpoints.md) | REST API endpoints for web file manager and settings control |
| [troubleshooting.md](troubleshooting.md) | Diagnostic steps and recovery procedures |
| [fix-bricked-xteink.md](fix-bricked-xteink.md) | Hardware unbricking using SPI flash clip |
| [comparison.md](comparison.md) | Feature comparison with stock firmware |
| [translators.md](translators.md) | Localization contributors and translation guidelines |
