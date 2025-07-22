# Hyper-Spiral-GP

This is a Dreamcast project built using KallistiOS (KOS) and a custom Raylib port for the Dreamcast.

## Dependencies

To build and run this project, you will need the following:

1.  **KallistiOS (KOS):** A complete [KOS development environment](https://github.com/KallistiOS/KallistiOS) set up and configured. This includes the KOS toolchain (gcc, binutils, etc.) and the KOS base libraries.
    *   Ensure your `KOS_BASE` environment variable is set to your KOS installation directory.
    *   Ensure your `KOS_PORTS` environment variable is set to your KOS ports directory.

2.  **Raylib4Dreamcast:** [A Dreamcast-specific port of the Raylib library](https://github.com/raylib4Consoles/raylib4Dreamcast). This project expects the Raylib headers to be available in `${KOS_PORTS}/include/raylib`. You will need to build and install this into your KOS ports.

4.  **mkdcdisk:** A utility for creating Dreamcast CDI images from KOS-built `.elf` files and romdisks. This is typically part of the KOS tools or can be built separately.

5.  **mksdiso (Linux):** [This toolkit](https://github.com/Nold360/mksdiso?tab=readme-ov-file#mksdiso) is a collection of free tools & scripts (for Linux), that can be used to create SDISO-Files which can run using Dreamshell on a Sega Dreamcast. Or burn a CDI-Image to disc..

## Building the Project

1.  **Set up KOS Environment:**
    Ensure your KOS environment variables (`KOS_BASE`, `KOS_PORTS`) are correctly set in your shell.

2.  **Build the Executable:**
    Navigate to the project's root directory and run `make`:
    ```bash
    make
    ```
    This will compile the source files and link them into `Hyper-Spiral-GP.elf`.

3.  **Create the CDI Image:**
    After a successful build, you can create the CDI image using `mkdcdisc`. The `Makefile` handles the romdisk creation.
    ```bash
    mkdcdisc -e Hyper-Spiral-GP.elf -o Hyper-Spiral-GP.cdi
    ```
    This command will create `Hyper-Spiral-GP.cdi` from your compiled ELF and romdisk. You can find more information about `mkdcdisc` [here](https://gitlab.com/simulant/mkdcdisc).

## Burning to Disc (Linux)

```bash
/mksdiso/bin/burncdi path/to/game.cdi
```

Once you have the `.cdi` image, you can burn it to a CD-R using `mksdiso` [here](https://github.com/nold360/mksdiso)

## Running on Emulator

You can also run the generated `.elf` or `.cdi` file in a Dreamcast emulator like Flycast.
