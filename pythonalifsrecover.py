import sys
import struct
import os

INODE_FMT = "<32sIIBB"
INODE_SIZE = 42
MAX_FILES = 12

def recover(source, is_disk):
    mode = "rb"
    if is_disk and sys.platform == "win32":
        source = f"\\\\.\\{source}"

    try:
        with open(source, mode) as f:
            f.seek(20001 * 512)
            inode_sector = f.read(512)

            # Collection of valid files/dirs
            inodes = []
            
            for i in range(MAX_FILES):
                offset = i * INODE_SIZE
                chunk = inode_sector[offset : offset + INODE_SIZE]
                name_bytes, lba, size, active, is_dir = struct.unpack(INODE_FMT, chunk)
                
                # 1. Decode and strip junk
                raw_name = name_bytes.decode('utf-8', errors='ignore').split('\x00')[0].strip()
                
                # 2. VALIDATION: Only process if active AND it's a valid AliFS path
                # This ignores memory garbage and uninitialized slots
                if active and raw_name.startswith('/'):
                    local_path = raw_name.lstrip('/')
                    
                    if is_dir:
                        os.makedirs(local_path, exist_ok=True)
                        print(f"[+] Created directory: {local_path}")
                    
                    inodes.append({'name': local_path, 'lba': lba, 'size': size, 'is_dir': is_dir})
            
            # 3. Extraction pass
            for entry in inodes:
                if not entry['is_dir'] and entry['lba'] > 0:
                    f.seek(entry['lba'] * 512)
                    data = f.read(entry['size'])
                    
                    # Ensure parent dir exists (e.g., /dir/dir2/file)
                    parent = os.path.dirname(entry['name'])
                    if parent and not os.path.exists(parent):
                        os.makedirs(parent, exist_ok=True)
                        
                    with open(entry['name'], "wb") as outfile:
                        outfile.write(data)
                        print(f"    [+] Extracted: {entry['name']}")

    except Exception as e:
        print(f"[!] Critical Error: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python pythonalifsrecover.py <d|i> <path>")
        sys.exit(1)
    recover(sys.argv[2], sys.argv[1] == 'd')
