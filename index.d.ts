declare module "mmap-io-latest" {
  export const PROT_READ: number;
  export const PROT_WRITE: number;
  export const PROT_EXEC: number;
  export const PROT_NONE: number;

  export const MAP_SHARED: number;
  export const MAP_PRIVATE: number;
  export const MAP_NONBLOCK: number;
  export const MAP_POPULATE: number;

  export const MADV_NORMAL: number;
  export const MADV_RANDOM: number;
  export const MADV_SEQUENTIAL: number;
  export const MADV_WILLNEED: number;
  export const MADV_DONTNEED: number;

  /**
   * Maps a file into memory and returns a Node.js Buffer.
   * 
   * @param size File size to map.
   * @param protection PROT_READ, PROT_WRITE, etc.
   * @param flags MAP_SHARED, MAP_PRIVATE, etc.
   * @param fd File descriptor (from fs.openSync).
   * @param offset Byte offset (default 0).
   * @param advise Usage hint (MADV_*).
   */
  export function map(
    size: number,
    protection: number,
    flags: number,
    fd: number,
    offset?: number,
    advise?: number
  ): Buffer;

  /**
   * Provides usage advice to the kernel for the mapped memory.
   */
  export function advise(
    buffer: Buffer,
    advise: number
  ): void;

  export function advise(
    buffer: Buffer,
    offset: number,
    length: number,
    advise: number
  ): void;

  /**
   * Checks how many pages are mapped in memory.
   * 
   * @returns [unmappedPages, mappedPages]
   */
  export function incore(buffer: Buffer): [number, number];

  /**
   * Syncs the mapped buffer to disk.
   */
  export function sync(
    buffer: Buffer,
    offset?: number,
    length?: number,
    blocking?: boolean,
    invalidate?: boolean
  ): void;
}
