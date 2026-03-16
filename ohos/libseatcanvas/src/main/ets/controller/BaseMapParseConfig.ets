export interface BaseMapParseConfig {
  serializeToString(): string | null;
}

export class SVGBaseMapParseConfig implements BaseMapParseConfig {
  zoneIdAttributeNames: Array<string>;

  constructor(zoneIdAttributeNames: Array<string> = ['zoneId']) {
    this.zoneIdAttributeNames = zoneIdAttributeNames;
  }

  serializeToString(): string | null {
    return JSON.stringify({
      zoneIdAttributeNames: this.zoneIdAttributeNames
    });
  }
}
