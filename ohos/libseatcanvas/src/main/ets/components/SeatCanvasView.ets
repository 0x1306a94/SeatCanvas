import { SeatCanvasViewController } from '../controller/SeatCanvasViewController';
import { seatcanvas } from 'libseatcanvas.so';

@Component
export struct SeatCanvasView {
  @ObjectLink controller: SeatCanvasViewController;
  private startPanX: number = 0;
  private startPanY: number = 0;
  private translationX: number = 0;
  private translationY: number = 0;
  private pinchX: number = 0;
  private pinchY: number = 0;
  private pinchScale: number = 1.0;

  aboutToAppear(): void {
    this.controller.attachToView(new WeakRef(this));
  }

  aboutToDisappear(): void {
    this.controller.detachFromView();
  }

  build() {
    Column() {
      XComponent(
        {
          id: this.controller.uniqueID(),
          type: XComponentType.SURFACE,
          libraryname: "seatcanvas"
        }
      )
        .backgroundColor(Color.Transparent)
        .onLoad(() => {

        })
    }
    .width('100%')
    .height('100%')
    .gesture(GestureGroup(GestureMode.Exclusive,
      TapGesture({ count: 1, fingers: 1 })
        .onAction((event: GestureEvent) => {
          if (event && event.tapLocation) { // API 20
            let x = vp2px(event.tapLocation.x);
            let y = vp2px(event.tapLocation.y);
            this.handleTap(x, y);
          } else if (event && event.fingerList.length > 0) {
            let finger = event.fingerList[0]
            let x = vp2px(finger.localX);
            let y = vp2px(finger.localY);
            this.handleTap(x, y);
          }
        }),

      PinchGesture({ fingers: 2, distance: 1 })
        .onActionStart((event: GestureEvent) => {
          if (event) {
            // this.pinchX = vp2px(event.fingerList[0].displayX + event.fingerList[1].displayX) / 2.0;
            // this.pinchY = vp2px(event.fingerList[0].displayY + event.fingerList[1].displayY) / 2.0;
            this.pinchX = vp2px(event.pinchCenterX);
            this.pinchY = vp2px(event.pinchCenterY);
            this.pinchScale = 1.0;
            this.controller.handlePinch(seatcanvas.GestureState.BEGAN, this.pinchScale, this.pinchX, this.pinchY);
          }
        })
        .onActionUpdate((event: GestureEvent) => {
          if (event) {
            // this.pinchX = vp2px(event.fingerList[0].displayX + event.fingerList[1].displayX) / 2.0;
            // this.pinchY = vp2px(event.fingerList[0].displayY + event.fingerList[1].displayY) / 2.0;
            this.pinchX = vp2px(event.pinchCenterX);
            this.pinchY = vp2px(event.pinchCenterY);
            this.pinchScale = 1.0 * event.scale;
            this.controller.handlePinch(seatcanvas.GestureState.CHANGED, this.pinchScale, this.pinchX, this.pinchY);
          }
        })
        .onActionEnd((event: GestureEvent) => {
          if (event) {
            // this.pinchX = vp2px(event.fingerList[0].displayX + event.fingerList[1].displayX) / 2.0;
            // this.pinchY = vp2px(event.fingerList[0].displayY + event.fingerList[1].displayY) / 2.0;
            this.pinchX = vp2px(event.pinchCenterX);
            this.pinchY = vp2px(event.pinchCenterY);
            this.pinchScale = 1.0 * event.scale;
            this.controller.handlePinch(seatcanvas.GestureState.ENDED, this.pinchScale, this.pinchX, this.pinchY);
          }
        })
        .onActionCancel(() => {
          this.controller.handlePinch(seatcanvas.GestureState.CANCELLED, this.pinchScale, this.pinchX, this.pinchY);
          this.pinchScale = 1.0;
        }),

      PanGesture({ fingers: 1, distance: 1 })
        .onActionStart((event: GestureEvent) => {
          if (event) {
            this.startPanX = event.offsetX;
            this.startPanY = event.offsetY;
            this.translationX = 0;
            this.translationY = 0;
            this.controller.handlePan(seatcanvas.GestureState.BEGAN, 0, 0);
          }
        })
        .onActionUpdate((event: GestureEvent) => {
          if (event) {
            let offsetX = vp2px(event.offsetX - this.startPanX);
            let offsetY = vp2px(event.offsetY - this.startPanY);
            this.translationX = -offsetX;
            this.translationY = -offsetY;
            this.controller.handlePan(seatcanvas.GestureState.CHANGED, this.translationX, this.translationY);
          }
        })
        .onActionEnd((event: GestureEvent) => {
          if (event) {
            let offsetX = vp2px(event.offsetX - this.startPanX);
            let offsetY = vp2px(event.offsetY - this.startPanY);
            this.translationX = -offsetX;
            this.translationY = -offsetY;
            this.controller.handlePan(seatcanvas.GestureState.ENDED, this.translationX, this.translationY);
          }
        })
        .onActionCancel(() => {
          this.controller.handlePan(seatcanvas.GestureState.CANCELLED, this.translationX, this.translationY);
        })
    ))
  }

  private handleTap(x: number, y: number) {
    this.controller.handleTap(x, y);
  }
}