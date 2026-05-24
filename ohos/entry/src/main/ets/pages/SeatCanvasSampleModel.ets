import {
  SeatCanvasRendererDelegate,
  SeatCanvasViewController,
  SeatCanvasViewport,
  SeatCanvasZoomLevelConfigEvent,
  SeatData,
  SeatZoneColor
} from 'libseatcanvas';
import { BaseMapFileInfo } from './BaseMapFileInfo';
import { ResourceExtensions } from './ResourceExtensions';
import { MockPriceData, parseMockPriceDataList } from './MockPriceData';
import { MockSeatData } from './MockSeatData';
import { SeatStatus } from './SeatStyleBuilder';
import { systemDateTime } from '@kit.BasicServicesKit';

export class SeatCanvasSampleModel {
  prices: MockPriceData[] = [];
  seatsMap: Map<string, MockSeatData> = new Map();
  seatZoneMap: Record<string, MockSeatData[]> = {};
  availableSeats: Map<string, Set<string>> = new Map();
  selectedSeatIds: Set<string> = new Set();
  private availableSeatsTimer: number = -1;
  private availableSeatsTimerPaused: boolean = false;
  private isViewportInteracting: boolean = false;
  private controller: WeakRef<SeatCanvasViewController> | null = null;
  private pendingBaseMap: BaseMapFileInfo | null = null;
  private pendingReadRawFile: ((path: string) => string | null) | null = null;
  private applySeatStyle: (() => void) | null = null;
  private static readonly REFRESH_INTERVAL_MS: number = 10 * 1000;
  readonly rendererDelegate: SeatCanvasRendererDelegate;

  constructor() {
    this.rendererDelegate = {
      didLoadBaseMap: (): void => {
        this.onDidLoadBaseMap();
      },
      didUnloadBaseMap: (): void => {
        this.onDidUnloadBaseMap();
      },
      didUpdateZoomLevelConfig: (event: SeatCanvasZoomLevelConfigEvent): void => {
        this.onDidUpdateZoomLevelConfig(event);
      },
      didTapSeat: (zoneId: string, seatId: string): boolean => {
        return this.onDidTapSeat(zoneId, seatId);
      },
      didTapZone: (zoneId: string): void => {
        this.onDidTapZone(zoneId);
      },
      willBeginDragging: (viewport: SeatCanvasViewport): void => {
        this.onWillBeginDragging(viewport);
      },
      didScroll: (viewport: SeatCanvasViewport): void => {
        this.onDidScroll(viewport);
      },
      didEndDragging: (viewport: SeatCanvasViewport, decelerate: boolean): void => {
        this.onDidEndDragging(viewport, decelerate);
      },
      didEndDecelerating: (viewport: SeatCanvasViewport): void => {
        this.onDidEndDecelerating(viewport);
      },
      willBeginZooming: (viewport: SeatCanvasViewport): void => {
        this.onWillBeginZooming(viewport);
      },
      didZoom: (viewport: SeatCanvasViewport): void => {
        this.onDidZoom(viewport);
      },
      didEndZooming: (viewport: SeatCanvasViewport): void => {
        this.onDidEndZooming(viewport);
      },
      didEndScrollingAnimation: (viewport: SeatCanvasViewport): void => {
        this.onDidEndScrollingAnimation(viewport);
      },
    };
  }

  prepareForBaseMap(
    controller: SeatCanvasViewController,
    baseMap: BaseMapFileInfo,
    readRawFile: (path: string) => string | null,
    applySeatStyle: () => void
  ): void {
    this.controller = new WeakRef(controller);
    this.pendingBaseMap = baseMap;
    this.pendingReadRawFile = readRawFile;
    this.applySeatStyle = applySeatStyle;
  }

  getDelegate(): SeatCanvasRendererDelegate {
    return this.rendererDelegate;
  }

  private onDidTapSeat(zoneId: string, seatId: string): boolean {
    const previousSelected = new Set(this.selectedSeatIds);
    let changed = false;
    if (this.selectedSeatIds.has(seatId)) {
      this.selectedSeatIds.delete(seatId);
      changed = true;
    } else if (this.seatAvailable(zoneId, seatId)) {
      this.selectedSeatIds.add(seatId);
      changed = true;
    }
    if (changed) {
      this.pushSelectedSeatIds(previousSelected);
    }
    console.log(`didTapSeat: ${zoneId} ${seatId}`);
    return changed;
  }

  private onDidTapZone(zoneId: string): void {
    console.log(`didTapZone: ${zoneId}`);
  }

  private onWillBeginDragging(viewport: SeatCanvasViewport): void {
    console.log(`willBeginDragging: ${JSON.stringify(viewport)}`);
    this.beginViewportInteraction();
  }

  private onDidScroll(viewport: SeatCanvasViewport): void {
    // console.log(`didScroll: ${JSON.stringify(viewport)}`);
  }

