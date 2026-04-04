# Coding Standards

## Commit Message Format

- **Length**: Within 120 characters
- **Language**: English only
- **Ending**: Must end with period (.)
- **Focus**: User-perceivable changes, not implementation details
- **Format**: `<verb> <what> <context>.`

**Good**:
```
Add minimap fade animation for better visual feedback.
Fix seat selection crash when tapping outside region bounds.
Optimize GPU rendering by reducing draw calls with instancing.
```

**Bad**:
```
update code          ← too vague
修复bug              ← wrong language
Add feature, fix bug ← multiple changes, has comma
```

## Naming Conventions

| Element | Convention | Example |
|---------|------------|---------|
| Classes | PascalCase | `SeatCanvasCoreRenderer` |
| Member methods/variables | camelCase (lowercase start) | `setBaseMapConfig`, `zoomScale` |
| Static methods, global functions/vars | camelCase (uppercase start) | `GetInstance` |
| Constants | UPPER_SNAKE_CASE | `MINIMAP_FADE_DURATION_MS` |
| Namespaces | lowercase, nestable | `namespace kk::renderer` |
| Enums | PascalCase | `enum class GestureState` |

Variable naming: avoid abbreviations, keep short and semantically clear.

## Code Style

- **Variable Init**: Always initialize at declaration (even `= {}`), smart pointers with `nullptr`
- **Function Order**: `.cpp` implementation order should match header declaration order
- **Smart Pointers**: Prefer for memory management
- **Const Correctness**: Use `const` wherever possible
- **Move Semantics**: Prefer for large objects
- **Memory Optimization**: Use `reserve` for vectors when size is known

## Comment Standards

| Scope | Requirement |
|-------|-------------|
| `include/` directory APIs | Detailed comments with parameter descriptions |
| Other public methods | One-sentence description |
| Private methods | No comments required |
| In-function code | Only if design intent cannot be understood from code |

Use Doxygen style (`///` or `/** */`).

## Code Formatting

Run `./codeformat.sh` before committing. Uses clang-format 14.x for C++ and swiftformat for Swift.
