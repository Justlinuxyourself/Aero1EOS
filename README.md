948
948
948<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
948
948<h3 align="center">
948  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
948</h3>
948
948<p align="center">
948  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
948  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
948</p>
948
948<p align="center">
948  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
948  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
948</p>
948
948---
948> [!WARNING]
948> some features dont work on real hardware, and some are untested.
948
948Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
948
948## 🚀 Key Features
948
948* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
948* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
948* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
948* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
948* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
948
948---
948
948## 🏗️ Technical Architecture
948
948### 1. Boot & Memory
948* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
948* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
948* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
948
948### 2. Clock & Timing
948* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
948* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
948
948### 3. Video & I/O
948* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
948* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
948
948---
948
948## 🛠️ Built-in Shell Commands
948
948| Command | Description |
948| :--- | :--- |
948| `help` | Lists all registered system commands. |
948| `cls` | Clears the notebook screen. |
948| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
948| `uptime` | Show how long Aero1EOS has been running. |
948| `free` | Check dynamic RAM usage (Total, Used, Free). |
948| `timezone` | Adjusts the status bar clock offset in real-time. |
948| `lock` | Manually triggers the hardened lock screen. |
948| `test` | Verifies timer calibration with a 5s countdown. |
948| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
948| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
948| `sleep` | puts system in sleep until a key press |
948| `plane` | shows a art of a plane |
948| `about_dev` | About Developer |
948| `echo`| print text to screen |
948| `ayah` | chooses random ayah from quran and prints it |
948| `verse` | chooses random verse from the bible and prints it |
948| `tdadd` | adds todo to mem |
948| `tdshw` | shows todo from mem |
948| `calc` | Calculator |
948-# many more with the `help` command 🥹
948
948## 📁 Directory Structure
948```text
948.
948├── src
948│   ├── kernel.c           # Main OS Loop & Hardening Logic
948│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
948│   ├── section2_video     # VGA Driver & TTY Management
948│   ├── section3_io        # CMOS & Keyboard Drivers
948│   └── section4_shell     # Shell Logic & Command Library
948└── linker.ld              # Kernel Memory Layout
948```
948## 🍎 Bad Apple!! VGA Demo
948Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
948
948### 🛠 Technical Specifications
948* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
948* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
948* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
948* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
948
948## 📜 Licensing & Credits
948
948### Kernel & Operating System
948Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
948
948By using, modifying, or distributing this software, you agree to the following terms:
948* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
948* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
948
948> [!CAUTION]
948> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
948
948### Demo Assets
948* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
948
948## Bugs:
948- Beep stutters
948- pit timer runs at 200hz instead of 100hz for some reason
948## Todo:
948- [ ] ADD: PCI Driver (WIP)
948- [ ] ADD: ETH and/or WIFI Support (WIP)
948- [ ] ADD: GUI Support (might never add)
948- [x] ADD: ATA Support (With AliFS)
948- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
