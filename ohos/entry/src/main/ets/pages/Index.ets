import { SeatCanvas } from './SeatCanvas';
import { SeatCanvasV2 } from './SeatCanvasV2';

@Preview
@Entry
@ComponentV2
struct Index {
  @Local basemapNames: string[] = [
    "performbg",
    "performbg_2",
    "73807",
    "73808",
  ]
  @Local useV2: boolean = false
  private navPathStack: NavPathStack = new NavPathStack();

  build() {
    Navigation(this.navPathStack) {
      Column() {
        Row() {
          Button(this.useV2 ? "使用 V1 状态" : "使用 V2 状态")
            .onClick(() => {
              this.useV2 = !this.useV2;
            })
        }
        .justifyContent(FlexAlign.Center)
        .alignItems(VerticalAlign.Center)
        .width('100%')


        List() {
          ForEach(this.basemapNames, (item: string, index: number) => {
            ListItem() {
              Row() {
                Text(item)
                  .fontSize(16)
              }
              .width('100%')
              .padding({ left: 20 })
              .alignItems(VerticalAlign.Center)
              .justifyContent(FlexAlign.Start)
            }
            .width('100%')
            .height(44)
            .onClick(() => {
              this.navPathStack.pushPath({ name: this.useV2 ? 'SeatCanvasV2' : 'SeatCanvas', param: item })
            })
          })
        }
        .listDirection(Axis.Vertical)
        .width('100%')
        .layoutWeight(1)
      }
      .width('100%')
      .height('100%')
    }
    .title("底图")
    .mode(NavigationMode.Stack)
    .navDestination(this.navDestinationMap)
  }

  @Builder
  navDestinationMap(name: string) {
    if (name == "SeatCanvas") {
      SeatCanvas()
    } else if (name == "SeatCanvasV2") {
      SeatCanvasV2()
    }
  }
}
