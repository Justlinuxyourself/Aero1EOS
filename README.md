934
934
934<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
934
934<h3 align="center">
934  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
934</h3>
934
934<p align="center">
934  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
934  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
934</p>
934
934<p align="center">
934  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
934  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
934</p>
934
934---
934> [!WARNING]
934> some features dont work on real hardware, and some are untested.
934
934Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
934
934## 🚀 Key Features
934
934* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
934* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
934* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
934* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
934* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
934
934---
934
934## 🏗️ Technical Architecture
934
934### 1. Boot & Memory
934* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
934* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
934* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
934
934### 2. Clock & Timing
934* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
934* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
934
934### 3. Video & I/O
934* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
934* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
934
934---
934
934## 🛠️ Built-in Shell Commands
934
934| Command | Description |
934| :--- | :--- |
934| `help` | Lists all registered system commands. |
934| `cls` | Clears the notebook screen. |
934| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
934| `uptime` | Show how long Aero1EOS has been running. |
934| `free` | Check dynamic RAM usage (Total, Used, Free). |
934| `timezone` | Adjusts the status bar clock offset in real-time. |
934| `lock` | Manually triggers the hardened lock screen. |
934| `test` | Verifies timer calibration with a 5s countdown. |
934| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
934| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
934| `sleep` | puts system in sleep until a key press |
934| `plane` | shows a art of a plane |
934| `about_dev` | About Developer |
934| `echo`| print text to screen |
934| `ayah` | chooses random ayah from quran and prints it |
934| `verse` | chooses random verse from the bible and prints it |
934| `tdadd` | adds todo to mem |
934| `tdshw` | shows todo from mem |
934| `calc` | Calculator |
934-# many more with the `help` command 🥹
934
934## 📁 Directory Structure
934```text
934.
934├── src
934│   ├── kernel.c           # Main OS Loop & Hardening Logic
934│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
934│   ├── section2_video     # VGA Driver & TTY Management
934│   ├── section3_io        # CMOS & Keyboard Drivers
934│   └── section4_shell     # Shell Logic & Command Library
934└── linker.ld              # Kernel Memory Layout
934```
934## 🍎 Bad Apple!! VGA Demo
934Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
934
934### 🛠 Technical Specifications
934* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
934* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
934* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
934* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
934
934## 📜 Licensing & Credits
934
934### Kernel & Operating System
934Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
934
934By using, modifying, or distributing this software, you agree to the following terms:
934* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
934* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
934
934> [!CAUTION]
934> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
934
934### Demo Assets
934* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
934
934## Bugs:
934- Beep stutters
934- pit timer runs at 200hz instead of 100hz for some reason
934## Todo:
934- [ ] ADD: PCI Driver (WIP)
934- [ ] ADD: ETH and/or WIFI Support (WIP)
934- [ ] ADD: GUI Support (might never add)
934- [x] ADD: ATA Support (With AliFS)
934- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
