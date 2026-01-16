export class SeatStyleKey {
  status: number;
  selected: boolean;

  constructor(status: number, selected: boolean) {
    this.status = status;
    this.selected = selected;
  }

  equals(other: SeatStyleKey): boolean {
    return this.status === other.status && this.selected === other.selected;
  }

  hashCode(): number {
    return this.status * 31 + (this.selected ? 1 : 0);
  }
}