  private onDidEndDragging(viewport: SeatCanvasViewport, decelerate: boolean): void {
    console.log(`didEndDragging: ${JSON.stringify(viewport)} decelerate: ${decelerate}`);
    if (!decelerate) {
      this.endViewportInteraction();
    }
  }

  private onDidEndDecelerating(viewport: SeatCanvasViewport): void {
    console.log(`didEndDecelerating: ${JSON.stringify(viewport)}`);
    this.endViewportInteraction();
  }

  private onWillBeginZooming(viewport: SeatCanvasViewport): void {
    console.log(`willBeginZooming: ${JSON.stringify(viewport)}`);
    this.beginViewportInteraction();
  }

  private onDidZoom(viewport: SeatCanvasViewport): void {
    console.log(`didZoom: ${JSON.stringify(viewport)}`);
  }

  private onDidEndZooming(viewport: SeatCanvasViewport): void {
    console.log(`didEndZooming: ${JSON.stringify(viewport)}`);
    this.endViewportInteraction();
  }

  private onDidEndScrollingAnimation(viewport: SeatCanvasViewport): void {
    console.log(`didEndScrollingAnimation: ${JSON.stringify(viewport)}`);
    this.endViewportInteraction();
  }

  private onDidLoadBaseMap(): void {
    const controller = this.controller?.deref();
    const baseMap = this.pendingBaseMap;
    const readRawFile = this.pendingReadRawFile;
    if (!controller || !baseMap || !readRawFile) {
      return;
    }
    this.loadMockData(baseMap, controller, readRawFile);
    this.regenerateRandomAvailableSeats(true);
    this.applySeatStyle?.();
    this.startAvailableSeatsTimer();
  }

  private onDidUnloadBaseMap(): void {
    this.stopAvailableSeatsTimer();
    this.availableSeats.clear();
    this.selectedSeatIds.clear();
  }

  private onDidUpdateZoomLevelConfig(event: SeatCanvasZoomLevelConfigEvent): void {
    const controller = this.controller?.deref();
    if (controller) {
      controller.seatRenderZoomThreshold = event.zoomLevels.venue;
    }
  }

  private beginViewportInteraction(): void {
    if (this.isViewportInteracting) {
      return;
    }
    this.isViewportInteracting = true;
    this.pauseAvailableSeatsTimer();
  }

  private endViewportInteraction(): void {
    if (!this.isViewportInteracting) {
      return;
    }
    this.isViewportInteracting = false;
    this.regenerateRandomAvailableSeats(false);
    this.resumeAvailableSeatsTimer();
  }

  loadMockPrice(baseMap: BaseMapFileInfo | null, readRawFile: (path: string) => string | null): MockPriceData[] {
    if (!baseMap) {
      return [];
    }
    const path = ResourceExtensions.priceDataPath(baseMap.scope, baseMap.filename);
    const jsonString = readRawFile(path);
    if (!jsonString) {
      return [];
    }
    try {
      return parseMockPriceDataList(jsonString);
    } catch (e) {
      console.error('Failed to parse price data:', e);
      return [];
    }
  }

  loadMockData(
    baseMap: BaseMapFileInfo | null,
    controller: SeatCanvasViewController,
    readRawFile: (path: string) => string | null
  ): void {
    if (!baseMap) {
      return;
    }
    this.controller = new WeakRef(controller);
    this.prices = this.loadMockPrice(baseMap, readRawFile);
    const zoneColors: SeatZoneColor[] = [];
    for (const price of this.prices) {
      for (const zoneId of price.zoneIds) {
        zoneColors.push({ zoneId, alternateColor: price.color });
      }
    }
    controller.updateSeatZoneAlternateColors(zoneColors);
    controller.updateMiniMapZoneAlternateColors(zoneColors);

    controller.registerPricecodes(this.prices.map((price: MockPriceData): string => price.code));

    this.seatsMap.clear();
    this.selectedSeatIds.clear();

    this.seatZoneMap = this.loadSeatDatas(baseMap, readRawFile);
    for (const zoneId of Object.keys(this.seatZoneMap)) {
      const seats = this.seatZoneMap[zoneId];
      for (const mockSeat of seats) {
        this.seatsMap.set(mockSeat.seatId, mockSeat);
        if (mockSeat.selected) {
          this.selectedSeatIds.add(mockSeat.seatId);
        }
      }
      const seatDataArray: SeatData[] = seats.map((mockSeat: MockSeatData): SeatData => ({
        seatId: mockSeat.seatId,
        x: mockSeat.x,
        y: mockSeat.y,
        rotation: mockSeat.rotation,
        pricecode: mockSeat.pricecode.length > 0 ? mockSeat.pricecode : undefined,
      }));
      controller.updateSeats(zoneId, seatDataArray);
      controller.updateSeatStatusesForZone(zoneId, this.buildStatusesForZone(zoneId, seats));
    }
    controller.setSelectedSeatIds(Array.from(this.selectedSeatIds));
  }

