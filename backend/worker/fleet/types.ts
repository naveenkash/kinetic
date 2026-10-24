// Fleet adapter interface (backend-architecture section 10.3). The fleet provider is chosen
// by the beacon test (tech/README decision 13); only the adapter changes.

export interface AllocateRequest {
  matchId: string;
  region: string;
  build: string;
  env: Record<string, string>;
}

export interface Allocation {
  fleetRef: string;
  host: string;
  port: number;
}

export interface Fleet {
  readonly name: string;
  allocate(req: AllocateRequest): Promise<Allocation>;
  deallocate(fleetRef: string): Promise<void>;
  shutdown(): Promise<void>;
}
