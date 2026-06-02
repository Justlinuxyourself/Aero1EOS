import sys
import struct
import platform

# < : Little-endian
# 32s : 32 bytes for filename
# I : 4 bytes for start_lba
# I : 4 bytes for size
# B : 1 byte for active
# B : 1 byte for is_dir
INODE_FMT = "<32sIIBB"
INODE_SIZE = 42

def recover(source, is_disk):
    mode = "rb"
    if is_disk and platform.system() == "Windows":
        source = f"\\\\.\\{source}"

    try:
        with open(source, mode) as f:
            # LBA 20000 + 1 = 20001
            f.seek(20001 * 512)
            inode_sector = f.read(512)

            # We have 12 files (MAX_FILES)
            for i in range(12):
                offset = i * INODE_SIZE
                chunk = inode_sector[offset : offset + INODE_SIZE]
                
                name_bytes, lba, size, active, is_dir = struct.unpack(INODE_FMT, chunk)
                
                if active:
                    name = name_bytes.decode('utf-8', errors='ignore').strip('\x00')
                    print(f"[*] Found: {name} (LBA: {lba}, Size: {size})")
                    
                    if not is_dir and size > 0:
                        f.seek(lba * 512)
                        data = f.read(size)
                        # Replace '/' with '_' to make a valid filename
                        out_name = name.replace("/", "_").lstrip("_")
                        with open(out_name, "wb") as outfile:
                            outfile.write(data)
                            print(f"    [+] Extracted to {out_name}")
                        
    except Exception as e:
        print(f"[!] Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python ali_recover.py <d|i> <path>")
        sys.exit(1)
    recover(sys.argv[2], sys.argv[1] == 'd')