  loadSeatDatas(
    baseMap: BaseMapFileInfo,
    readRawFile: (path: string) => string | null
  ): Record<string, MockSeatData[]> {
    const path = ResourceExtensions.seatDataPath(baseMap.scope, baseMap.filename);
    const jsonString = readRawFile(path);
    if (!jsonString) {
      return {};
    }
    try {
      return JSON.parse(jsonString);
    } catch (e) {
      console.error('Failed to parse seat data:', e);
      return {};
    }
  }

  seatAvailable(zoneId: string, seatId: string): boolean {
    const seats = this.availableSeats.get(zoneId);
    if (seats === undefined) {
      return false;
    }
    return seats.has(seatId);
  }

  regenerateRandomAvailableSeats(fullRefresh: boolean = false): void {
    const controller = this.controller?.deref();
    if (!controller) {
      return;
    }

    let zoneIds: string[] = [];
    if (fullRefresh) {
      zoneIds = Object.keys(this.seatZoneMap);
    } else {
      if (controller.zoomScale < controller.seatRenderZoomThreshold) {
        return;
      }
      const visibleRect = controller.visibleOriginalRect();
      zoneIds = controller.zoneIdsInOriginalRect(visibleRect);
      if (zoneIds.length === 0) {
        return;
      }
    }

    if (fullRefresh) {
      this.availableSeats.clear();
    }

    for (const zoneId of zoneIds) {
      const seats = this.seatZoneMap[zoneId];
      if (seats === undefined || seats.length === 0) {
        continue;
      }
      const ratio = 0.2 + Math.random() * 0.7;
      const availableCount = Math.max(1, Math.floor(seats.length * ratio));
      const shuffled = [...seats].sort(() => Math.random() - 0.5);
      const availableIds = new Set<string>();
      for (let i = 0; i < availableCount && i < shuffled.length; i++) {
        availableIds.add(shuffled[i].seatId);
      }
      this.availableSeats.set(zoneId, availableIds);
      controller.updateSeatStatusesForZone(zoneId, this.buildStatusesForZone(zoneId, seats));
    }
    this.pruneSelectedSeatsForAvailability();
    controller.setSelectedSeatIds(Array.from(this.selectedSeatIds));

    if (!fullRefresh) {
      console.log(`refreshAvailableSeatsForVisibleZones: zoneIds=${JSON.stringify(zoneIds)}`);
    }
  }

  private buildStatusesForZone(zoneId: string, seats: MockSeatData[]): ArrayBuffer {
    const availableIds = this.availableSeats.get(zoneId);
    const buffer = new ArrayBuffer(seats.length * 4);
    const view = new Uint32Array(buffer);
    for (let i = 0; i < seats.length; i++) {
      view[i] = availableIds?.has(seats[i].seatId) ? SeatStatus.AVAILABLE : SeatStatus.UNAVAILABLE;
    }
    return buffer;
  }

  private pushSelectedSeatIds(previousSelected: Set<string>): void {
    const controller = this.controller?.deref();
    if (!controller) {
      return;
    }
    const added: string[] = [];
    const removed: string[] = [];
    for (const seatId of this.selectedSeatIds) {
      if (!previousSelected.has(seatId)) {
        added.push(seatId);
      }
    }
    for (const seatId of previousSelected) {
      if (!this.selectedSeatIds.has(seatId)) {
        removed.push(seatId);
      }
    }
    controller.updateSelectedSeatIds(added, removed);
  }

  private pruneSelectedSeatsForAvailability(): void {
    const kept = new Set<string>();
    for (const seatId of this.selectedSeatIds) {
      for (const zoneId of Object.keys(this.seatZoneMap)) {
        const seats = this.seatZoneMap[zoneId];
        if (!seats.some((seat: MockSeatData) => seat.seatId === seatId)) {
          continue;
        }
        if (this.seatAvailable(zoneId, seatId)) {
          kept.add(seatId);
        }
        break;
      }
    }
    this.selectedSeatIds = kept;
  }

  startAvailableSeatsTimer(): void {
    this.stopAvailableSeatsTimer();
    this.availableSeatsTimerPaused = false;
    this.availableSeatsTimer = setInterval(() => {
      if (this.availableSeatsTimerPaused) {
        return;
      }
      this.regenerateRandomAvailableSeats(false);
    }, SeatCanvasSampleModel.REFRESH_INTERVAL_MS);
  }

  pauseAvailableSeatsTimer(): void {
    this.availableSeatsTimerPaused = true;
  }

  resumeAvailableSeatsTimer(): void {
    if (!this.availableSeatsTimerPaused) {
      return;
    }
    this.availableSeatsTimerPaused = false;
  }

  stopAvailableSeatsTimer(): void {
    this.availableSeatsTimerPaused = false;
    this.isViewportInteracting = false;
    if (this.availableSeatsTimer >= 0) {
      clearInterval(this.availableSeatsTimer);
      this.availableSeatsTimer = -1;
    }
  }

  detachSeatCanvasController(): void {
    this.controller = null;
  }
}
