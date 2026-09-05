# Documentation

The LOPK kernel is a simple kernel for x86-64 UEFI machines.
I mainly used qemu for my tests so real machines might have bugs

## features
- Has interrupt 0x50 providing fetures to user mode alltough user mode (ring 3) isnt set up yet
- int 0x50 provides the folllowing:
- Simple video features printf print_char clear and putp
- Memory allocation without freeing
- FIle operations for FAT 32
- ATA dirvers
- Reaidng and writing hardware ports for now only ps/2 keyboard is probably only useful case
