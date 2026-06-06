963<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
963
963<h3 align="center">
963  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
963</h3>
963
963<p align="center">
963  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
963  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
963</p>
963
963<p align="center">
963  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
963  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
963</p>
963
963---
963> [!WARNING]
963> some features dont work on real hardware, and some are untested.
963
963Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
963
963## 🚀 Key Features
963
963**64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
963**Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
963**Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
963**Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
963**Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
963
963---
963
963## 🏗️ Technical Architecture
963
963### 1. Boot & Memory
963**Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
963**Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
963**Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
963
963### 2. Clock & Timing
963**PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
963**Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
963
963### 3. Video & I/O
963**VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
963**CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
963
963---
963
963## 🛠️ Built-in Shell Commands
963
963| Command | Description |
963| :--- | :--- |
963| `help` | Lists all registered system commands. |
963| `cls` | Clears the notebook screen. |
963| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
963| `uptime` | Show how long Aero1EOS has been running. |
963| `free` | Check dynamic RAM usage (Total, Used, Free). |
963| `timezone` | Adjusts the status bar clock offset in real-time. |
963| `lock` | Manually triggers the hardened lock screen. |
963| `test` | Verifies timer calibration with a 5s countdown. |
963| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
963| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
963| `sleep` | puts system in sleep until a key press |
963| `plane` | shows a art of a plane |
963| `about_dev` | About Developer |
963| `echo`| print text to screen |
963| `ayah` | chooses random ayah from quran and prints it |
963| `verse` | chooses random verse from the bible and prints it |
963| `tdadd` | adds todo to mem |
963| `tdshw` | shows todo from mem |
963| `calc` | Calculator |
963-# many more with the `help` command 🥹
963
963## 📁 Directory Structure
963```text
963.
963├── src
963│   ├── kernel.c           # Main OS Loop & Hardening Logic
963│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
963│   ├── section2_video     # VGA Driver & TTY Management
963│   ├── section3_io        # CMOS & Keyboard Drivers
963│   └── section4_shell     # Shell Logic & Command Library
963└── linker.ld              # Kernel Memory Layout
963```
963## 🍎 Bad Apple!! VGA Demo
963Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
963
963### 🛠 Technical Specifications
963**Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
963**Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
963**Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
963**Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
963
963## 📜 Licensing & Credits
963
963### Kernel & Operating System
963Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
963
963 using, modifying, or distributing this software, you agree to the following terms:
963**Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
963**No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
963
963> [!CAUTION]
963> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
963
963### Demo Assets
963**Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
963
963## Bugs:
963- Beep stutters
963- pit timer runs at 200hz instead of 100hz for some reason
963## Todo:
963- [ ] ADD: PCI Driver (WIP)
963- [ ] ADD: ETH and/or WIFI Support (WIP)
963- [ ] ADD: GUI Support (might never add)
963- [x] ADD: ATA Support (With AliFS)
963- [x] FIX: Beep stutter (Doesnt stutter on real hardware)