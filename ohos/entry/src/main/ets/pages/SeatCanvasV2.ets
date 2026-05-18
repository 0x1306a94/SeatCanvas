import {
  SeatCanvasViewV2,
  SeatCanvasViewController,
  BaseMapFormat,
  SeatCanvasRendererDelegate,
  SeatData,
  SeatZoneColor,
  SVGBaseMapParseConfig,
  SeatCanvasViewport
} from 'libseatcanvas';
import { SeatStyleBuilder } from './SeatStyleBuilder';
import { resourceManager } from '@kit.LocalizationKit';
import { util } from '@kit.ArkTS';
import { BaseMapFileInfo } from './BaseMapFileInfo';
import { ResourceExtensions } from './ResourceExtensions';
import { MockSeatData } from './MockSeatData';
import { MockZoneInfo } from './MockZoneInfo';

@ComponentV2
export struct SeatCanvasV2 {
  private navPathStack: NavPathStack = new NavPathStack();
  @Local private baseMap: BaseMapFileInfo | null = null;
  @Local private controller: SeatCanvasViewController = new SeatCanvasViewController();
  @Local useCircleSeatStyle: boolean = false;
  @Local useSVGSeatStyle: boolean = true;
  private delegate: SeatCanvasRendererDelegate | null = null;
  private seatStatusMap: Map<string, number> = new Map();
  private selectedSeatIds: Set<string> = new Set();

  private sampleStyleId(status: number, selected: boolean): string {
    return `status_${status}_selected_${selected ? 1 : 0}`;
  }

