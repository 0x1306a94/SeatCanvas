export enum ResourceScope {
  DEFAULT = 'default',
  CUSTOMIZED = 'customized'
}

export interface BaseMapFileInfo {
  scope: ResourceScope;
  filename: string;
  assetPath: string;
}
