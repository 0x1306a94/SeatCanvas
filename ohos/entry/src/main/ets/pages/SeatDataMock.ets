import { SeatData, SeatZoneData } from 'libseatcanvas';

export interface RegionRect {
  x: number;
  y: number;
  width: number;
  height: number;
}

export interface RegionInfo {
  zoneId: string;
  rect: RegionRect;
}

export function getMockRegions(): RegionInfo[] {
  return [
    { zoneId: '20011', rect: { x: 850, y: 2800, width: 550, height: 320 } },
    { zoneId: '20012', rect: { x: 850, y: 3170, width: 550, height: 320 } },
    { zoneId: '20013', rect: { x: 850, y: 3540, width: 550, height: 320 } },
    { zoneId: '20014', rect: { x: 850, y: 3910, width: 550, height: 320 } },
    { zoneId: '20031', rect: { x: 2000, y: 2800, width: 450, height: 280 } },
    { zoneId: '20032', rect: { x: 2000, y: 3130, width: 450, height: 280 } },
    { zoneId: '20034', rect: { x: 2000, y: 3790, width: 450, height: 280 } },
    { zoneId: '10085', rect: { x: 1700, y: 1520, width: 700, height: 230 } },
    { zoneId: '10086', rect: { x: 2450, y: 1520, width: 700, height: 230 } },
    { zoneId: '30031', rect: { x: 9550, y: 2800, width: 450, height: 280 } },
    { zoneId: '30032', rect: { x: 9550, y: 3130, width: 450, height: 280 } },
    { zoneId: '30033', rect: { x: 9550, y: 3460, width: 450, height: 280 } },
    { zoneId: '10096', rect: { x: 9600, y: 1800, width: 700, height: 220 } },
    { zoneId: '10098', rect: { x: 11100, y: 1800, width: 700, height: 220 } },
    { zoneId: '40109', rect: { x: 6950, y: 9300, width: 700, height: 250 } },
    { zoneId: '40110', rect: { x: 7700, y: 9300, width: 700, height: 250 } },
    { zoneId: '40123', rect: { x: 6600, y: 9600, width: 750, height: 300 } },
    { zoneId: '40124', rect: { x: 7400, y: 9600, width: 750, height: 300 } },
    { zoneId: '37492', rect: { x: 33, y: 411, width: 289, height: 329 } },
    { zoneId: '74148', rect: { x: 400, y: 76, width: 332, height: 232 } },
  ];
}

function toHex2(n: number): string {
  const s = n.toString(16);
  return s.length >= 2 ? s : '0' + s;
}

function randomPriceColorHex(): string {
  const r = 128 + Math.floor(Math.random() * 128);
  const g = 128 + Math.floor(Math.random() * 128);
  const b = 128 + Math.floor(Math.random() * 128);
  return '#' + toHex2(r) + toHex2(g) + toHex2(b);
}

export function generateMockZoneDatas(): SeatZoneData[] {
  const regions = getMockRegions();
  return regions.map((r: RegionInfo): SeatZoneData => {
    return {
      zoneId: r.zoneId,
      color: '#ffffff',
      priceColor: randomPriceColorHex(),
    };
  });
}

export function generateMockSeatDatas(rect: RegionRect): SeatData[] {
  const itemSize = 36.0;
  const spacing = 10.0;
  const cols = Math.floor(rect.width / (itemSize + spacing));
  const rows = Math.floor(rect.height / (itemSize + spacing));
  const seats: SeatData[] = [];

  for (let row = 0; row < rows; row++) {
    for (let col = 0; col < cols; col++) {
      const seatX = rect.x + col * (itemSize + spacing);
      const seatY = rect.y + row * (itemSize + spacing);
      const seatId = `seat_${row}_${col}`;
      const randValue = (row * cols + col) % 10;
      let status: number;
      if (randValue < 6) {
        status = 0;
      } else if (randValue < 8) {
        status = 1;
      } else if (randValue < 9) {
        status = 2;
      } else {
        status = 3;
      }
      seats.push({
        seatId,
        status,
        selected: false,
        x: seatX,
        y: seatY,
      });
    }
  }
  return seats;
}