  aboutToAppear(): void {
    this.delegate = {
      styleIdForSeat: (zoneId: string, seatId: string): string | null => {
        const status = this.seatStatusMap.get(seatId);
        if (status === undefined) {
          return null;
        }
        return this.sampleStyleId(status, this.selectedSeatIds.has(seatId));
      },
      didTapSeat: (zoneId: string, seatId: string): boolean => {
        if (this.selectedSeatIds.has(seatId)) {
          this.selectedSeatIds.delete(seatId);
        } else {
          this.selectedSeatIds.add(seatId);
        }
        console.log(`tgfx didTapSeat: ${zoneId} ${seatId}`);
        return true;
      },
      didTapZone: (zoneId: string) => {
        console.log(`tgfx didTapZone: ${zoneId}`)
      },
      willBeginDragging: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx willBeginDragging: ${JSON.stringify(viewport)}`)
      },
      didScroll: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx didScroll: ${JSON.stringify(viewport)}`)
      },
      didEndDragging: (viewport: SeatCanvasViewport, decelerate: boolean) => {
        console.log(`tgfx didEndDragging: ${JSON.stringify(viewport)} decelerate: ${decelerate}`)
      },
      didEndDecelerating: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx didEndDecelerating: ${JSON.stringify(viewport)}`)
      },
      willBeginZooming: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx willBeginZooming: ${JSON.stringify(viewport)}`)
      },
      didZoom: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx didZoom: ${JSON.stringify(viewport)}`)
      },
      didEndZooming: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx didEndZooming: ${JSON.stringify(viewport)}`)
      },
      didEndScrollingAnimation: (viewport: SeatCanvasViewport) => {
        console.log(`tgfx didEndScrollingAnimation: ${JSON.stringify(viewport)}`)
      },
    };

    this.controller.setDelegate(this.delegate);
    this.controller.canvasColor = 0xFFF8F8F8;
  }

  aboutToDisappear(): void {
    this.controller.release();
  }

  applyCircleSeatStyleConfig(): void {
    let config = SeatStyleBuilder.BuildCircleSeatStyleConfig();
    this.controller.applySeatStyleJSONConfig(config);
  }

  applySVGSeatStyleConfig(): void {
    let manager = getContext(this).resourceManager;
    let config = SeatStyleBuilder.BuildSVGSeatStyleConfig(manager);
    this.controller.applySeatStyleJSONConfig(config);
  }

  loadMockData() {
    if (!this.baseMap) {
      return;
    }
    const colors = this.loadZoneColors();
    this.controller.updateSeatZoneAlternateColors(colors);

    this.seatStatusMap.clear();
    this.selectedSeatIds.clear();

    const seatDatas = this.loadSeatDatas();
    for (const zoneId of Object.keys(seatDatas)) {
      const seats = seatDatas[zoneId];
      for (const mockSeat of seats) {
        this.seatStatusMap.set(mockSeat.seatId, mockSeat.status);
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
      this.controller.updateSeats(zoneId, seatDataArray);
    }
  }

  loadZoneColors(): SeatZoneColor[] {
    if (!this.baseMap) {
      return [];
    }
    const path = ResourceExtensions.zoneDataPath(this.baseMap.scope, this.baseMap.filename);
    const jsonString = this.readRawFileToString(path);
    if (!jsonString) {
      return [];
    }
    try {
      const zones: MockZoneInfo[] = JSON.parse(jsonString);
      return zones.map((zone: MockZoneInfo): SeatZoneColor => ({
        zoneId: zone.zoneId,
        alternateColor: zone.alternateColor,
      }));
    } catch (e) {
      console.error('Failed to parse zone data:', e);
      return [];
    }
  }

  loadSeatDatas(): Record<string, MockSeatData[]> {
    if (!this.baseMap) {
      return {};
    }
    const path = ResourceExtensions.seatDataPath(this.baseMap.scope, this.baseMap.filename);
    const jsonString = this.readRawFileToString(path);
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

  readRawFileToString(fileName: string): string | null {
    try {
      const manager = getContext(this).resourceManager;
      const rawFileContent = manager.getRawFileContentSync(fileName);
      if (rawFileContent == null) {
        console.error(`Failed to load file: ${fileName}`);
        return null;
      }
      const textDecoderOptions: util.TextDecoderOptions = {
        fatal: false,
        ignoreBOM: true
      };
      const decodeToStringOptions: util.DecodeToStringOptions = {
        stream: false
      };
      const textDecoder = util.TextDecoder.create('utf-8', textDecoderOptions);
      return textDecoder.decodeToString(rawFileContent, decodeToStringOptions);
    } catch (e) {
      console.error(`Error loading file ${fileName}:`, e);
      return null;
    }
  }

  build() {
    NavDestination() {
      Column() {
        SeatCanvasViewV2({ controller: this.controller })
          .width('100%')
          .layoutWeight(1)

        Row({ space: 20 }) {
          Row() {
            Text("圆形座位")
              .fontSize(18)

            Toggle({ type: ToggleType.Switch, isOn: $$this.useCircleSeatStyle })
              .onChange((isOn) => {
                if (isOn) {
                  this.useSVGSeatStyle = false
                  this.applyCircleSeatStyleConfig()
                }
              })
          }

          Row() {
            Text("SVG座位")
              .fontSize(18)

            Toggle({ type: ToggleType.Switch, isOn: $$this.useSVGSeatStyle })
              .onChange(async (isOn) => {
                if (isOn) {
                  this.useCircleSeatStyle = false
                  this.applySVGSeatStyleConfig();
                }
              })
          }
        }
        .justifyContent(FlexAlign.Center)
        .alignItems(VerticalAlign.Center)
        .width('100%')
        .height(60)
      }
      .width('100%')
      .height('100%')
    }
    .title(this.baseMap?.filename || '')
    .onReady(async (ctx) => {
      this.navPathStack = ctx.pathStack
      this.baseMap = ctx.pathInfo.param as BaseMapFileInfo

      if (!this.baseMap) {
        return;
      }

      let manager = getContext(this).resourceManager;
      this.applySVGSeatStyleConfig();
      let parseConfig = new SVGBaseMapParseConfig(['zoneId', 'regioncode'])
      await this.controller.loadFromAssets(manager, this.baseMap.assetPath, BaseMapFormat.SVG, parseConfig);
      console.log(`底图: ${this.baseMap.assetPath} 加载成功`);
      this.loadMockData();
    })
  }
}