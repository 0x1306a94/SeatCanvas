import { SeatCanvasViewV2, SeatCanvasViewController, BaseMapFormat, SeatCanvasRendererDelegate } from 'libseatcanvas';
import { SeatStyleBuilder } from './SeatStyleBuilder';
import { resourceManager } from '@kit.LocalizationKit';
import { generateMockSeatDatas, generateMockZoneDatas, getMockRegions } from './SeatDataMock';

@ComponentV2
export struct SeatCanvasV2 {
  private navPathStack: NavPathStack = new NavPathStack();
  @Local private basemapName: string = '';
  @Local private controller: SeatCanvasViewController = new SeatCanvasViewController();
  @Local useCircleSeatStyle: boolean = false;
  @Local useSVGSeatStyle: boolean = true;
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
    const zones = generateMockZoneDatas();
    this.controller.updateSeatZones(generateMockZoneDatas());
    const regions = getMockRegions();
    for (const r of regions) {
      this.controller.updateSeats(r.zoneId, generateMockSeatDatas(r.rect));
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
    .title(this.basemapName)
    .onReady(async (ctx) => {
      this.navPathStack = ctx.pathStack
      this.basemapName = ctx.pathInfo.param as string

      let manager = getContext(this).resourceManager;
      this.applySVGSeatStyleConfig();
      await this.controller.loadFromAssets(manager, `svg/${this.basemapName}.svg`, BaseMapFormat.SVG);
      console.log(`底图: svg/${this.basemapName}.svg 加载成功`);
      this.loadMockData();
    })
  }
}