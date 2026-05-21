# SeatCanvas Agent Guide

## Quick Reference

**Build & Tooling** — when a build is required (verification, debugging, or user request), read and follow @./docs/agents/build.md exactly: full platform commands, prerequisites, and troubleshooting. Do not substitute `./ios/gen_simulator` unless the user explicitly requests a simulator build. Do not claim a build passes without running the complete command sequence from that guide.

- Format code: `./codeformat.sh`
- Sync dependencies: `./sync_deps.sh`

**Key Files**
- Core renderer: `src/SeatCanvas/core/renderer/SeatCanvasCoreRenderer.{hpp,cpp}`
- Platform views: `src/SeatCanvas/platform/{ios,android,ohos}/`
- Style system: `src/SeatCanvas/core/style/`
- Gesture handling: `src/SeatCanvas/core/gesture/ElasticZoomPanController.{hpp,cpp}`
- Render passes: `src/SeatCanvas/core/renderer/pass/`

## ⚠️ Critical Development Rules

1. **Design First**: For new features, output key interfaces and pseudocode first, ask ALL questions, get confirmation before coding
2. **No Documentation**: Don't generate README/docs unless explicitly requested
3. **Code Reuse**: Reuse existing project functionality, keep changes minimal, avoid duplicate code
4. **No Backward Compatibility**: When refactoring, review and clean up redundant code without backward compatibility hacks
5. **Variable Init**: ALL variables must be initialized at declaration (even `= {}`), smart pointers initialized with `nullptr`
6. **Function Order**: Implementation order in .cpp files should match header declaration order whenever possible

## Project Overview

SeatCanvas is a cross-platform seat map rendering library (iOS, Android, OHOS). It provides GPU-accelerated seat map rendering, interactive gesture handling, and customizable style configuration.

## Directory Structure

```
SeatCanvas/
├── src/SeatCanvas/              # C++ core code
│   ├── core/                    # Core modules
│   │   ├── renderer/            # Renderer + render passes
│   │   ├── gesture/             # Gesture handling
│   │   ├── style/               # Style configuration
│   │   ├── layers/              # Layer management
│   │   ├── parser/              # Basemap parsers
│   │   ├── animation/           # Animation system
│   │   └── drawers/             # Drawers
│   ├── platform/                # Platform-specific implementations
│   │   ├── ios/
│   │   ├── android/
│   │   ├── ohos/
│   │   └── apple/swift/         # Swift bridging (iOS)
├── ios/                         # iOS sample app
├── android/                     # Android sample app
├── ohos/                        # OHOS sample app
├── resources/                   # SVGBaseMap.bundle test resources
├── third_party/                 # Third-party dependencies
├── docs/                        # Detailed documentation
└── CMakeLists.txt
```

## Documentation Index

| Topic | File |
|-------|------|
| Build commands, dependencies, formatting | @./docs/agents/build.md |
| Architecture, core components, render pipeline, APIs | @./docs/agents/architecture.md |
| Platform integration (iOS / Android / OHOS) | @./docs/agents/platform.md |
| C++ coding standards, naming, commit format | @./docs/agents/coding-standards.md |
| Adding features, common tasks | @./docs/agents/development-guide.md |
| Layer version implementation | @./docs/layer_version_implementation.md |
