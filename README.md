# Haxchi PC Installer
## Usage
### Preparing the USB
1. It is recommended to use a clean USB and format it for this process. The installer is expermintal and may corrupt the file system.
2. Buy and install one of the supported DS VC games, and move it to the usb.
### Installing
Choose one of the following options:
#### Option 1: Direct install to the USB (faster but more risky)
1. Connect the USB to your PC, and find out the drive index:  
  Open PowerShell (```WinKey + R```, enter ```powershell```)  
  Run the command ```Get-WmiObject Win32_DiskDrive```  
  Find the DeviceID for your USB (Example: \\.\PHYSICALDRIVE3)  
2. Open command line with admin privileges (Start Menu -> Search for "Command Prompt" -> Right click wnd "Run as Administrator")  
3. Go to the directory with the tool:  
  ```cd <where you extracted the files from the zip>```  
4. Run the installer: (Don't forget to replace \\.\PHYSICALDRIVE3 with the correct device)  
  ```haxchi-pc-installer --image \\.\PHYSICALDRIVE3 --otp otp.bin --seeprom seeprom.bin```  
  And follow the instructions.

#### Option 2: Install on an image (safer)
1. Dump an image of the USB device with tool like Win32DiskImager.
2. It is recommended to backup the image.
3. Open ccommand line (```WinKey + R```, enter ```cmd```)
4. Go to the directory with the tool:  
  ```cd <where you extracted the files from the zip>```  
5. Run the installer:
  ```haxchi-pc-installer --image <dumped image file> --otp otp.bin --seeprom seeprom.bin```  
  And follow the instructions.
6. Flash the image back to the USB.

### Finishing
Now just connect the USB back to the Wii U. If you run the game HBL should be opened instead.  
Now with HBL access it is recommended to move the game back to the NAND and reinstall haxchi again with the regular Wii U installer.


## Build
For the roms, you need to build this:  
https://github.com/FIX94/haxchi/tree/master/dsrom  
For the binary, clone the repository recursively, and build (with .sln or Makefile)
```
git clone https://github.com/koolkdev/haxchi-pc-installer.git --recursive
```
For more information on the requirements see [wfslib README](https://github.com/koolkdev/wfslib/blob/master/README.md).

