import { SeatCanvasModule } from "./types";
import { setSeatCanvasModule } from './SeatCanvas-module';
import { TGFX } from '@tgfx/types';
import { TGFXBind } from '@tgfx/binding';

export const SeatCanvasModuleBinding = (module: SeatCanvasModule) => {
    TGFXBind(module as unknown as TGFX);
    setSeatCanvasModule(module);
    module.module = module;
};