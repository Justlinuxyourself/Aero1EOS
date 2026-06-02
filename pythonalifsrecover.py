import sys
import struct
import platform
import os

INODE_FMT = "<32sIIBB"
INODE_SIZE = 42

def recover(source, is_disk):
    mode = "rb"
    if is_disk and platform.system() == "Windows":
        source = f"\\\\.\\{source}"

    try:
        with open(source, mode) as f:
            f.seek(20001 * 512)
            inode_sector = f.read(512)

            # First, extract all directories so we can save files into them
            for i in range(12):
                offset = i * INODE_SIZE
                chunk = inode_sector[offset : offset + INODE_SIZE]
                name_bytes, lba, size, active, is_dir = struct.unpack(INODE_FMT, chunk)
                
                if active and is_dir:
                    name = name_bytes.decode('utf-8', errors='ignore').strip('\x00').lstrip("/")
                    if name:
                        os.makedirs(name, exist_ok=True)
                        print(f"[+] Created directory on host: {name}")

            # Second, extract files into their respective folders
            f.seek(20001 * 512) # Rewind to read files
            for i in range(12):
                offset = i * INODE_SIZE
                chunk = inode_sector[offset : offset + INODE_SIZE]
                name_bytes, lba, size, active, is_dir = struct.unpack(INODE_FMT, chunk)
                
                if active and not is_dir:
                    name = name_bytes.decode('utf-8', errors='ignore').strip('\x00').lstrip("/")
                    f.seek(lba * 512)
                    data = f.read(size)
                    with open(name, "wb") as outfile:
                        outfile.write(data)
                        print(f"    [+] Extracted file: {name}")
                        
    except Exception as e:
        print(f"[!] Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python ali_recover.py <d|i> <path>")
        sys.exit(1)
    recover(sys.argv[2], sys.argv[1] == 'd')
