# HaPara
**H**ardware **A**bstraction for **Para**llel Computing

## Introductions

This project targets those who are familiar with one parallel programming models (i.g. OpenCL) and want to migrate their projects onto FPGAs for prototyping or research purposes. Essentially, a hardware framework that is compatible with OpenCL programming model is designed on an FPGA board. This tutorial mainly introduces how to generate a hybrid framework including hardware platform and software by using automatic TCL scripts. For more details, please email the author of Hongyuan Ding (hding4@gmail.com) or refer to publications at the end of this tutorial. 

## Requirements 

### Hardware Devices

The project is tested under **Xilinx ZC706** experimental boards. Theoretically, any development boards with a off-chip DDR (namely, PL DDR) can work with this project. But probably you have to modify some TCL codes (e.g. address assignments) by your self. 

### Software Preparation

CentOS 6.6 is tested and it cannot guarantee to run under Windows. Xilinx Vivado/Vivado HLS/Petalinux SDK 2015.4 are used.

## Project Contents
```bash
.
├── hardware
│   ├── hw_apps
│   │   └── vector_add
│   └── ip_repo
│       ├── hapara_axis_barrier_1.0
│       ├── hapara_axis_id_dispatcher_1.0
│       ├── hapara_axis_id_generator_1.0
│       ├── hapara_bram_dma_dup_1.0
│       ├── hapara_bram_dma_switch_1.0
│       ├── hapara_burst_icap
│       └── hapara_lmb_dma_dup
└── software
	├── generic
	│   ├── CL
	│   └── include
	├── microblaze
	│   ├── apps
	│   │   └── vector_add
	│   ├── lscript
	│   ├── mutex_manager
	│   ├── scheduler
	│   └── slave_kernel
	└── petalinux
    	├── apps
    	│   └── autologin
    	├── libs
    	│   ├── libddrmalloc
    	│   ├── libelfmb
    	│   └── libregister
    	└── modules
        	├── modddrmalloc
        	├── modmutex
        	└── modregister
```

`software` and `hardware` parts consist of the whole project. 

Within the `hardware` part, Vivado HLS source files are located under `hw_apps`, and user hardware IPs for Vivado are located under `ip_repo`.

`software` part is divided into two components: MicroBlaze and PetaLinux project source files. The shared including files are placed under `generic`. For a hybrid running mode in HaPara (both software and hardware kernels are used in the hardware framework), one Vivado HLS project needs another MicroBlaze project (e.g. `hardware/hw_apps/vector_add` and `software/microblaze/apps/vector_add`). Otherwise, they don't have to be one-to-one mapping.

## TCL Usage

There are mainly four separate scripts to help you build the project.

- `hls_config.tcl`: Create Vivado HLS projects, and export IPs and dcp files. This command will generate a folder named `resource` to hold exported IPs and dcp files.
- `generate.tcl`: Generate the hardware platform, bitstream files and SDK projects. If there are hardware kernels within the project configuration, corresponding partial reconfiguration bitstreams are also generated.
- `sdk_config.tcl`: Create all MicroBlaze projects and generate `elf` files.
- `petalinux_config.sh`: Create petalinux projects along with modules, libs, and apps.

## Known Issues and Bugs
- When you have multiple hardware groups (4g2s8h) with PR enabled, and launch multiple applications with only one software group one by one, one group scheduler may stuck at `for_each_valid` and cannot release the lock causing others cannot request the mutex.
- The item ID number cannot be equal to the number of PEs within one group, which may cause the PEs cannot get proper item IDs. Multiple numbers would be best.

## Acknowledgement

Please cite the following publication if you have refer this work in your publications.

- Hongyuan Ding, Miaoqing Huang, **Exploiting Hardware Abstraction for Hybrid Parallel Computing Framework**, in Proceedings of 2015 International Conference on ReConFigurable Computing and FPGAs (ReConFig'15), pp.1-7, Cancun, Mexico, December 7-9, 2015.

  ```latex
  @INPROCEEDINGS{7393305, 
  author={H. Ding and M. Huang}, 
  booktitle={2015 International Conference on ReConFigurable Computing and FPGAs (ReConFig)}, 
  title={Exploiting hardware abstraction for hybrid parallel computing framework}, 
  year={2015}, 
  pages={1-7}, 
  month={Dec},}
  ```



