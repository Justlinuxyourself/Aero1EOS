946
946
946<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
946
946<h3 align="center">
946  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
946</h3>
946
946<p align="center">
946  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
946  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
946</p>
946
946<p align="center">
946  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
946  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
946</p>
946
946---
946> [!WARNING]
946> some features dont work on real hardware, and some are untested.
946
946Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
946
946## 🚀 Key Features
946
946* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
946* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
946* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
946* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
946* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
946
946---
946
946## 🏗️ Technical Architecture
946
946### 1. Boot & Memory
946* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
946* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
946* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
946
946### 2. Clock & Timing
946* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
946* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
946
946### 3. Video & I/O
946* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
946* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
946
946---
946
946## 🛠️ Built-in Shell Commands
946
946| Command | Description |
946| :--- | :--- |
946| `help` | Lists all registered system commands. |
946| `cls` | Clears the notebook screen. |
946| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
946| `uptime` | Show how long Aero1EOS has been running. |
946| `free` | Check dynamic RAM usage (Total, Used, Free). |
946| `timezone` | Adjusts the status bar clock offset in real-time. |
946| `lock` | Manually triggers the hardened lock screen. |
946| `test` | Verifies timer calibration with a 5s countdown. |
946| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
946| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
946| `sleep` | puts system in sleep until a key press |
946| `plane` | shows a art of a plane |
946| `about_dev` | About Developer |
946| `echo`| print text to screen |
946| `ayah` | chooses random ayah from quran and prints it |
946| `verse` | chooses random verse from the bible and prints it |
946| `tdadd` | adds todo to mem |
946| `tdshw` | shows todo from mem |
946| `calc` | Calculator |
946-# many more with the `help` command 🥹
946
946## 📁 Directory Structure
946```text
946.
946├── src
946│   ├── kernel.c           # Main OS Loop & Hardening Logic
946│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
946│   ├── section2_video     # VGA Driver & TTY Management
946│   ├── section3_io        # CMOS & Keyboard Drivers
946│   └── section4_shell     # Shell Logic & Command Library
946└── linker.ld              # Kernel Memory Layout
946```
946## 🍎 Bad Apple!! VGA Demo
946Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
946
946### 🛠 Technical Specifications
946* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
946* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
946* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
946* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
946
946## 📜 Licensing & Credits
946
946### Kernel & Operating System
946Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
946
946By using, modifying, or distributing this software, you agree to the following terms:
946* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
946* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
946
946> [!CAUTION]
946> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
946
946### Demo Assets
946* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
946
946## Bugs:
946- Beep stutters
946- pit timer runs at 200hz instead of 100hz for some reason
946## Todo:
946- [ ] ADD: PCI Driver (WIP)
946- [ ] ADD: ETH and/or WIFI Support (WIP)
946- [ ] ADD: GUI Support (might never add)
946- [x] ADD: ATA Support (With AliFS)
946- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
