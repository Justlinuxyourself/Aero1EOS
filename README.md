928
928
928<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
928
928<h3 align="center">
928  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
928</h3>
928
928<p align="center">
928  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
928  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
928</p>
928
928<p align="center">
928  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
928  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
928</p>
928
928---
928> [!WARNING]
928> some features dont work on real hardware, and some are untested.
928
928Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
928
928## 🚀 Key Features
928
928* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
928* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
928* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
928* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
928* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
928
928---
928
928## 🏗️ Technical Architecture
928
928### 1. Boot & Memory
928* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
928* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
928* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
928
928### 2. Clock & Timing
928* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
928* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
928
928### 3. Video & I/O
928* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
928* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
928
928---
928
928## 🛠️ Built-in Shell Commands
928
928| Command | Description |
928| :--- | :--- |
928| `help` | Lists all registered system commands. |
928| `cls` | Clears the notebook screen. |
928| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
928| `uptime` | Show how long Aero1EOS has been running. |
928| `free` | Check dynamic RAM usage (Total, Used, Free). |
928| `timezone` | Adjusts the status bar clock offset in real-time. |
928| `lock` | Manually triggers the hardened lock screen. |
928| `test` | Verifies timer calibration with a 5s countdown. |
928| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
928| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
928| `sleep` | puts system in sleep until a key press |
928| `plane` | shows a art of a plane |
928| `about_dev` | About Developer |
928| `echo`| print text to screen |
928| `ayah` | chooses random ayah from quran and prints it |
928| `verse` | chooses random verse from the bible and prints it |
928| `tdadd` | adds todo to mem |
928| `tdshw` | shows todo from mem |
928| `calc` | Calculator |
928-# many more with the `help` command 🥹
928
928## 📁 Directory Structure
928```text
928.
928├── src
928│   ├── kernel.c           # Main OS Loop & Hardening Logic
928│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
928│   ├── section2_video     # VGA Driver & TTY Management
928│   ├── section3_io        # CMOS & Keyboard Drivers
928│   └── section4_shell     # Shell Logic & Command Library
928└── linker.ld              # Kernel Memory Layout
928```
928## 🍎 Bad Apple!! VGA Demo
928Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
928
928### 🛠 Technical Specifications
928* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
928* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
928* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
928* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
928
928## 📜 Licensing & Credits
928
928### Kernel & Operating System
928Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
928
928By using, modifying, or distributing this software, you agree to the following terms:
928* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
928* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
928
928> [!CAUTION]
928> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
928
928### Demo Assets
928* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
928
928## Bugs:
928- Beep stutters
928- pit timer runs at 200hz instead of 100hz for some reason
928## Todo:
928- [ ] ADD: PCI Driver (WIP)
928- [ ] ADD: ETH and/or WIFI Support (WIP)
928- [ ] ADD: GUI Support (might never add)
928- [x] ADD: ATA Support (With AliFS)
928- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
