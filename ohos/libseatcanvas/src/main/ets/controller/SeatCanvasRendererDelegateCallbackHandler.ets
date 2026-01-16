import { SeatCanvasViewController } from "./SeatCanvasViewController";

export class SeatCanvasRendererDelegateCallbackHandler {
  private weakViewController: WeakRef<SeatCanvasViewController>;

  constructor(controller: SeatCanvasViewController) {
    this.weakViewController = new WeakRef(controller);
  }

  readonly onShouldSelectSeat = (regionId: string, seatId: string): boolean => {
    const viewController = this.weakViewController.deref();
    if (viewController) {
      return viewController.onShouldSelectSeat(regionId, seatId);
    }
    return false;
  }
  readonly onDidSelectSeat = (regionId: string, seatId: string) => {
    const viewController = this.weakViewController.deref();
    if (viewController) {
      viewController.onDidSelectSeat(regionId, seatId);
    }
  }
  readonly onDidDeselectSeat = (regionId: string, seatId: string) => {
    const viewController = this.weakViewController.deref();
    if (viewController) {
      viewController.onDidDeselectSeat(regionId, seatId);
    }
  }
}