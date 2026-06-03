import sys
import struct
import os

INODE_FMT = "<32sIIBB"
INODE_SIZE = 42
MAX_FILES = 12

def recover(source, is_disk):
    # If it's a disk, handle Windows raw access
    mode = "rb"
    if is_disk and sys.platform == "win32":
        source = f"\\\\.\\{source}"

    try:
        with open(source, mode) as f:
            # AliFS Inode Table is at ALIFS_START_LBA (20000) + 1
            f.seek(20001 * 512)
            inode_sector = f.read(512)

            # 1. First pass: Collect all paths and create directories
            inodes = []
            for i in range(MAX_FILES):
                offset = i * INODE_SIZE
                chunk = inode_sector[offset : offset + INODE_SIZE]
                if len(chunk) < INODE_SIZE: break
                
                name_bytes, lba, size, active, is_dir = struct.unpack(INODE_FMT, chunk)
                
                if active:
                    # Clean the path string
                    name = name_bytes.decode('utf-8', errors='ignore').split('\x00')[0].strip()
                    # Remove leading slash for local host path
                    local_path = name.lstrip('/')
                    
                    if is_dir:
                        os.makedirs(local_path, exist_ok=True)
                        print(f"[+] Created directory: {local_path}")
                    
                    inodes.append({'name': local_path, 'lba': lba, 'size': size, 'is_dir': is_dir})

            # 2. Second pass: Extract files
            for entry in inodes:
                if not entry['is_dir'] and entry['lba'] > 0:
                    f.seek(entry['lba'] * 512)
                    data = f.read(entry['size'])
                    
                    # Ensure parent dir exists (handles deep nesting like /a/b/c/file.txt)
                    parent = os.path.dirname(entry['name'])
                    if parent and not os.path.exists(parent):
                        os.makedirs(parent, exist_ok=True)
                        
                    with open(entry['name'], "wb") as outfile:
                        outfile.write(data)
                        print(f"    [+] Extracted file: {entry['name']}")

    except Exception as e:
        print(f"[!] Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python pythonalifsrecover.py <d|i> <path>")
        sys.exit(1)
    recover(sys.argv[2], sys.argv[1] == 'd')
