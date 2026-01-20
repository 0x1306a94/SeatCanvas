import { BaseMapFormat, SeatCanvasRendererDelegate, SeatCanvasView, SeatCanvasViewController, SeatData, SeatZoneData } from 'libseatcanvas';
import { SeatStyleBuilder } from './SeatStyleBuilder';
import { resourceManager } from '@kit.LocalizationKit';
import { util } from '@kit.ArkTS';
import { BaseMapFileInfo } from './BaseMapFileInfo';
import { ResourceExtensions } from './ResourceExtensions';
import { MockSeatData } from './MockSeatData';
import { MockZoneInfo } from './MockZoneInfo';

@Component
export struct SeatCanvas {
  private navPathStack: NavPathStack = new NavPathStack();
  @State private baseMap: BaseMapFileInfo | null = null;
  @State private controller: SeatCanvasViewController = new SeatCanvasViewController();
  @State useCircleSeatStyle: boolean = false;
  @State useSVGSeatStyle: boolean = true;
  private delegate: SeatCanvasRendererDelegate | null = null;

  aboutToAppear(): void {
    this.delegate = {
      shouldSelectSeat: (zoneId: string, seatId: string) => {
        console.log(`tgfx shouldSelectSeat: ${zoneId} ${seatId}`)
        return true;
      },
      didSelectSeat: (zoneId: string, seatId: string) => {
        console.log(`tgfx didSelectSeat: ${zoneId} ${seatId}`)
      },
      didDeselectSeat: (zoneId: string, seatId: string) => {
        console.log(`tgfx didDeselectSeat: ${zoneId} ${seatId}`)
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
    const zoneDatas = this.loadZoneDatas();
    this.controller.updateSeatZones(zoneDatas);

    const seatDatas = this.loadSeatDatas();
    for (const zoneId of Object.keys(seatDatas)) {
      const seats = seatDatas[zoneId];
      const seatDataArray: SeatData[] = seats.map((mockSeat: MockSeatData): SeatData => ({
        seatId: mockSeat.seatId,
        status: mockSeat.status,
        selected: mockSeat.selected || false,
        x: mockSeat.x,
        y: mockSeat.y
      }));
      this.controller.updateSeats(zoneId, seatDataArray);
    }
  }

  loadZoneDatas(): SeatZoneData[] {
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
      return zones.map((zone: MockZoneInfo): SeatZoneData => ({
        zoneId: zone.zoneId,
        color: zone.color,
        priceColor: zone.pirceColor
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
        SeatCanvasView({ controller: this.controller })
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
      await this.controller.loadFromAssets(manager, this.baseMap.assetPath, BaseMapFormat.SVG);
      console.log(`底图: ${this.baseMap.assetPath} 加载成功`);
      this.loadMockData();
    })
  }
}