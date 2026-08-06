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
            
            bytes_to_read = MAX_FILES * INODE_SIZE
            inode_sector = f.read(bytes_to_read)
            
            print(f"[*] Read {len(inode_sector)} bytes from sector 20001.")

            inodes = []
            
            for i in range(MAX_FILES):
                offset = i * INODE_SIZE
                chunk = inode_sector[offset : offset + INODE_SIZE]
                
                if len(chunk) < INODE_SIZE:
                    print(f"[-] Reached end of buffer at index {i}.")
                    break
                
                name_bytes, lba, size, active, is_dir = struct.unpack(INODE_FMT, chunk)
                raw_name = name_bytes.decode('utf-8', errors='ignore').split('\x00')[0].strip()
                
                # DEBUG PRINT: Inspect every single entry found in memory
                print(f"[DEBUG Inode {i:02d}] Active: {active} | IsDir: {is_dir} | LBA: {lba} | Size: {size} | Name: '{raw_name}'")
                
                # Check if path starts with / or \ (handling Windows/Unix path formats)
                is_valid_path = raw_name.startswith('/') or raw_name.startswith('\\') or len(raw_name) > 0
                
                if active and is_valid_path:
                    local_path = raw_name.lstrip('/\\')
                    if not local_path:
                        continue
                    
                    if is_dir:
                        os.makedirs(local_path, exist_ok=True)
                        print(f"[+] Created directory: {local_path}")
                    
                    inodes.append({'name': local_path, 'lba': lba, 'size': size, 'is_dir': is_dir})
            
            if not inodes:
                print("[-] No active inodes matching validation rules were found.")
                return

            for entry in inodes:
                if not entry['is_dir'] and entry['lba'] > 0 and entry['size'] > 0:
                    f.seek(entry['lba'] * 512)
                    data = f.read(entry['size'])
                    
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
