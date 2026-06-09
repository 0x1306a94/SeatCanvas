//
//  index.ts
//  SeatCanvas
//
//  Created by king on 2026/5/26.
//

import { SeatCanvasInit, SeatCanvasApp, SeatCanvasFont, types } from '../src/SeatCanvas';
import type { SeatCanvasRenderer, SeatData, SVGBaseMapParseConfig } from '../src/types';
import { updateCanvasSize } from '../src/common';

const RESOURCES_BASE = 'sample_resources/default';

// MARK: - Types

interface PriceData {
    color: string;
    name: string;
    code: string;
    zoneIds: string[];
}

interface SeatEntry {
    seatId: string;
    pricecode: string;
    y: number;
    x: number;
    rotation?: number;
    selected?: boolean;
}

// MARK: - State

let renderer: SeatCanvasRenderer;
let priceList: PriceData[] = [];
let seatsMap: Record<string, SeatEntry> = {};
let seatZoneMap: Record<string, SeatEntry[]> = {};
let pricecodeMap: Record<string, number> = {};
let availableSeats: Record<string, Set<string>> = {};
let selectedSeatIds: Set<string> = new Set();
let availableSeatsTimer: number | null = null;
let availableSeatsTimerPaused = false;
const AVAILABLE_SEATS_REFRESH_INTERVAL = 10000;

// MARK: - Utilities

function setStatus(text: string) {
    const el = document.getElementById('status-display');
    if (el) el.textContent = 'Status: ' + text;
}

async function fetchText(url: string): Promise<string> {
    const resp = await fetch(url);
    if (!resp.ok) throw new Error(`HTTP ${resp.status} for ${url}`);
    return resp.text();
}

async function fetchJSON<T>(url: string): Promise<T> {
    const resp = await fetch(url);
    if (!resp.ok) throw new Error(`HTTP ${resp.status} for ${url}`);
    return resp.json();
}

async function fetchBasemapList(): Promise<string[]> {
    const resp = await fetch('/sample_resources/basemaps.json');
    if (!resp.ok) throw new Error(`HTTP ${resp.status}`);
    return resp.json();
}

async function fetchFontBytes(url: string): Promise<Uint8Array> {
    const resp = await fetch(url);
    if (!resp.ok) throw new Error(`HTTP ${resp.status} for ${url}`);
    return new Uint8Array(await resp.arrayBuffer());
}

async function registerDemoFonts(): Promise<void> {
    const textFontData = await fetchFontBytes('fonts/NotoSansSC-Regular.otf')
    SeatCanvasFont.registerFonts(textFontData);
}

// MARK: - Data Loading

async function loadPriceData(baseFileName: string): Promise<PriceData[]> {
    const path = `${RESOURCES_BASE}/zonedata/${baseFileName}_pricecode.json`;
    try {
        return await fetchJSON<PriceData[]>(path);
    } catch (e: any) {
        console.warn('[SeatCanvas] No price data found:', e.message);
        return [];
    }
}

async function loadSeatData(baseFileName: string): Promise<Record<string, SeatEntry[]>> {
    const path = `${RESOURCES_BASE}/seatdata/${baseFileName}.json`;
    try {
        return await fetchJSON<Record<string, SeatEntry[]>>(path);
    } catch (e: any) {
        console.warn('[SeatCanvas] No seat data found:', e.message);
        return {};
    }
}

let cachedIcons: { selectable: string; selected: string; nonselectable: string } | null = null;

async function loadSeatIcons() {
    if (cachedIcons) return cachedIcons;
    const base = `${RESOURCES_BASE}/seatstyle`;
    cachedIcons = {
        selectable: await fetchText(`${base}/icon_seat_selectable.svg`),
        selected: await fetchText(`${base}/icon_seat_selected.svg`),
        nonselectable: await fetchText(`${base}/icon_seat_nonselectable.svg`),
    };
    return cachedIcons;
}

// MARK: - Style Config

