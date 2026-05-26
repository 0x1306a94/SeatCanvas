import type { SeatCanvasModule } from "./types";

export let seatcanvasModule: SeatCanvasModule | null = null;
export const setSeatCanvasModule = (module: SeatCanvasModule) => {
    seatcanvasModule = module;
};

export const getSeatCanvasModule = () => {
    return seatcanvasModule;
};