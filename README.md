942
942
942<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
942
942<h3 align="center">
942  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
942</h3>
942
942<p align="center">
942  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
942  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
942</p>
942
942<p align="center">
942  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
942  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
942</p>
942
942---
942> [!WARNING]
942> some features dont work on real hardware, and some are untested.
942
942Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
942
942## 🚀 Key Features
942
942* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
942* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
942* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
942* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
942* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
942
942---
942
942## 🏗️ Technical Architecture
942
942### 1. Boot & Memory
942* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
942* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
942* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
942
942### 2. Clock & Timing
942* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
942* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
942
942### 3. Video & I/O
942* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
942* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
942
942---
942
942## 🛠️ Built-in Shell Commands
942
942| Command | Description |
942| :--- | :--- |
942| `help` | Lists all registered system commands. |
942| `cls` | Clears the notebook screen. |
942| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
942| `uptime` | Show how long Aero1EOS has been running. |
942| `free` | Check dynamic RAM usage (Total, Used, Free). |
942| `timezone` | Adjusts the status bar clock offset in real-time. |
942| `lock` | Manually triggers the hardened lock screen. |
942| `test` | Verifies timer calibration with a 5s countdown. |
942| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
942| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
942| `sleep` | puts system in sleep until a key press |
942| `plane` | shows a art of a plane |
942| `about_dev` | About Developer |
942| `echo`| print text to screen |
942| `ayah` | chooses random ayah from quran and prints it |
942| `verse` | chooses random verse from the bible and prints it |
942| `tdadd` | adds todo to mem |
942| `tdshw` | shows todo from mem |
942| `calc` | Calculator |
942-# many more with the `help` command 🥹
942
942## 📁 Directory Structure
942```text
942.
942├── src
942│   ├── kernel.c           # Main OS Loop & Hardening Logic
942│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
942│   ├── section2_video     # VGA Driver & TTY Management
942│   ├── section3_io        # CMOS & Keyboard Drivers
942│   └── section4_shell     # Shell Logic & Command Library
942└── linker.ld              # Kernel Memory Layout
942```
942## 🍎 Bad Apple!! VGA Demo
942Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
942
942### 🛠 Technical Specifications
942* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
942* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
942* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
942* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
942
942## 📜 Licensing & Credits
942
942### Kernel & Operating System
942Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
942
942By using, modifying, or distributing this software, you agree to the following terms:
942* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
942* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
942
942> [!CAUTION]
942> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
942
942### Demo Assets
942* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
942
942## Bugs:
942- Beep stutters
942- pit timer runs at 200hz instead of 100hz for some reason
942## Todo:
942- [ ] ADD: PCI Driver (WIP)
942- [ ] ADD: ETH and/or WIFI Support (WIP)
942- [ ] ADD: GUI Support (might never add)
942- [x] ADD: ATA Support (With AliFS)
942- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
