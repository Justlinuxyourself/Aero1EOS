944
944
944<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
944
944<h3 align="center">
944  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
944</h3>
944
944<p align="center">
944  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
944  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
944</p>
944
944<p align="center">
944  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
944  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
944</p>
944
944---
944> [!WARNING]
944> some features dont work on real hardware, and some are untested.
944
944Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
944
944## 🚀 Key Features
944
944* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
944* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
944* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
944* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
944* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
944
944---
944
944## 🏗️ Technical Architecture
944
944### 1. Boot & Memory
944* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
944* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
944* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
944
944### 2. Clock & Timing
944* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
944* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
944
944### 3. Video & I/O
944* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
944* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
944
944---
944
944## 🛠️ Built-in Shell Commands
944
944| Command | Description |
944| :--- | :--- |
944| `help` | Lists all registered system commands. |
944| `cls` | Clears the notebook screen. |
944| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
944| `uptime` | Show how long Aero1EOS has been running. |
944| `free` | Check dynamic RAM usage (Total, Used, Free). |
944| `timezone` | Adjusts the status bar clock offset in real-time. |
944| `lock` | Manually triggers the hardened lock screen. |
944| `test` | Verifies timer calibration with a 5s countdown. |
944| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
944| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
944| `sleep` | puts system in sleep until a key press |
944| `plane` | shows a art of a plane |
944| `about_dev` | About Developer |
944| `echo`| print text to screen |
944| `ayah` | chooses random ayah from quran and prints it |
944| `verse` | chooses random verse from the bible and prints it |
944| `tdadd` | adds todo to mem |
944| `tdshw` | shows todo from mem |
944| `calc` | Calculator |
944-# many more with the `help` command 🥹
944
944## 📁 Directory Structure
944```text
944.
944├── src
944│   ├── kernel.c           # Main OS Loop & Hardening Logic
944│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
944│   ├── section2_video     # VGA Driver & TTY Management
944│   ├── section3_io        # CMOS & Keyboard Drivers
944│   └── section4_shell     # Shell Logic & Command Library
944└── linker.ld              # Kernel Memory Layout
944```
944## 🍎 Bad Apple!! VGA Demo
944Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
944
944### 🛠 Technical Specifications
944* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
944* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
944* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
944* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
944
944## 📜 Licensing & Credits
944
944### Kernel & Operating System
944Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
944
944By using, modifying, or distributing this software, you agree to the following terms:
944* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
944* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
944
944> [!CAUTION]
944> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
944
944### Demo Assets
944* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
944
944## Bugs:
944- Beep stutters
944- pit timer runs at 200hz instead of 100hz for some reason
944## Todo:
944- [ ] ADD: PCI Driver (WIP)
944- [ ] ADD: ETH and/or WIFI Support (WIP)
944- [ ] ADD: GUI Support (might never add)
944- [x] ADD: ATA Support (With AliFS)
944- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
