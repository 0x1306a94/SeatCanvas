import { SeatCanvasView, SeatCanvasViewController } from 'libseatcanvas';

@Component
export struct SeatCanvas {
  private navPathStack: NavPathStack = new NavPathStack();
  @State private basemapName: string = '';
  @State private controller: SeatCanvasViewController = new SeatCanvasViewController();
  @State enableTiled: boolean = false;
  @State enableZoomBlur: boolean = false;

  aboutToAppear(): void {

  }

  aboutToDisappear(): void {
    this.controller.release();
  }

  build() {
    NavDestination() {
      Column() {
        SeatCanvasView({ controller: this.controller })
          .width('100%')
          .layoutWeight(1)

        Row({ space: 20 }) {
          Row() {
            Text("Tiled")
              .fontSize(18)

            Toggle({ type: ToggleType.Switch, isOn: this.enableTiled })
              .onChange((isOn) => {
                this.enableTiled = isOn;
                this.controller.enableTiled(isOn);
              })
          }

          Row() {
            Text("ZoomBlur")
              .fontSize(18)

            Toggle({ type: ToggleType.Switch, isOn: this.enableZoomBlur })
              .onChange((isOn) => {
                this.enableZoomBlur = isOn;
                this.controller.enableZoomBlur(isOn);
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
      await this.controller.loadFromAssets(manager, `svg/${this.basemapName}.svg`);
      console.log(`底图: svg/${this.basemapName}.svg 加载成功`);
    })
  }
}