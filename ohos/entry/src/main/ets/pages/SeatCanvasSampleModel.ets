import {
  SeatCanvasRendererDelegate,
  SeatCanvasViewController,
  SeatCanvasViewport,
  SeatData,
  SeatZoneColor
} from 'libseatcanvas';
import { BaseMapFileInfo } from './BaseMapFileInfo';
import { ResourceExtensions } from './ResourceExtensions';
import { MockPriceData, parseMockPriceDataList } from './MockPriceData';
import { MockSeatData } from './MockSeatData';

export class SeatCanvasSampleModel {
  prices: MockPriceData[] = [];
  seatsMap: Map<string, MockSeatData> = new Map();
  seatZoneMap: Record<string, MockSeatData[]> = {};
  availableSeats: Map<string, Set<string>> = new Map();
  selectedSeatIds: Set<string> = new Set();
  private availableSeatsTimer: number = -1;
  private controller: WeakRef<SeatCanvasViewController> | null = null;
  private static readonly REFRESH_INTERVAL_MS: number = 10 * 1000;
  /** Controller 以 WeakRef 持有 delegate，此处强引用避免被回收 */
  readonly rendererDelegate: SeatCanvasRendererDelegate;

  constructor() {
    this.rendererDelegate = {
      styleIdForSeat: (zoneId: string, seatId: string): string | null => {
        return this.onStyleIdForSeat(zoneId, seatId);
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

  getDelegate(): SeatCanvasRendererDelegate {
    return this.rendererDelegate;
  }

  styleId(pricecode: string, available: boolean, selected: boolean): string {
    return `pricecode_${pricecode}_available_${available}_selected_${selected ? 1 : 0}`;
  }

  private onStyleIdForSeat(zoneId: string, seatId: string): string | null {
    const seat = this.seatsMap.get(seatId);
    if (seat === undefined) {
      return null;
    }
    const available = this.seatAvailable(zoneId, seatId);
    return this.styleId(seat.pricecode, available, this.selectedSeatIds.has(seatId));
  }

  private onDidTapSeat(zoneId: string, seatId: string): boolean {
    let changed = false;
    if (this.selectedSeatIds.has(seatId)) {
      this.selectedSeatIds.delete(seatId);
      changed = true;
    } else if (this.seatAvailable(zoneId, seatId)) {
      this.selectedSeatIds.add(seatId);
      changed = true;
    }
    console.log(`didTapSeat: ${zoneId} ${seatId}`);
    return changed;
  }

  private onDidTapZone(zoneId: string): void {
    console.log(`didTapZone: ${zoneId}`);
  }

  private onWillBeginDragging(viewport: SeatCanvasViewport): void {
    console.log(`willBeginDragging: ${JSON.stringify(viewport)}`);
  }

  private onDidScroll(viewport: SeatCanvasViewport): void {
    console.log(`didScroll: ${JSON.stringify(viewport)}`);
  }

  private onDidEndDragging(viewport: SeatCanvasViewport, decelerate: boolean): void {
    console.log(`didEndDragging: ${JSON.stringify(viewport)} decelerate: ${decelerate}`);
    if (!decelerate) {
      this.scheduleRefreshForVisibleSeats();
    }
  }

  private onDidEndDecelerating(viewport: SeatCanvasViewport): void {
    console.log(`didEndDecelerating: ${JSON.stringify(viewport)}`);
    this.scheduleRefreshForVisibleSeats();
  }

  private onWillBeginZooming(viewport: SeatCanvasViewport): void {
    console.log(`willBeginZooming: ${JSON.stringify(viewport)}`);
  }

  private onDidZoom(viewport: SeatCanvasViewport): void {
    console.log(`didZoom: ${JSON.stringify(viewport)}`);
  }

  private onDidEndZooming(viewport: SeatCanvasViewport): void {
    console.log(`didEndZooming: ${JSON.stringify(viewport)}`);
    this.scheduleRefreshForVisibleSeats();
  }

  private onDidEndScrollingAnimation(viewport: SeatCanvasViewport): void {
    console.log(`didEndScrollingAnimation: ${JSON.stringify(viewport)}`);
    this.scheduleRefreshForVisibleSeats();
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

    this.seatsMap.clear();
    this.seatZoneMap = this.loadSeatDatas(baseMap, readRawFile);
    this.selectedSeatIds.clear();
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
      }));
      controller.updateSeats(zoneId, seatDataArray);
    }
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

  regenerateRandomAvailableSeats(): void {
    this.availableSeats.clear();
    for (const zoneId of Object.keys(this.seatZoneMap)) {
      const seats = this.seatZoneMap[zoneId];
      if (seats.length === 0) {
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
    }
    this.pruneSelectedSeatsForAvailability();
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
    this.availableSeatsTimer = setInterval(() => {
      this.regenerateRandomAvailableSeats();
    }, SeatCanvasSampleModel.REFRESH_INTERVAL_MS);
  }

  stopAvailableSeatsTimer(): void {
    if (this.availableSeatsTimer >= 0) {
      clearInterval(this.availableSeatsTimer);
      this.availableSeatsTimer = -1;
    }
  }

  detachSeatCanvasController(): void {
    this.controller = null;
  }

  private scheduleRefreshForVisibleSeats(): void {
    const controller = this.controller?.deref();
    if (!controller) {
      return;
    }

    /*
     * 1. 判断当前是否是显示座位级别
     * 这里只判断了缩放级别，实际业务场景可能还有其他条件
     */
    const zoomScale = controller.zoomScale;
    const seatRenderZoomThreshold = controller.seatRenderZoomThreshold;
    if (zoomScale < seatRenderZoomThreshold) {
      return;
    }

    /*
     * 2. 获取当前可视范围内的区域ID
     * 2.1 刷新指定区域ID内的座位状态
     *
     * 根据实际业务场景调整
     */
    const visibleRect = controller.visibleOriginalRect();
    const zoneIds = controller.zoneIdsInOriginalRect(visibleRect);
    if (zoneIds.length === 0) {
      return;
    }

    console.log(`scheduleRefreshForVisibleSeats: zoneIds=${JSON.stringify(zoneIds)}`);
  }
}
