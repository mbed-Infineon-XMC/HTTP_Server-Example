# Simple HTTP Server Example

This example shows how you can setup the Ethernet Interface on the XMC4500 Relax Kit to run a simple HTTP server.<br/><br/>


The web-ui provides functionallity to toggle both Led's and see the actual status of the push buttons.
Futhermore, the network time provided by an NTP server can be programmed into the internal RTC!

![](https://github.com/hackdino/mbed_xmc_images/blob/master/http_example_3.png)

* Static IP Address<br/>
Define 'STATIC_IP' and configure IP-Address, Network-Mask and Default Gateway.

**NOTE** The NTP client maybe not work because the connection to the default NTP server is not possible!<br/>
The default server is 2.pool.ntp.org and the default port is 123.<br/>


```
#define STATIC_IP

eth.set_network("192.168.1.40", "255.255.255.0", "192.168.1.2"); //IP-Address, Network-Mask, Gateway
```

**NOTE:** Use this option when connecting directly with the ethernet interface on your PC!<br/>
**NOTE:** Not forget to configure also a static IP-Adress on your PC. See Example:<br/>

![](https://github.com/hackdino/mbed_xmc_images/blob/master/http_example_2.png)

* Dynamic IP Address<br/>
Remove the 'STATIC_IP' to get an IP-Address from an DHCP server in your network.
```
//#define STATIC_IP
```
**NOTE:** Use this option when connectiing the XMC directly to an network switch!<br/>
**NOTE:** Connect the stdio UART to see which IP-Address the XMC gets from the DHCP server! // TX: P0_5, RX: P0_4<br/>

![](https://github.com/hackdino/mbed_xmc_images/blob/master/http_example_1.png)

**NOTE** Wireshark and ICMP (Ping...) are very helpful tools to debug or monitor your application!

* Known Bugs
- Only works if ethernet cable is pluged-in during start-up! Workaround: Create thread to monitor the ethernet link!<br/>
- Click to the 'Toggle Led' button at the same time when the web-page is refreching can cause a missing button request because the TCP-Socket is already in used!

## Step 1: Download mbed CLI

* [Mbed CLI](https://docs.mbed.com/docs/mbed-os-handbook/en/latest/dev_tools/cli/#installing-mbed-cli) - Download and install mbed CLI.

## Step 2: Import Heap_Stack_Statistics Example project

Import Heap_Stack_Statistics Example project from GitHub.

```
mbed import https://github.com/mbed-Infineon-XMC/HTTP_Server-Example.git
```

## Step 3: Install ARM GCC toolchain

* [GNU ARM toolchain](https://launchpad.net/gcc-arm-embedded) - Download and install the last stable version of the ARM GCC toolchain.
* Open the file "mbed_settings.py" and add the ARM GCC install path.

Example:
```
#GCC_ARM_PATH = "home/bin/arm_gcc_toolchain/gcc-arm-none-eabi-5_4-2016q2/arm-none-eabi/bin"
```

## Step 4: Compile project

Navigate into the project folder and execute the following command:
```
mbed compile -m XMC_4500_RELAX_KIT -t GCC_ARM
```
mbed creates a BUID directory where you can find the executables (bin, elf, hex ...).

## Step 6: Flash to board

* [Segger JLink](https://www.segger.com/downloads/jlink) - Install the JLink software for your platform.
* Navigate to the BUILD directory and execute the following JLinkExe commands.
```
$ JLinkExe
J-LINK> device xmc4500-1024
J-LINK> h
J-Link> loadfile HTTP_Server-Example.git.hex
J-Link> r
J-Link> g
```
* Choose SWD, 4000kHz as interface settings!!


