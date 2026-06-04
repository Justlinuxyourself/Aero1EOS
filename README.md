930
930
930<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
930
930<h3 align="center">
930  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
930</h3>
930
930<p align="center">
930  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
930  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
930</p>
930
930<p align="center">
930  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
930  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
930</p>
930
930---
930> [!WARNING]
930> some features dont work on real hardware, and some are untested.
930
930Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
930
930## 🚀 Key Features
930
930* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
930* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
930* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
930* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
930* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
930
930---
930
930## 🏗️ Technical Architecture
930
930### 1. Boot & Memory
930* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
930* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
930* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
930
930### 2. Clock & Timing
930* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
930* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
930
930### 3. Video & I/O
930* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
930* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
930
930---
930
930## 🛠️ Built-in Shell Commands
930
930| Command | Description |
930| :--- | :--- |
930| `help` | Lists all registered system commands. |
930| `cls` | Clears the notebook screen. |
930| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
930| `uptime` | Show how long Aero1EOS has been running. |
930| `free` | Check dynamic RAM usage (Total, Used, Free). |
930| `timezone` | Adjusts the status bar clock offset in real-time. |
930| `lock` | Manually triggers the hardened lock screen. |
930| `test` | Verifies timer calibration with a 5s countdown. |
930| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
930| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
930| `sleep` | puts system in sleep until a key press |
930| `plane` | shows a art of a plane |
930| `about_dev` | About Developer |
930| `echo`| print text to screen |
930| `ayah` | chooses random ayah from quran and prints it |
930| `verse` | chooses random verse from the bible and prints it |
930| `tdadd` | adds todo to mem |
930| `tdshw` | shows todo from mem |
930| `calc` | Calculator |
930-# many more with the `help` command 🥹
930
930## 📁 Directory Structure
930```text
930.
930├── src
930│   ├── kernel.c           # Main OS Loop & Hardening Logic
930│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
930│   ├── section2_video     # VGA Driver & TTY Management
930│   ├── section3_io        # CMOS & Keyboard Drivers
930│   └── section4_shell     # Shell Logic & Command Library
930└── linker.ld              # Kernel Memory Layout
930```
930## 🍎 Bad Apple!! VGA Demo
930Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
930
930### 🛠 Technical Specifications
930* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
930* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
930* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
930* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
930
930## 📜 Licensing & Credits
930
930### Kernel & Operating System
930Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
930
930By using, modifying, or distributing this software, you agree to the following terms:
930* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
930* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
930
930> [!CAUTION]
930> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
930
930### Demo Assets
930* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
930
930## Bugs:
930- Beep stutters
930- pit timer runs at 200hz instead of 100hz for some reason
930## Todo:
930- [ ] ADD: PCI Driver (WIP)
930- [ ] ADD: ETH and/or WIFI Support (WIP)
930- [ ] ADD: GUI Support (might never add)
930- [x] ADD: ATA Support (With AliFS)
930- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
