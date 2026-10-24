// Edgegap adapter: deliberately not implemented yet. The fleet provider is chosen by the
// two-week beacon test (tech/README decision 13) before any provider API code is written.
import type { AllocateRequest, Allocation, Fleet } from "./types.ts";

export class EdgegapFleet implements Fleet {
  readonly name = "edgegap";

  allocate(_req: AllocateRequest): Promise<Allocation> {
    return Promise.reject(new Error("Edgegap adapter not implemented; choose the fleet first"));
  }

  deallocate(_fleetRef: string): Promise<void> {
    return Promise.reject(new Error("Edgegap adapter not implemented; choose the fleet first"));
  }

  shutdown(): Promise<void> {
    return Promise.resolve();
  }
}
