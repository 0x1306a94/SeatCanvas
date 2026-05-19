export interface MockPriceData {
  code: string;
  name: string;
  color: string;
  zoneIds: string[];
}

export function parseMockPriceDataList(jsonString: string): MockPriceData[] {
  const prices: MockPriceData[] = JSON.parse(jsonString);
  return prices;
}
