# Development Guide

## Adding New Seat Styles

1. Inherit from `SeatStyleConfig`
2. Implement `SeatStyleRenderer`
3. Add style key to `SeatStyleKey`
4. Add JSON support in `SeatStyleConfigJSONHelper`
5. Register with renderer

## Adding New Basemap Formats

1. Implement `IBaseMapParser`
2. Add enum to `BaseMapFormat`
3. Register in `BaseMapParserFactory`
4. Implement parse logic returning `BaseMapParseResult`

## Custom Render Passes

1. Inherit from `CustomRenderPass`
2. Implement `onDraw()`
3. Implement shader methods
4. Implement `outputImage()`
5. Integrate in `SeatCanvasCoreRenderer`

## Common Tasks

### Loading a Seat Map

```
setDelegate() before loadBaseMap()
→ loadBaseMap() / setBaseMapConfig()
→ didLoadBaseMap: registerPricecodes → applySeatStyleJSONConfig → setSeatData/updateSeats per zone → updateSeatStatusesForZone → setSelectedSeatIds
→ didUnloadBaseMap: stop polling, clear cached seat state
```

### Handling Seat Tap

```
Implement SeatCanvasCoreRendererDelegate
→ setDelegate() before loadBaseMap()
→ didTapSeat: update business state → updateSelectedSeatIds(added, removed) when selection changes
→ handleTap() → getSeatRegionDataByPoint() to find region
```

### Custom Seat Styles

```
Prepare style JSON
→ setStyleKeyToConfigFromJSON()
→ Auto-updates texture atlas
→ Styles applied via SeatStyleKey
```

### Region Zoom

```
zoomToRect()           → zoom to region (with animation params)
getVisibleOriginalRect() → get visible area
```

### Zoom and seat visibility

`ZoomLevelConfig` sets scale thresholds for when seats, row labels, zone labels, and venue labels are shown. Call `setBaseMapConfig()` with a prepared `BaseMapConfig`, then tune zoom range and `ZoomLevelConfig` as needed.
