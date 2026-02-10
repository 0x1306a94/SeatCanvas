export interface ZoomLevel {
  // 最近视角（座位级别）
  seat: number;

  // 行/小块级视角
  row: number;

  // 区域级视角
  zone: number;

  // 全场/概览级视角（默认作为彩虹图与座位渲染切换的阈值）
  venue: number;
}