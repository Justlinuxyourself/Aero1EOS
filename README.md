936
936
936<h1 align="center">💻 Aero1EOS 4.0 - Notebook Edition</h1>
936
936<h3 align="center">
936  Total Development Commits: <!--COMMIT_COUNT-->926<!--/COMMIT_COUNT-->
936</h3>
936
936<p align="center">
936  <img src="https://img.shields.io/badge/Built%20With-C-A8B9CC?style=flat-square&logo=c&logoColor=white" alt="C">
936  <img src="https://img.shields.io/badge/Built%20With-Assembly-62A106?style=flat-square&logo=assemblyscript&logoColor=white" alt="ASM">
936</p>
936
936<p align="center">
936  <strong>A custom-built, 64-bit "Notebook" style operating system.</strong><br>
936  Featuring a hardened security model, multi-terminal interface, and real-time hardware integration.
936</p>
936
936---
936> [!WARNING]
936> some features dont work on real hardware, and some are untested.
936
936Aero1EOS 4 is a custom-built, 64-bit "Notebook" style operating system. It features a hardened security model, a multi-terminal interface (TTY), and a real-time status bar calibrated for high-precision timing.
936
936## 🚀 Key Features
936
936* **64-bit Long Mode:** Boots from 32-bit Multiboot 2 into a fully functional 64-bit environment with identity-mapped paging.
936* **Hardened Security:** Integrated `lock_system_hardened` routine that stores failed login attempts in CMOS to prevent unauthorized access across reboots. (PASSWORD: Ali123)
936* **Multi-TTY Support:** Supports 10 independent virtual terminals (TTY0-TTY9) accessible via `Ctrl + Alt + F1-F10`.
936* **Real-Time Status Bar:** A persistent 25th-row UI showing Date, Time (12h format), and active TTY ID, driven by CMOS data.
936* **Dynamic Shell:** Features command tab-completion, system diagnostics (`neofetch`), and real-time memory tracking.
936
936---
936
936## 🏗️ Technical Architecture
936
936### 1. Boot & Memory
936* **Kernel Entry:** Written in Assembly (`boot.asm`), it builds a 4-level paging hierarchy: PML4 -> PDPT -> PDT.
936* **Huge Pages:** Identity maps the first 10MB of RAM using 2MB "Huge Pages" to simplify the initial memory map.
936* **Heap Manager:** A dynamic memory allocator starting after the kernel (`kernel_end`) to avoid overlapping kernel code when something is added.
936
936### 2. Clock & Timing
936* **PIT Frequency:** The Programmable Interval Timer is calibrated to **200Hz** (divisor 11931) for uptime and sleep functions (for some reason it runs at 200hz).
936* **Polling Loop:** The kernel uses a hardware polling loop (`timer_wait_tick`) to ensure the clock updates every 100 ticks without interrupting user input.
936
936### 3. Video & I/O
936* **VGA Driver:** Manages an 80x25 text buffer at `0xB8000` with custom "Notebook Yellow" styling (0x1E).
936* **CMOS Integration:** Directly communicates with hardware ports `0x70/0x71` to retrieve real-time clock data and track security strikes.
936
936---
936
936## 🛠️ Built-in Shell Commands
936
936| Command | Description |
936| :--- | :--- |
936| `help` | Lists all registered system commands. |
936| `cls` | Clears the notebook screen. |
936| `neofetch` | Displays CPU vendor, RAM usage, and OS mode. |
936| `uptime` | Show how long Aero1EOS has been running. |
936| `free` | Check dynamic RAM usage (Total, Used, Free). |
936| `timezone` | Adjusts the status bar clock offset in real-time. |
936| `lock` | Manually triggers the hardened lock screen. |
936| `test` | Verifies timer calibration with a 5s countdown. |
936| `beep` | Plays a system alert sound through the PC Speaker. (has a bug, check **Bugs** section.)|
936| `twins` | Shows my twins names bc they are my lovely twins 🥹🥹🥹|
936| `sleep` | puts system in sleep until a key press |
936| `plane` | shows a art of a plane |
936| `about_dev` | About Developer |
936| `echo`| print text to screen |
936| `ayah` | chooses random ayah from quran and prints it |
936| `verse` | chooses random verse from the bible and prints it |
936| `tdadd` | adds todo to mem |
936| `tdshw` | shows todo from mem |
936| `calc` | Calculator |
936-# many more with the `help` command 🥹
936
936## 📁 Directory Structure
936```text
936.
936├── src
936│   ├── kernel.c           # Main OS Loop & Hardening Logic
936│   ├── section1_cpu       # Boot, Timer, Speaker, & Heap
936│   ├── section2_video     # VGA Driver & TTY Management
936│   ├── section3_io        # CMOS & Keyboard Drivers
936│   └── section4_shell     # Shell Logic & Command Library
936└── linker.ld              # Kernel Memory Layout
936```
936## 🍎 Bad Apple!! VGA Demo
936Aero1EOS 4 includes a built-in "Bad Apple!!" ASCII animation demo to showcase the stability of the kernel's high-speed VGA memory mapping and the **Notebook-Edition** independent TTY system.
936
936### 🛠 Technical Specifications
936* **Resolution:** 80x24 (Workspace mode) (24 BC OF STATUS BAR!!!(
936* **Frame Rate:** ~10-15 FPS (Adjustable via kernel delay loop)
936* **Protected Status Bar:** The 25th row remains fully functional, displaying the real-time CMOS clock and active TTY ID without flickering during playback.
936* **Memory Mapping:** Direct writes to `0xB8000` using a high-performance DMA-simulated loop.
936
936## 📜 Licensing & Credits
936
936### Kernel & Operating System
936Aero1EOS 4.0 is open-source software licensed under the **MIT License**. 
936
936By using, modifying, or distributing this software, you agree to the following terms:
936* **Attribution Required:** You must preserve the original copyright notice and permission notice in all copies or substantial portions of the software.
936* **No Warranty:** The software is provided "as is", without warranty of any kind. The author is not liable for any damages or data loss resulting from its use.
936
936> [!CAUTION]
936> **Plagiarism Warning:** While the MIT license permits modification and redistribution, removing the developer's name, claiming authorship of the original core components, or rebranding Aero1EOS without explicit attribution violates the license terms. Copyright infringement and license violations will be tracked via Git history and reported publicly on hosting platforms.
936
936### Demo Assets
936* **Bad Apple!! Animation:** Based on *Touhou Project*. These ASCII assets are community-driven and included solely for demonstrating the Aero1EOS 4 video driver performance.
936
936## Bugs:
936- Beep stutters
936- pit timer runs at 200hz instead of 100hz for some reason
936## Todo:
936- [ ] ADD: PCI Driver (WIP)
936- [ ] ADD: ETH and/or WIFI Support (WIP)
936- [ ] ADD: GUI Support (might never add)
936- [x] ADD: ATA Support (With AliFS)
936- [x] FIX: Beep stutter (Doesnt stutter on real hardware)