function composeSeatStyleId(pricecode: string, status: number, selected: boolean): string {
    return `pricecode_${pricecode}_status_${status}_selected_${selected ? 1 : 0}`;
}

function toRGBHex(argbHex: string): string {
    // Convert #AARRGGBB to #RRGGBB
    if (argbHex.length === 9) {
        return '#' + argbHex.substring(3);
    }
    return argbHex;
}

function buildSVGSeatStyleConfig(
    icons: { selectable: string; selected: string; nonselectable: string },
    prices: PriceData[],
): string {
    const configs: { key: string; config: { type: number; content: string } }[] = [];
    if (prices.length === 0) return JSON.stringify(configs);

    for (const price of prices) {
        const code = price.code;
        const rgbHex = toRGBHex(price.color);
        // Unavailable (status=0, selected=false)
        configs.push({
            key: composeSeatStyleId(code, 0, false),
            config: { type: 1, content: icons.nonselectable },
        });
        // Available (status=1, selected=false) — replace color placeholder
        const availabeSvg = icons.selectable.replace(/#EB484A/g, rgbHex);
        configs.push({
            key: composeSeatStyleId(code, 1, false),
            config: { type: 1, content: availabeSvg },
        });
        // Selected (status=1, selected=true) — replace color placeholder
        const selectedSvg = icons.selected.replace(/#5BC64D/g, rgbHex);
        configs.push({
            key: composeSeatStyleId(code, 1, true),
            config: { type: 1, content: selectedSvg },
        });
    }
    return JSON.stringify(configs);
}

// MARK: - Seat Status

function buildStatusesForZone(zoneId: string, seats: SeatEntry[]): Uint32Array {
    const statuses = new Uint32Array(seats.length);
    const zoneAvailable = availableSeats[zoneId];
    for (let i = 0; i < seats.length; i++) {
        const available = zoneAvailable ? zoneAvailable.has(seats[i].seatId) : false;
        statuses[i] = available ? 1 : 0;
    }
    return statuses;
}

function pushSelectedSeatIds(previousSelected: Set<string>) {
    const added = [...selectedSeatIds].filter(id => !previousSelected.has(id));
    const removed = [...previousSelected].filter(id => !selectedSeatIds.has(id));
    renderer.updateSelectedSeatIds(added, removed);
}

function seatAvailable(zoneId: string, seatId: string): boolean {
    return availableSeats[zoneId]?.has(seatId) ?? false;
}

function pruneSelectedSeatsForAvailability() {
    const toRemove: string[] = [];
    for (const seatId of selectedSeatIds) {
        let found = false;
        for (const [zoneId, seats] of Object.entries(seatZoneMap)) {
            if (seats.some(s => s.seatId === seatId)) {
                if (!seatAvailable(zoneId, seatId)) {
                    toRemove.push(seatId);
                }
                found = true;
                break;
            }
        }
        if (!found) toRemove.push(seatId);
    }
    for (const id of toRemove) {
        selectedSeatIds.delete(id);
    }
}

// MARK: - Random Availability

function regenerateRandomAvailableSeats(fullRefresh: boolean = false) {
    if (fullRefresh) {
        availableSeats = {};
    }

    for (const [zoneId, seats] of Object.entries(seatZoneMap)) {
        if (seats.length === 0) continue;
        const ratio = 0.2 + Math.random() * 0.7;
        const count = Math.max(1, Math.floor(seats.length * ratio));
        const shuffled = [...seats].sort(() => Math.random() - 0.5).slice(0, count);
        availableSeats[zoneId] = new Set(shuffled.map(s => s.seatId));
        renderer.updateSeatStatusesForZone(zoneId, buildStatusesForZone(zoneId, seats));
    }
    pruneSelectedSeatsForAvailability();
    renderer.setSelectedSeatIds([...selectedSeatIds]);
}

function startAvailableSeatsTimer() {
    availableSeatsTimerPaused = false;
    availableSeatsTimer = window.setInterval(() => {
        if (availableSeatsTimerPaused) return;
        regenerateRandomAvailableSeats();
    }, AVAILABLE_SEATS_REFRESH_INTERVAL);
}

function stopAvailableSeatsTimer() {
    if (availableSeatsTimer !== null) {
        clearInterval(availableSeatsTimer);
        availableSeatsTimer = null;
    }
    availableSeatsTimerPaused = false;
}

// MARK: - Basemap

async function loadBasemap(basemapName: string) {
    stopAvailableSeatsTimer();
    availableSeats = {};
    selectedSeatIds.clear();
    priceList = [];
    seatsMap = {};
    seatZoneMap = {};
    pricecodeMap = {};

    setStatus('Loading ' + basemapName + '...');
    try {
        const svgData = await fetchText(`${RESOURCES_BASE}/basemap/${basemapName}`);
        const parseConfig: SVGBaseMapParseConfig = { zoneIdAttributeNames: ["zoneId", "regioncode"] };
        const loaded = renderer.loadBaseMapFromSVG(svgData, JSON.stringify(parseConfig));
        if (!loaded) {
            throw new Error('Failed to parse SVG basemap');
        }
        console.log('[SeatCanvas] SVG data sent to renderer:', basemapName);
    } catch (e: any) {
        console.error('[SeatCanvas] Error loading SVG:', e);
        setStatus('Error: ' + e.message);
    }
}

// MARK: - Mock Data (matching iOS loadMockData)

async function loadMockData(baseFileName: string) {
    // Load price data and set zone colors
    priceList = await loadPriceData(baseFileName);
    const zoneColors: Record<string, string> = {};
    for (const price of priceList) {
        for (const zoneId of price.zoneIds) {
            zoneColors[zoneId] = price.color;
        }
    }

    if (Object.keys(zoneColors).length > 0) {
        try {
            renderer.updateSeatZoneAlternateColors(zoneColors);
            renderer.updateMiniMapZoneAlternateColors(zoneColors);
        } catch (_) {
            // API not available until WASM is rebuilt
        }
    }

    // Register pricecodes
    const codes = priceList.map(p => p.code);
    pricecodeMap = {};
    codes.forEach((code, idx) => { pricecodeMap[code] = idx; });
    if (codes.length > 0) {
        renderer.registerPricecodes(codes);
    }

    // Clear flat map and selection
    seatsMap = {};
    selectedSeatIds.clear();

    // Load seat data and push to renderer
    seatZoneMap = await loadSeatData(baseFileName);
    for (const [zoneId, seats] of Object.entries(seatZoneMap)) {
        for (const seat of seats) {
            seatsMap[seat.seatId] = seat;
            if (seat.selected) {
                selectedSeatIds.add(seat.seatId);
            }
        }

        const seatList: SeatData[] = seats.map(seat => {
            const data: SeatData = {
                seatId: seat.seatId,
                x: seat.x,
                y: seat.y,
                rotation: seat.rotation ?? 0,
                pricecode: seat.pricecode,
            };
            return data;
        });
        renderer.setSeatData(zoneId, seatList);
        renderer.updateSeatStatusesForZone(zoneId, buildStatusesForZone(zoneId, seats));
    }
    renderer.setSelectedSeatIds([...selectedSeatIds]);
}

// MARK: - Delegate Callback

function makeDidLoadBaseMapCallback(getCurrentBaseFileName: () => string) {
    return async () => {
        const baseFileName = getCurrentBaseFileName();
        console.log('[SeatCanvas] Base map loaded, loading companion data...');
        setStatus('Loading companion data...');

        try {
            await loadMockData(baseFileName);
            regenerateRandomAvailableSeats(true);
            const icons = await loadSeatIcons();
            if (priceList.length > 0) {
                const styleConfig = buildSVGSeatStyleConfig(icons, priceList);
                renderer.applySeatStyleJSONConfig(styleConfig);
            }
            if (availableSeatsTimer === null) {
                startAvailableSeatsTimer();
            }
            renderer.invalidateContent();
            console.log('[SeatCanvas] Companion data loaded, zones:', Object.keys(seatZoneMap).length);
        } catch (e: any) {
            console.error('[SeatCanvas] Error loading companion data:', e);
            setStatus('Error: ' + e.message);
        }
    };
}

// MARK: - Main

if (typeof window !== 'undefined') {
    window.onload = async () => {
        const canvas = document.getElementById('seat-canvas') as HTMLCanvasElement;
        if (!canvas) {
            console.error('Canvas element #seat-canvas not found');
            return;
        }

        updateCanvasSize(canvas);

        let app: SeatCanvasApp | null = null;

        try {
            const module = await SeatCanvasInit({
                locateFile: (file: string) => './wasm/' + file,
            }) as types.SeatCanvasModule;

            await registerDemoFonts();
            app = new SeatCanvasApp('#seat-canvas');
            app.init(module);
            console.log('[SeatCanvas] Initialized successfully');
        } catch (err) {
            console.error('[SeatCanvas] Failed to initialize:', err);
            setStatus('Init failed');
            return;
        }

        renderer = app.getRenderer()!;
        renderer.setSeatSize(24);
        renderer.setDebugHUDEnabled(true);

        const delegate = app.getDelegate()!;

        let currentBasemapName = '';
        let currentBaseFileName = '';

        // Delegate: base map loaded
        delegate.setDidLoadBaseMapCallback(
            makeDidLoadBaseMapCallback(() => currentBaseFileName),
        );

        // Delegate: base map unloaded
        delegate.setDidUnloadBaseMapCallback(() => {
            stopAvailableSeatsTimer();
            availableSeats = {};
            selectedSeatIds.clear();
        });

        // Delegate: zoom level config
        delegate.setDidUpdateZoomLevelConfigCallback((event) => {
            renderer.setSeatRenderZoomThreshold(event.zoomLevels.venue);
        });

        // Delegate: tap zone
        delegate.setDidTapZoneCallback((zoneId: string) => {
            console.log('[SeatCanvas] Tapped zone:', zoneId);
        });

        // Delegate: tap seat — toggle selection
        delegate.setDidTapSeatCallback((zoneId: string, seatId: string) => {
            const previousSelected = new Set(selectedSeatIds);
            let changed = false;
            if (selectedSeatIds.has(seatId)) {
                selectedSeatIds.delete(seatId);
                changed = true;
            } else if (seatAvailable(zoneId, seatId)) {
                selectedSeatIds.add(seatId);
                changed = true;
            }
            if (changed) {
                pushSelectedSeatIds(previousSelected);
            }
            console.log('[SeatCanvas] Tapped seat:', seatId, 'in zone:', zoneId);
            return changed;
        });

        app.start();

        // Populate basemap selector
        try {
            const basemaps = await fetchBasemapList();
            const select = document.getElementById('basemap-select') as HTMLSelectElement;
            basemaps.forEach(name => {
                const option = document.createElement('option');
                option.value = name;
                option.textContent = name;
                select.appendChild(option);
            });
            if (basemaps.length > 0) {
                select.value = basemaps[0];
            }
        } catch (e) {
            console.error('[SeatCanvas] Failed to fetch basemap list:', e);
            setStatus('Failed to load basemap list');
        }

        // Load button handler
        const loadBtn = document.getElementById('load-btn');
        if (loadBtn) {
            loadBtn.addEventListener('click', () => {
                const select = document.getElementById('basemap-select') as HTMLSelectElement;
                const basemapName = select.value;
                if (!basemapName) return;
                currentBasemapName = basemapName;
                currentBaseFileName = basemapName.replace(/\.svg$/, '');
                loadBasemap(basemapName);
            });
        }

        // FPS display
        setInterval(() => {
            const fps = renderer.getFPS();
            const el = document.getElementById('fps-display');
            if (el) el.textContent = `FPS: ${fps.toFixed(1)}`;
        }, 500);

        console.log('[SeatCanvas] Ready');
    };
}
