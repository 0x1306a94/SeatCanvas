import { resourceManager } from '@kit.LocalizationKit';
import { BaseMapFileInfo, ResourceScope } from './BaseMapFileInfo';

export class ResourceExtensions {
  static defaultSeatStylePath(name: string): string {
    return `SeatCanvasSampleAssets/${ResourceScope.DEFAULT}/seatstyle/${name}`;
  }

  static async defaultBaseMaps(manager: resourceManager.ResourceManager): Promise<BaseMapFileInfo[]> {
    const baseMapPath = `SeatCanvasSampleAssets/${ResourceScope.DEFAULT}/basemap`;
    try {
      const files = manager.getRawFileListSync(baseMapPath);
      return files.map((filename: string): BaseMapFileInfo => ({
        scope: ResourceScope.DEFAULT,
        filename: filename,
        assetPath: `${baseMapPath}/${filename}`
      }));
    } catch (e) {
      console.error(`Failed to list base maps from ${baseMapPath}:`, e);
      return [];
    }
  }

  static async customizedBaseMaps(manager: resourceManager.ResourceManager): Promise<BaseMapFileInfo[]> {
    const baseMapPath = `SeatCanvasSampleAssets/${ResourceScope.CUSTOMIZED}/basemap`;
    try {
      const files = manager.getRawFileListSync(baseMapPath);
      return files.map((filename: string): BaseMapFileInfo => ({
        scope: ResourceScope.CUSTOMIZED,
        filename: filename,
        assetPath: `${baseMapPath}/${filename}`
      }));
    } catch (e) {
      console.error(`Failed to list base maps from ${baseMapPath}:`, e);
      return [];
    }
  }

  static zoneDataPath(scope: ResourceScope, name: string): string {
    const jsonName = name.replace('.svg', '.json');
    return `SeatCanvasSampleAssets/${scope}/zonedata/${jsonName}`;
  }

  static seatDataPath(scope: ResourceScope, name: string): string {
    const jsonName = name.replace('.svg', '.json');
    return `SeatCanvasSampleAssets/${scope}/seatdata/${jsonName}`;
  }
}
