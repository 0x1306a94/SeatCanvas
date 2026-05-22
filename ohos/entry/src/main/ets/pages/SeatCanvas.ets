import {
  BaseMapFormat,
  SeatCanvasView,
  SeatCanvasViewController,
  SVGBaseMapParseConfig
} from 'libseatcanvas';
import { SeatStyleBuilder } from './SeatStyleBuilder';
import { util } from '@kit.ArkTS';
import { BaseMapFileInfo } from './BaseMapFileInfo';
import { SeatCanvasSampleModel } from './SeatCanvasSampleModel';

@Component
export struct SeatCanvas {
  private navPathStack: NavPathStack = new NavPathStack();
  @State private baseMap: BaseMapFileInfo | null = null;
  @State private controller: SeatCanvasViewController = new SeatCanvasViewController();
  @State useCircleSeatStyle: boolean = false;
  @State useSVGSeatStyle: boolean = true;
  private sampleModel: SeatCanvasSampleModel = new SeatCanvasSampleModel();

  aboutToAppear(): void {
    this.controller.seatSize = 24
    this.controller.debugHUDEnabled = true
    this.controller.setDelegate(this.sampleModel.getDelegate());
    this.controller.canvasColor = 0xFFF8F8F8;
  }

  aboutToDisappear(): void {
    this.sampleModel.stopAvailableSeatsTimer();
    this.sampleModel.detachSeatCanvasController();
    this.controller.setDelegate(null);
    this.controller.release();
  }

  applyCircleSeatStyleConfig(): void {
    const config = SeatStyleBuilder.buildCircleSeatStyleConfig(this.sampleModel.prices);
    this.controller.applySeatStyleJSONConfig(config);
  }

  applySVGSeatStyleConfig(): void {
    const manager = getContext(this).resourceManager;
    const config = SeatStyleBuilder.buildSVGSeatStyleConfig(manager, this.sampleModel.prices);
    this.controller.applySeatStyleJSONConfig(config);
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

      const manager = getContext(this).resourceManager;
      const readRawFile = (path: string): string | null => this.readRawFileToString(path);
      const parseConfig = new SVGBaseMapParseConfig(['zoneId', 'regioncode']);
      this.sampleModel.prepareForBaseMap(this.controller, this.baseMap, readRawFile, () => {
        this.applySVGSeatStyleConfig();
      });
      await this.controller.loadFromAssets(manager, this.baseMap.assetPath, BaseMapFormat.SVG, parseConfig);
      console.log(`底图: ${this.baseMap.assetPath} 加载成功`);
    })
  }
}
