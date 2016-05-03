proc hapara_vivado_version_check {} {
    set scripts_vivado_version 2015.4
    set current_vivado_version [version -short]
    if {[string first $scripts_vivado_version $current_vivado_version] == -1} {
       puts "ERROR: This HaPara system has been tested under Vivdao $scripts_vivado_version, and cannot guarantee working correctly under Vivado $current_vivado_version."
       return 0
    }
    return 1
}
proc hapara_create_project {proj_name {fpga_device zc706}} {
    if {$proj_name eq ""} {
        puts "ERROR: Project name cannot be blank."
        return 0;
    }
    set curr_dir $::current_dir
    set proj_dir "$curr_dir/$proj_name"
    if {[file exists $proj_dir]} {
        puts "ERROR: $proj_dir exists. Cannot create project directory."
        return 0
    }
    create_project $proj_name $proj_dir -part xc7z045ffg900-2
    set_property board_part xilinx.com:zc706:part0:1.2 [current_project]
    set_property coreContainer.enable 1 [current_project]
    return 1
}

proc hapara_create_bd {{bd_name system}} {
    create_bd_design $bd_name
    current_bd_design $bd_name
    return 1
}

proc hapara_update_ip_repo {repo_dir resource_hls} {
    if {$repo_dir eq ""} {
        puts "ERROR: IP repository path cannot be blank."
        return 0
    }
    set repo_path ""
    # Set repository path.
    # May NOT work properly under Windows
    # FIXME
    if {[regexp / $repo_dir]} {
        set repo_path $repo_dir
    } else {
        set repo_path "$::current_dir/$repo_dir"
    }
    if {[file exists $repo_path] == 0} {
        puts "ERROR: IP repository $repo_path does not exist."
        return 0
    }
    set ip ""
    set ip [glob -nocomplain "$repo_path/hapara_*"]
    if {$ip == ""} {
        puts "ERROR: There are not HaPara IPs locating at $repo_path"
        return 0
    }
    set hls_project_dir_list [glob -nocomplain -type d "$resource_hls/*"]
    set ip [concat $ip $hls_project_dir_list]

    set_property ip_repo_paths $ip [current_project]
    update_ip_catalog
    return 1
}

################################################################################
# Create normal MicroBlaze local memory
################################################################################
proc create_hier_cell_mb_local_memory {parentCell nameHier} {
    if { $parentCell eq "" || $nameHier eq "" } {
        puts "ERROR: create_hier_cell_mutex_manager_local_memory() - Empty argument(s)!"
        return 0
    }
    set parentObj [get_bd_cells $parentCell]
    if { $parentObj == "" } {
        puts "ERROR: Unable to find parent cell <$parentCell>!"
        return 0
    }
    set parentType [get_property TYPE $parentObj]
    if {$parentType ne "hier"} {
        puts "ERROR: Type of parent <$parentObj> is expected to be <hier>."
        return 0
    }
    set oldCurInst [current_bd_instance .]
    current_bd_instance $parentObj

    # Create cell and set as current instance
    set hier_obj [create_bd_cell -type hier $nameHier]
    current_bd_instance $hier_obj

    # Create interface pins
    create_bd_intf_pin -mode MirroredMaster -vlnv xilinx.com:interface:lmb_rtl:1.0 DLMB
    create_bd_intf_pin -mode MirroredMaster -vlnv xilinx.com:interface:lmb_rtl:1.0 ILMB

    # Create pins
    create_bd_pin -dir I -type clk LMB_Clk
    create_bd_pin -dir I -from 0 -to 0 -type rst SYS_Rst

    # Create instance: dlmb_bram_if_cntlr, and set properties
    set dlmb_bram_if_cntlr [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:* dlmb_bram_if_cntlr ]
    set_property -dict [ list \
        CONFIG.C_ECC {0} \
    ] $dlmb_bram_if_cntlr

    # Create instance: dlmb_v10, and set properties
    set dlmb_v10 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_v10:* dlmb_v10 ]

    # Create instance: ilmb_bram_if_cntlr, and set properties
    set ilmb_bram_if_cntlr [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:* ilmb_bram_if_cntlr ]
    set_property -dict [ list \
        CONFIG.C_ECC {0} \
    ] $ilmb_bram_if_cntlr

    # Create instance: ilmb_v10, and set properties
    set ilmb_v10 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_v10:* ilmb_v10 ]

    # Create instance: lmb_bram, and set properties
    set lmb_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:* lmb_bram ]
    set_property -dict [ list \
        CONFIG.Memory_Type {True_Dual_Port_RAM} \
        CONFIG.use_bram_block {BRAM_Controller} \
    ] $lmb_bram

    # Create interface connections
    connect_bd_intf_net -intf_net mb_dlmb [get_bd_intf_pins DLMB] [get_bd_intf_pins dlmb_v10/LMB_M]
    connect_bd_intf_net -intf_net mb_dlmb_bus [get_bd_intf_pins dlmb_bram_if_cntlr/SLMB] [get_bd_intf_pins dlmb_v10/LMB_Sl_0]
    connect_bd_intf_net -intf_net mb_dlmb_cntlr [get_bd_intf_pins dlmb_bram_if_cntlr/BRAM_PORT] [get_bd_intf_pins lmb_bram/BRAM_PORTA]
    connect_bd_intf_net -intf_net mb_ilmb [get_bd_intf_pins ILMB] [get_bd_intf_pins ilmb_v10/LMB_M]
    connect_bd_intf_net -intf_net mb_ilmb_bus [get_bd_intf_pins ilmb_bram_if_cntlr/SLMB] [get_bd_intf_pins ilmb_v10/LMB_Sl_0]
    connect_bd_intf_net -intf_net mb_ilmb_cntlr [get_bd_intf_pins ilmb_bram_if_cntlr/BRAM_PORT] [get_bd_intf_pins lmb_bram/BRAM_PORTB]

    # Create port connections
    connect_bd_net -net SYS_Rst_1 [get_bd_pins SYS_Rst] [get_bd_pins dlmb_bram_if_cntlr/LMB_Rst] [get_bd_pins dlmb_v10/SYS_Rst] [get_bd_pins ilmb_bram_if_cntlr/LMB_Rst] [get_bd_pins ilmb_v10/SYS_Rst]
    connect_bd_net -net mb_Clk [get_bd_pins LMB_Clk] [get_bd_pins dlmb_bram_if_cntlr/LMB_Clk] [get_bd_pins dlmb_v10/LMB_Clk] [get_bd_pins ilmb_bram_if_cntlr/LMB_Clk] [get_bd_pins ilmb_v10/LMB_Clk]

    # Perform GUI Layout
    regenerate_bd_layout

    # Restore current instance
    current_bd_instance $oldCurInst
}

################################################################################
# Create slave MicroBlaze local memory
################################################################################
proc create_hier_cell_slave_local_memory { parentCell nameHier } {
    if { $parentCell eq "" || $nameHier eq "" } {
        puts "ERROR: create_hier_cell_slave_local_memory() - Empty argument(s)!"
        return 0
    }

    # Get object for parentCell
    set parentObj [get_bd_cells $parentCell]
    if { $parentObj == "" } {
        puts "ERROR: Unable to find parent cell <$parentCell>!"
        return 0
    }

    # Make sure parentObj is hier blk
    set parentType [get_property TYPE $parentObj]
    if { $parentType ne "hier" } {
        puts "ERROR: Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."
        return 0
    }

    # Save current instance; Restore later
    set oldCurInst [current_bd_instance .]

    # Set parent object as current
    current_bd_instance $parentObj

    # Create cell and set as current instance
    set hier_obj [create_bd_cell -type hier $nameHier]
    current_bd_instance $hier_obj

    # Create interface pins
    create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:bram_rtl:1.0 BRAM_PORT
    create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:bram_rtl:1.0 BRAM_PORTA
    create_bd_intf_pin -mode MirroredMaster -vlnv xilinx.com:interface:lmb_rtl:1.0 DLMB
    create_bd_intf_pin -mode MirroredMaster -vlnv xilinx.com:interface:lmb_rtl:1.0 ILMB

    # Create pins
    create_bd_pin -dir I -type clk LMB_Clk
    create_bd_pin -dir I -from 0 -to 0 -type rst SYS_Rst

    # Create instance: dlmb_bram_if_cntlr, and set properties
    set dlmb_bram_if_cntlr [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:* dlmb_bram_if_cntlr ]
    set_property -dict [ list \
        CONFIG.C_ECC {0} \
    ] $dlmb_bram_if_cntlr

    # Create instance: dlmb_bram_if_cntlr1, and set properties
    set dlmb_bram_if_cntlr1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:* dlmb_bram_if_cntlr1 ]
    set_property -dict [ list \
        CONFIG.C_ECC {0} \
    ] $dlmb_bram_if_cntlr1

    # Create instance: dlmb_v10, and set properties
    set dlmb_v10 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_v10:* dlmb_v10 ]
    set_property -dict [ list \
        CONFIG.C_LMB_NUM_SLAVES {2} \
    ] $dlmb_v10

    # Create instance: ilmb_bram_if_cntlr, and set properties
    set ilmb_bram_if_cntlr [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:* ilmb_bram_if_cntlr ]
    set_property -dict [ list \
        CONFIG.C_ECC {0} \
    ] $ilmb_bram_if_cntlr

    # Create instance: ilmb_bram_if_cntlr1, and set properties
    set ilmb_bram_if_cntlr1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_bram_if_cntlr:* ilmb_bram_if_cntlr1 ]
    set_property -dict [ list \
        CONFIG.C_ECC {0} \
    ] $ilmb_bram_if_cntlr1

    # Create instance: ilmb_v10, and set properties
    set ilmb_v10 [ create_bd_cell -type ip -vlnv xilinx.com:ip:lmb_v10:* ilmb_v10 ]
    set_property -dict [ list \
        CONFIG.C_LMB_NUM_SLAVES {2} \
    ] $ilmb_v10

    # Create instance: lmb_bram, and set properties
    set lmb_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:* lmb_bram ]
    set_property -dict [ list \
        CONFIG.Memory_Type {True_Dual_Port_RAM} \
        CONFIG.use_bram_block {BRAM_Controller} \
    ] $lmb_bram

    # Create instance: lmb_bram1, and set properties
    set lmb_bram1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:* lmb_bram1 ]
    set_property -dict [ list \
        CONFIG.Memory_Type {True_Dual_Port_RAM} \
        CONFIG.use_bram_block {BRAM_Controller} \
    ] $lmb_bram1

    # Create interface connections
    connect_bd_intf_net -intf_net Conn [get_bd_intf_pins dlmb_bram_if_cntlr1/SLMB] [get_bd_intf_pins dlmb_v10/LMB_Sl_1]
    connect_bd_intf_net -intf_net Conn1 [get_bd_intf_pins BRAM_PORT] [get_bd_intf_pins dlmb_bram_if_cntlr1/BRAM_PORT]
    connect_bd_intf_net -intf_net Conn2 [get_bd_intf_pins BRAM_PORTA] [get_bd_intf_pins lmb_bram1/BRAM_PORTA]
    connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins ilmb_bram_if_cntlr1/SLMB] [get_bd_intf_pins ilmb_v10/LMB_Sl_1]
    connect_bd_intf_net -intf_net ilmb_bram_if_cntlr1_BRAM_PORT [get_bd_intf_pins ilmb_bram_if_cntlr1/BRAM_PORT] [get_bd_intf_pins lmb_bram1/BRAM_PORTB]
    connect_bd_intf_net -intf_net slave_dlmb [get_bd_intf_pins DLMB] [get_bd_intf_pins dlmb_v10/LMB_M]
    connect_bd_intf_net -intf_net slave_dlmb_bus [get_bd_intf_pins dlmb_bram_if_cntlr/SLMB] [get_bd_intf_pins dlmb_v10/LMB_Sl_0]
    connect_bd_intf_net -intf_net slave_dlmb_cntlr [get_bd_intf_pins dlmb_bram_if_cntlr/BRAM_PORT] [get_bd_intf_pins lmb_bram/BRAM_PORTA]
    connect_bd_intf_net -intf_net slave_ilmb [get_bd_intf_pins ILMB] [get_bd_intf_pins ilmb_v10/LMB_M]
    connect_bd_intf_net -intf_net slave_ilmb_bus [get_bd_intf_pins ilmb_bram_if_cntlr/SLMB] [get_bd_intf_pins ilmb_v10/LMB_Sl_0]
    connect_bd_intf_net -intf_net slave_ilmb_cntlr [get_bd_intf_pins ilmb_bram_if_cntlr/BRAM_PORT] [get_bd_intf_pins lmb_bram/BRAM_PORTB]

    # Create port connections
    connect_bd_net -net SYS_Rst_1 [get_bd_pins SYS_Rst] [get_bd_pins dlmb_bram_if_cntlr/LMB_Rst] [get_bd_pins dlmb_bram_if_cntlr1/LMB_Rst] [get_bd_pins dlmb_v10/SYS_Rst] [get_bd_pins ilmb_bram_if_cntlr/LMB_Rst] [get_bd_pins ilmb_bram_if_cntlr1/LMB_Rst] [get_bd_pins ilmb_v10/SYS_Rst]
    connect_bd_net -net slave_Clk [get_bd_pins LMB_Clk] [get_bd_pins dlmb_bram_if_cntlr/LMB_Clk] [get_bd_pins dlmb_bram_if_cntlr1/LMB_Clk] [get_bd_pins dlmb_v10/LMB_Clk] [get_bd_pins ilmb_bram_if_cntlr/LMB_Clk] [get_bd_pins ilmb_bram_if_cntlr1/LMB_Clk] [get_bd_pins ilmb_v10/LMB_Clk]

    # Perform GUI Layout
    regenerate_bd_layout
    # Restore current instance
    current_bd_instance $oldCurInst
}

################################################################################
# Create hierarchical design: group
################################################################################
# numOfSlave:   The total number of slaves within one group (including MicroBlazes and Hardware IPs)
# numOfHWSlave: The number of hardware IPs within one group
# numOfMBSlave: The number of MicroBlaze slaves within one group
proc create_hier_cell_group {parentCell nameHier numOfSlave numOfHWSlave groupNum {hw_name vector_add} {dma_burst_length 256}} {
    if { $parentCell eq "" || $nameHier eq "" } {
        puts "ERROR: create_hier_cell_group() - Empty argument(s)!"
        return 0
    }

    # Get object for parentCell
    set parentObj [get_bd_cells $parentCell]
    if { $parentObj == "" } {
        puts "ERROR: Unable to find parent cell <$parentCell>!"
        return 0
    }

    # Make sure parentObj is hier blk
    set parentType [get_property TYPE $parentObj]
    if { $parentType ne "hier" } {
        puts "ERROR: Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."
        return 0
    }

    # Save current instance; Restore later
    set oldCurInst [current_bd_instance .]

    # Set parent object as current
    current_bd_instance $parentObj

    # Create cell and set as current instance
    set hier_obj [create_bd_cell -type hier $nameHier]
    current_bd_instance $hier_obj

    # Calculate the number of MicroBlaze slaves
    set numOfMBSlave [expr $numOfSlave-$numOfHWSlave]

    # Create interface pins
    create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:mbdebug_rtl:3.0 DEBUG_scheduler
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set debug_name "DEBUG_s$i"
        create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:mbdebug_rtl:3.0 $debug_name
    }
    #1
    create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M00_AXI_data_ddr
    #3 intercon_mutex_manager
    create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M00_AXI_sche
    #4 intercon_htdt
    create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M01_AXI_sche
    #5 intercon_mdm
    create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 M02_AXI_sche

    # Create pins
    create_bd_pin -dir I -type rst INTERCONNECT_ARESETN
    create_bd_pin -dir I -type clk Clk
    create_bd_pin -dir I -type rst PERIPHERAL_ARESETN
    create_bd_pin -dir I -type rst MB_RESET
    create_bd_pin -dir I -from 0 -to 0 -type rst BUS_STRUCT_RESET

    # Create instance: cdma, and set properties
    set cdma [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_cdma:* cdma ]
    set_property -dict [ list \
        CONFIG.C_INCLUDE_SG {0} \
        CONFIG.C_M_AXI_MAX_BURST_LEN $dma_burst_length \
    ] $cdma

    # Create instance: dma_bram_ctrl, and set properties
    if {$numOfMBSlave > 0} {
        set dma_bram_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:* dma_bram_ctrl ]
        set_property -dict [ list \
            CONFIG.SINGLE_PORT_BRAM {1} \
        ] $dma_bram_ctrl        
    }

    # Create instance: intercon_dma, and set properties
    if {$numOfMBSlave > 0} {
        set intercon_dma [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_dma ]
        set_property -dict [ list \
            CONFIG.NUM_MI {2} \
        ] $intercon_dma        
    } else {
        set intercon_dma [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_dma ]
        set_property -dict [ list \
            CONFIG.NUM_MI {1} \
        ] $intercon_dma   
    }


    # Create instance: hapara_axis_barrier, and set properties
    set hapara_axis_barrier [ create_bd_cell -type ip -vlnv user.org:user:hapara_axis_barrier:* hapara_axis_barrier ]
    set_property -dict [ list \
        CONFIG.NUM_SLAVES [expr "$numOfSlave"] \
    ] $hapara_axis_barrier

    # Create instance: hapara_axis_id_dispatcher, and set properties
    set hapara_axis_id_dispatcher [ create_bd_cell -type ip -vlnv hding:hding.org.hapara:hapara_axis_id_dispatcher:* hapara_axis_id_dispatcher ]
    set_property -dict [ list \
        CONFIG.NUM_SLAVES [expr "$numOfSlave"] \
    ] $hapara_axis_id_dispatcher

    # Create instance: hapara_axis_id_generator, and set properties
    set hapara_axis_id_generator [ create_bd_cell -type ip -vlnv user.org:user:hapara_axis_id_generator:* hapara_axis_id_generator ]

    # Create instance: hapara_lmb_dma_dup, and set properties
    if {$numOfMBSlave > 0} {
        set hapara_lmb_dma_dup [ create_bd_cell -type ip -vlnv user.org:user:hapara_lmb_dma_dup:* hapara_lmb_dma_dup ]
        set_property -dict [ list \
            CONFIG.NUM_SLAVE [expr "$numOfMBSlave"] \
        ] $hapara_lmb_dma_dup        
    }


    # Create instance: local_mem_ctrl, and set properties
    set local_mem_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:* local_mem_ctrl ]
    set_property -dict [ list \
        CONFIG.SINGLE_PORT_BRAM {1} \
    ] $local_mem_ctrl

    # Create instance: local_mem_ctrl_bram, and set properties
    set local_mem_ctrl_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:* local_mem_ctrl_bram ]
    set_property -dict [ list \
        CONFIG.Enable_B {Always_Enabled} \
        CONFIG.Memory_Type {Single_Port_RAM} \
        CONFIG.Port_B_Clock {0} \
        CONFIG.Port_B_Enable_Rate {0} \
        CONFIG.Port_B_Write_Rate {0} \
        CONFIG.Use_RSTB_Pin {false} \
    ] $local_mem_ctrl_bram

    # Create instance: intercon_data, and set properties
    set intercon_data [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_data ]
    set_property -dict [ list \
        CONFIG.NUM_MI {2} \
        CONFIG.NUM_SI [expr "2+$numOfSlave"] \
    ] $intercon_data

    # Create instance: scheduler, and set properties
    set scheduler [ create_bd_cell -type ip -vlnv xilinx.com:ip:microblaze:* scheduler ]
    set_property -dict [ list \
        CONFIG.C_DEBUG_ENABLED {1} \
        CONFIG.C_D_AXI {1} \
        CONFIG.C_D_LMB {1} \
        CONFIG.C_PVR {2} \
        CONFIG.C_PVR_USER1 {0x00} \
        CONFIG.C_PVR_USER2 [format "0x%08X" $groupNum] \
        CONFIG.C_I_LMB {1} \
    ] $scheduler

    # Create instance: scheduler_local_memory
    create_hier_cell_mb_local_memory $hier_obj scheduler_local_memory

    # Create instance: scheduler_axi_periph, and set properties
    set scheduler_axi_periph [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* scheduler_axi_periph ]
    set_property -dict [ list \
        CONFIG.NUM_MI {6} \
    ] $scheduler_axi_periph

    # Create instance: xlconcat, and set properties
    set xlconcat [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:* xlconcat ]
    set_property -dict [ list \
        CONFIG.NUM_PORTS [expr "$numOfSlave"] \
    ] $xlconcat

    # Create Hardware slaves
    for {set i 0} {$i < $numOfHWSlave} {incr i} {
        set hw_ip_name "${hw_name}_s$i"
        set hw [ create_bd_cell -type ip -vlnv xilinx.com:hls:${hw_name}:* $hw_ip_name ]
        set slice [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_register_slice:* "axis_register_slice_$i" ]
        set xlconstant [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconstant:* "xlconstant_$i" ]
        set_property -dict [ list \
            CONFIG.CONST_VAL $i \
            CONFIG.CONST_WIDTH {8} \
        ] $xlconstant
        # Connect interface between them
        connect_bd_intf_net [get_bd_intf_pins "axis_register_slice_$i/M_AXIS"] [get_bd_intf_pins "$hw_ip_name/id"]
        connect_bd_net [get_bd_pins "$hw_ip_name/htID"] [get_bd_pins "xlconstant_$i/dout"]
    }

    # Create MicroBlaze slaves
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        # Create instance: slave_s#, and set properties
        set slave_name "slave_s$i"
        set slave [ create_bd_cell -type ip -vlnv xilinx.com:ip:microblaze:* $slave_name ]
        set_property -dict [ list \
            CONFIG.C_DEBUG_ENABLED {1} \
            CONFIG.C_D_AXI {1} \
            CONFIG.C_D_LMB {1} \
            CONFIG.C_FSL_LINKS {2} \
            CONFIG.C_PVR {2} \
            CONFIG.C_PVR_USER1 {0x00} \
            CONFIG.C_PVR_USER2 [format "0x%08X" [expr $i+$numOfHWSlave]] \
            CONFIG.C_I_AXI {0} \
            CONFIG.C_I_LMB {1} \
        ] $slave

        # Create instance: slave_s#_local_memory
        create_hier_cell_slave_local_memory $hier_obj "${slave_name}_local_memory"
    }


    # Create interface connections
    connect_bd_intf_net -intf_net debug_sche [get_bd_intf_pins DEBUG_scheduler] [get_bd_intf_pins scheduler/DEBUG]
    # Connect interface DEBUG to slave debug ports
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set debug_name "DEBUG_s$i"
        set slave_name "slave_s$i"
        connect_bd_intf_net [get_bd_intf_pins $debug_name] [get_bd_intf_pins "$slave_name/DEBUG"]
    }

    # Connect internal interfaces to outside interfaces
    connect_bd_intf_net [get_bd_intf_pins M00_AXI_data_ddr] [get_bd_intf_pins intercon_data/M01_AXI]
    connect_bd_intf_net [get_bd_intf_pins M00_AXI_sche] [get_bd_intf_pins scheduler_axi_periph/M03_AXI]
    connect_bd_intf_net [get_bd_intf_pins M01_AXI_sche] [get_bd_intf_pins scheduler_axi_periph/M04_AXI]
    connect_bd_intf_net [get_bd_intf_pins M02_AXI_sche] [get_bd_intf_pins scheduler_axi_periph/M05_AXI]

    # Connect scheduler_axi_periph
    connect_bd_intf_net [get_bd_intf_pins scheduler/M_AXI_DP] [get_bd_intf_pins scheduler_axi_periph/S00_AXI]
    connect_bd_intf_net [get_bd_intf_pins hapara_axis_id_generator/S00_AXI] [get_bd_intf_pins scheduler_axi_periph/M00_AXI]
    connect_bd_intf_net [get_bd_intf_pins cdma/S_AXI_LITE] [get_bd_intf_pins scheduler_axi_periph/M01_AXI]
    connect_bd_intf_net [get_bd_intf_pins "intercon_data/S[format "%02d" [expr "1+$numOfSlave"]]_AXI"] [get_bd_intf_pins scheduler_axi_periph/M02_AXI]

    connect_bd_intf_net [get_bd_intf_pins scheduler/DLMB] [get_bd_intf_pins scheduler_local_memory/DLMB]
    connect_bd_intf_net [get_bd_intf_pins scheduler/ILMB] [get_bd_intf_pins scheduler_local_memory/ILMB]

    # Connect dma-related interfaces
    connect_bd_intf_net [get_bd_intf_pins cdma/M_AXI] [get_bd_intf_pins intercon_dma/S00_AXI]
    if {$numOfMBSlave > 0} {
        connect_bd_intf_net [get_bd_intf_pins dma_bram_ctrl/BRAM_PORTA] [get_bd_intf_pins hapara_lmb_dma_dup/bram_ctrl]   
    }
    connect_bd_intf_net [get_bd_intf_pins "intercon_data/S[format "%02d" $numOfSlave]_AXI"] [get_bd_intf_pins intercon_dma/M00_AXI]
    if {$numOfMBSlave > 0} {
        connect_bd_intf_net [get_bd_intf_pins dma_bram_ctrl/S_AXI] [get_bd_intf_pins intercon_dma/M01_AXI]
    }
    
    # Connect barrier master axi-stream to Hardware slave
    for {set i 0} {$i < $numOfHWSlave} {incr i} {
        set barrier_master_name "hapara_axis_barrier/M[format "%02d" $i]_AXIS"
        set hw_ip_name "${hw_name}_s$i" 
        connect_bd_intf_net [get_bd_intf_pins $barrier_master_name] [get_bd_intf_pins "$hw_ip_name/barrier"]
    }
    # Connect barrier master axi-stream to MicroBlaze slave
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set barrier_master_name "hapara_axis_barrier/M[format "%02d" [expr $i+$numOfHWSlave]]_AXIS"
        set slave_name "slave_s$i"
        connect_bd_intf_net [get_bd_intf_pins $barrier_master_name] [get_bd_intf_pins "$slave_name/S0_AXIS"]
    }
    # Connect dispatcher master axi-stream to hardware slave
    for {set i 0} {$i < $numOfHWSlave} {incr i} {
        set dispatcher_master_name "hapara_axis_id_dispatcher/M[format "%02d" $i]_AXIS"
        set hw_ip_name "${hw_name}_s$i"
        connect_bd_intf_net [get_bd_intf_pins $dispatcher_master_name] [get_bd_intf_pins "axis_register_slice_$i/S_AXIS"]
    }
    # Connect dispatcher master axi-stream to slave
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set dispatcher_master_name "hapara_axis_id_dispatcher/M[format "%02d" [expr $i+$numOfHWSlave]]_AXIS"
        set slave_name "slave_s$i"
        connect_bd_intf_net [get_bd_intf_pins $dispatcher_master_name] [get_bd_intf_pins "$slave_name/S1_AXIS"]
    }
    # Connect dispatcher to generator
    connect_bd_intf_net [get_bd_intf_pins hapara_axis_id_dispatcher/S00_AXIS] [get_bd_intf_pins hapara_axis_id_generator/M00_AXIS]

    # Connect lmb DMA duplicator to slave local memory
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set dma_dup_bram_name "hapara_lmb_dma_dup/bram_b$i"
        set slave_name "slave_s$i"
        connect_bd_intf_net [get_bd_intf_pins $dma_dup_bram_name] [get_bd_intf_pins "${slave_name}_local_memory/BRAM_PORTA"]
    }

    # Connect intercon_data related interface
    connect_bd_intf_net [get_bd_intf_pins intercon_data/M00_AXI] [get_bd_intf_pins local_mem_ctrl/S_AXI]
    connect_bd_intf_net [get_bd_intf_pins local_mem_ctrl/BRAM_PORTA] [get_bd_intf_pins local_mem_ctrl_bram/BRAM_PORTA]

    # Connect Hardware slave to intercon_data
    for {set i 0} {$i < $numOfHWSlave} {incr i} {
        set intercon_data_slave_name "intercon_data/S[format "%02d" $i]_AXI"
        set hw_ip_name "${hw_name}_s$i"
        connect_bd_intf_net [get_bd_intf_pins $intercon_data_slave_name] [get_bd_intf_pins "$hw_ip_name/m_axi_data"]
    }

    # Connect MicroBlaze slave local memories and intercon_data to slaves
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set intercon_data_slave_name "intercon_data/S[format "%02d" [expr $i+$numOfHWSlave]]_AXI"
        set dma_dup_bram_name "hapara_lmb_dma_dup/bram_s$i"
        set slave_name "slave_s$i"
        connect_bd_intf_net [get_bd_intf_pins $intercon_data_slave_name] [get_bd_intf_pins "$slave_name/M_AXI_DP"]
        connect_bd_intf_net [get_bd_intf_pins "$slave_name/DLMB"] [get_bd_intf_pins "${slave_name}_local_memory/DLMB"]
        connect_bd_intf_net [get_bd_intf_pins "$slave_name/ILMB"] [get_bd_intf_pins "${slave_name}_local_memory/ILMB"]
        connect_bd_intf_net [get_bd_intf_pins $dma_dup_bram_name] [get_bd_intf_pins "${slave_name}_local_memory/BRAM_PORT"]
    }

    # Create port connections
    # Connect interconnect ARESTN
    connect_bd_net -net INTERCON_ARESETN [get_bd_pins INTERCONNECT_ARESETN] [get_bd_pins intercon_data/ARESETN] [get_bd_pins intercon_dma/ARESETN] [get_bd_pins scheduler_axi_periph/ARESETN]

    # Connect clk and rst that are not related to the number of slaves
    set clk ""
    set rst ""
    lappend clk [get_bd_pins Clk]
    lappend rst [get_bd_pins PERIPHERAL_ARESETN]
    lappend clk [get_bd_pins cdma/m_axi_aclk]
    lappend clk [get_bd_pins cdma/s_axi_lite_aclk]
    lappend rst [get_bd_pins cdma/s_axi_lite_aresetn]
    if {$numOfMBSlave > 0} {
        lappend clk [get_bd_pins dma_bram_ctrl/s_axi_aclk]
        lappend rst [get_bd_pins dma_bram_ctrl/s_axi_aresetn]        
    }
    lappend clk [get_bd_pins hapara_axis_id_dispatcher/s00_axis_aclk]
    lappend clk [get_bd_pins hapara_axis_id_generator/m00_axis_aclk]
    lappend clk [get_bd_pins hapara_axis_id_generator/s00_axi_aclk]
    lappend clk [get_bd_pins intercon_data/ACLK]
    lappend clk [get_bd_pins intercon_data/M00_ACLK]
    lappend clk [get_bd_pins intercon_data/M01_ACLK] 
    lappend rst [get_bd_pins hapara_axis_id_dispatcher/s00_axis_aresetn]
    lappend rst [get_bd_pins hapara_axis_id_generator/m00_axis_aresetn]
    lappend rst [get_bd_pins hapara_axis_id_generator/s00_axi_aresetn]
    lappend rst [get_bd_pins intercon_data/M00_ARESETN]
    lappend rst [get_bd_pins intercon_data/M01_ARESETN]
    lappend clk [get_bd_pins intercon_dma/ACLK]
    lappend clk [get_bd_pins intercon_dma/M00_ACLK]
    lappend clk [get_bd_pins intercon_dma/M01_ACLK]
    lappend clk [get_bd_pins intercon_dma/S00_ACLK]
    lappend clk [get_bd_pins local_mem_ctrl/s_axi_aclk]
    lappend clk [get_bd_pins scheduler/Clk]
    lappend clk [get_bd_pins scheduler_axi_periph/ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/M00_ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/M01_ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/M02_ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/M03_ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/M04_ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/M05_ACLK]
    lappend clk [get_bd_pins scheduler_axi_periph/S00_ACLK]
    lappend clk [get_bd_pins scheduler_local_memory/LMB_Clk]     
    lappend rst [get_bd_pins intercon_dma/M00_ARESETN]
    lappend rst [get_bd_pins intercon_dma/M01_ARESETN]
    lappend rst [get_bd_pins intercon_dma/S00_ARESETN]
    lappend rst [get_bd_pins local_mem_ctrl/s_axi_aresetn]
    lappend rst [get_bd_pins scheduler_axi_periph/M00_ARESETN]
    lappend rst [get_bd_pins scheduler_axi_periph/M01_ARESETN]
    lappend rst [get_bd_pins scheduler_axi_periph/M02_ARESETN]
    lappend rst [get_bd_pins scheduler_axi_periph/M03_ARESETN]
    lappend rst [get_bd_pins scheduler_axi_periph/M04_ARESETN]
    lappend rst [get_bd_pins scheduler_axi_periph/M05_ARESETN]
    lappend rst [get_bd_pins scheduler_axi_periph/S00_ARESETN]
    for {set i 0} {$i < $numOfSlave} {incr i} {
        set barrier_master_clk "hapara_axis_barrier/m[format "%02d" $i]_axis_aclk"
        set barrier_master_rst "hapara_axis_barrier/m[format "%02d" $i]_axis_aresetn"
        lappend clk [get_bd_pins $barrier_master_clk]
        lappend rst [get_bd_pins $barrier_master_rst]
        set dispatcher_master_clk "hapara_axis_id_dispatcher/m[format "%02d" $i]_axis_aclk"
        set dispatcher_master_rst "hapara_axis_id_dispatcher/m[format "%02d" $i]_axis_aresetn"
        lappend clk [get_bd_pins $dispatcher_master_clk]
        lappend rst [get_bd_pins $dispatcher_master_rst]
        set intercon_data_slave_clk "intercon_data/S[format "%02d" $i]_ACLK"
        set intercon_data_slave_rst "intercon_data/S[format "%02d" $i]_ARESETN"
        lappend clk [get_bd_pins $intercon_data_slave_clk]
        lappend rst [get_bd_pins $intercon_data_slave_rst]
    }
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set slave_name "slave_s$i"
        lappend clk [get_bd_pins "$slave_name/Clk"]
        lappend clk [get_bd_pins "${slave_name}_local_memory/LMB_Clk"]       
    }
    for {set i 0} {$i < $numOfHWSlave} {incr i} {
        set hw_ip_name "${hw_name}_s$i"
        lappend clk [get_bd_pins $hw_ip_name/ap_clk]
        lappend rst [get_bd_pins $hw_ip_name/ap_rst_n]
        lappend clk [get_bd_pins axis_register_slice_$i/aclk]
        lappend rst [get_bd_pins axis_register_slice_$i/aresetn]
    }
    # Handle intercon_data slave clk and rst for inter_dma and scheduler_axi_periph
    lappend clk [get_bd_pins "intercon_data/S[format "%02d" $numOfSlave]_ACLK"]
    lappend clk [get_bd_pins "intercon_data/S[format "%02d" [expr "1+$numOfSlave"]]_ACLK"]
    lappend rst [get_bd_pins "intercon_data/S[format "%02d" $numOfSlave]_ARESETN"]
    lappend rst [get_bd_pins "intercon_data/S[format "%02d" [expr "1+$numOfSlave"]]_ARESETN"]
    connect_bd_net -net CLK $clk
    connect_bd_net -net PERI_ARESETN $rst

    # Connect dispatcher and Hardware axi-stream slice ready signal
    for {set i 0} {$i < $numOfHWSlave} {incr i} {
        set dispatcher_axis_ready_name "hapara_axis_id_dispatcher/m[format "%02d" $i]_axis_tready"
        set concat_in_name "xlconcat/In$i"
        set hw_ip_name "${hw_name}_s$i"
        connect_bd_net -net "ReadyHW$i" [get_bd_pins $dispatcher_axis_ready_name] [get_bd_pins "axis_register_slice_$i/s_axis_tready"] [get_bd_pins $concat_in_name]
    }
    # Connect dispatcher and slave ready signal
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set dispatcher_axis_ready_name "hapara_axis_id_dispatcher/m[format "%02d" [expr $i+$numOfHWSlave]]_axis_tready"
        set concat_in_name "xlconcat/In[expr $i+$numOfHWSlave]"
        set slave_name "slave_s$i"
        connect_bd_net -net "ReadyMB$i" [get_bd_pins $dispatcher_axis_ready_name] [get_bd_pins "$slave_name/S1_AXIS_TREADY"] [get_bd_pins $concat_in_name]
    }
    connect_bd_net [get_bd_pins hapara_axis_id_dispatcher/priority_sel] [get_bd_pins xlconcat/dout]

    # Connect MicroBlaze reset and local memory reset
    set mb_reset ""
    set bus_struct_reset ""
    lappend mb_reset [get_bd_pins MB_RESET]
    lappend mb_reset [get_bd_pins scheduler/Reset]
    lappend bus_struct_reset [get_bd_pins BUS_STRUCT_RESET]
    lappend bus_struct_reset [get_bd_pins scheduler_local_memory/SYS_Rst]
    for {set i 0} {$i < $numOfMBSlave} {incr i} {
        set slave_name "slave_s$i"
        lappend mb_reset [get_bd_pins "$slave_name/Reset"]
        lappend bus_struct_reset [get_bd_pins "${slave_name}_local_memory/SYS_Rst"]
    }
    connect_bd_net -net MICROBLAZE_RESET $mb_reset
    connect_bd_net -net BS_RESET $bus_struct_reset

    # Perform GUI Layout
    # regenerate_bd_layout

    # Restore current instance
    current_bd_instance $oldCurInst
}

################################################################################
# Return how many hardware kernel for a given group number 
################################################################################
# group_number:     The index of current group
# number_per_group: Total number of slaves within one group (including mb and hw)
# total_number:     Total number of hardware slaves 
proc hapara_return_hw_number {group_number number_per_group total_number} {
    set q [expr $total_number / $number_per_group]
    set r [expr $total_number % $number_per_group]
    if {$group_number < $q} {
        return $number_per_group
    }
    if {($group_number == $q) && ($r == 0)} {
        return 0
    }
    if {($group_number == $q) && ($r != 0)} {
        return $r
    }
    if {$group_number > $q} {
        return 0
    }
    return 0
}

################################################################################
# Create top design
################################################################################
proc hapara_create_root_design {numOfGroup numOfSlave numOfHWSlave hw_name} {
    set parentCell "/"
    set parentObj [get_bd_cells $parentCell]
    if {$parentObj == ""} {
        puts "ERROR: Unable to find parent cell <$parentCell>."
        return 0
    }
    set parentType [get_property TYPE $parentObj]
    if {$parentType ne "hier"} {
        puts "ERROR: Type of parent <$parentObj> is expected to be <hier>."
        return 0
    }
    set oldCurInst [current_bd_instance .]
    current_bd_instance $parentObj

    set max_hw_slave $::max_hw_slave

    # Create interface ports
    set DDR [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 DDR ]
    set FIXED_IO [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_processing_system7:fixedio_rtl:1.0 FIXED_IO ]
    set ddr3_sdram [ create_bd_intf_port -mode Master -vlnv xilinx.com:interface:ddrx_rtl:1.0 ddr3_sdram ]
    set sys_diff_clock [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_clock_rtl:1.0 sys_diff_clock ]

    # Create ports
    set reset [ create_bd_port -dir I -type rst reset ]
    set_property -dict [ list \
        CONFIG.POLARITY {ACTIVE_HIGH} \
    ] $reset

    # Create instance: htdt_ctrl, and set properties
    set htdt_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:* htdt_ctrl ]
    set_property -dict [ list \
        CONFIG.SINGLE_PORT_BRAM {1} \
    ] $htdt_ctrl

    # Create instance: htdt_ctrl_bram, and set properties
    set htdt_ctrl_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:* htdt_ctrl_bram ]
    set_property -dict [ list \
        CONFIG.Enable_B {Always_Enabled} \
        CONFIG.Memory_Type {Single_Port_RAM} \
        CONFIG.Port_B_Clock {0} \
        CONFIG.Port_B_Enable_Rate {0} \
        CONFIG.Port_B_Write_Rate {0} \
        CONFIG.Use_RSTB_Pin {false} \
    ] $htdt_ctrl_bram

    # Create instance: intercon_htdt, and set properties
    set intercon_htdt [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_htdt ]
    set_property -dict [ list \
        CONFIG.NUM_MI {1} \
        CONFIG.NUM_SI [expr "1+$numOfGroup"] \
    ] $intercon_htdt

    # Create instance: mdm, and set properties
    set mdm [ create_bd_cell -type ip -vlnv xilinx.com:ip:mdm:* mdm ]
    set_property -dict [ list \
        CONFIG.C_MB_DBG_PORTS [expr "1+(1+$numOfSlave)*$numOfGroup-$numOfHWSlave"] \
        CONFIG.C_USE_UART {1} \
    ] $mdm

    # Create instance: intercon_mdm, and set properties
    set intercon_mdm [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_mdm ]
    set_property -dict [ list \
        CONFIG.NUM_MI {1} \
        CONFIG.NUM_SI [expr "$numOfGroup"] \
    ] $intercon_mdm

    # Create instance: mutex_manager, and set properties
    set mutex_manager [ create_bd_cell -type ip -vlnv xilinx.com:ip:microblaze:* mutex_manager ]
    set_property -dict [ list \
        CONFIG.C_DEBUG_ENABLED {1} \
        CONFIG.C_D_AXI {1} \
        CONFIG.C_D_LMB {1} \
        CONFIG.C_I_LMB {1} \
    ] $mutex_manager

    # Create instance: mutex_manager_ctrl, and set properties
    set mutex_manager_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:* mutex_manager_ctrl ]
    set_property -dict [ list \
        CONFIG.SINGLE_PORT_BRAM {1} \
    ] $mutex_manager_ctrl

    # Create instance: mutex_manager_ctrl_bram, and set properties
    set mutex_manager_ctrl_bram [ create_bd_cell -type ip -vlnv xilinx.com:ip:blk_mem_gen:* mutex_manager_ctrl_bram ]
    set_property -dict [ list \
        CONFIG.Enable_B {Always_Enabled} \
        CONFIG.Memory_Type {Single_Port_RAM} \
        CONFIG.Port_B_Clock {0} \
        CONFIG.Port_B_Enable_Rate {0} \
        CONFIG.Port_B_Write_Rate {0} \
        CONFIG.Use_RSTB_Pin {false} \
    ] $mutex_manager_ctrl_bram

    # Create instance: mutex_manager_local_memory
    create_hier_cell_mb_local_memory [current_bd_instance .] mutex_manager_local_memory

    # Create instance: intercon_mutex_manager, and set properties
    set intercon_mutex_manager [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_mutex_manager ]
    set_property -dict [ list \
        CONFIG.NUM_MI {1} \
        CONFIG.NUM_SI [expr "2+$numOfGroup"] \
    ] $intercon_mutex_manager

    # Create instance: mig_7series_0, and set properties
    set mig_7series_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:mig_7series:* mig_7series_0 ]
    set_property -dict [ list \
        CONFIG.BOARD_MIG_PARAM {ddr3_sdram} \
        CONFIG.RESET_BOARD_INTERFACE {reset} \
    ] $mig_7series_0

    # Create instance: intercon_ddr, and set properties
    set intercon_ddr [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_ddr ]
    set_property -dict [ list \
        CONFIG.NUM_MI {2} \
        CONFIG.NUM_SI [expr "1+$numOfGroup"] \
    ] $intercon_ddr

    # Create instance: hapara_burst_icap_0, and set properties
    set hapara_burst_icap_0 [ create_bd_cell -type ip -vlnv user.org:user:hapara_burst_icap:* hapara_burst_icap_0 ]

    # Create instance: icap_ctrl, and set properties
    set icap_ctrl [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_bram_ctrl:* icap_ctrl ]
    set_property -dict [ list \
        CONFIG.SINGLE_PORT_BRAM {1} \
    ] $icap_ctrl

    # Create instance: rst_mig_7series_0_100M, and set properties
    set rst_mig_7series_0_100M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:* rst_mig_7series_0_100M ]

    for {set i 0} {$i < $numOfGroup} {incr i} {
        # Create instance: group
        set group_name "group$i"
        set numhw [hapara_return_hw_number $i $numOfSlave $max_hw_slave]
        create_hier_cell_group [current_bd_instance .] $group_name $numOfSlave $numhw $i $hw_name
    }

    #####################################################################
    # The following components are NOT compatible with none-zynq device
    # FIXME for further none zynq updates
    #####################################################################

    # Create instance: processing_system7_0, and set properties
    set processing_system7_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:* processing_system7_0 ]
    set_property -dict [ list \
        CONFIG.preset {ZC706} \
    ] $processing_system7_0

    # Create instance: intercon_zynq, and set properties
    set intercon_zynq [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:* intercon_zynq ]
    set_property -dict [ list \
        CONFIG.NUM_MI {3} \
    ] $intercon_zynq

    # Create instance: rst_clk_wiz_1_zynq, and set properties
    set rst_clk_wiz_1_zynq [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:* rst_clk_wiz_1_zynq ]
    set_property -dict [ list \
        CONFIG.RESET_BOARD_INTERFACE {Custom} \
        CONFIG.USE_BOARD_FLOW {true} \
    ] $rst_clk_wiz_1_zynq

    ############################################################################
    # Connect interfaces
    ############################################################################
    # Connect mdm related interface 
    set calculative_mb 0
    connect_bd_intf_net [get_bd_intf_pins intercon_mdm/M00_AXI] [get_bd_intf_pins mdm/S_AXI]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        set intercon_mdm_slave_name "intercon_mdm/S[format "%02d" $i]_AXI"
        connect_bd_intf_net [get_bd_intf_pins "$group_name/M02_AXI_sche"] [get_bd_intf_pins $intercon_mdm_slave_name]
        # set mdm_debug_sche_name "mdm/MBDEBUG_[expr "$i*($numOfSlave+1)"]"
        set mdm_debug_sche_name "mdm/MBDEBUG_$calculative_mb"
        set num_hw_per_group [hapara_return_hw_number $i $numOfSlave $max_hw_slave]
        set num_mb_per_group [expr $numOfSlave - $num_hw_per_group]
        connect_bd_intf_net [get_bd_intf_pins $group_name/DEBUG_scheduler] [get_bd_intf_pins $mdm_debug_sche_name]
        for {set j 0} {$j < $num_mb_per_group} {incr j} {
            # set mdm_debug_slave_name "mdm/MBDEBUG_[expr "$i*($numOfSlave+1)+1+$j"]"
            set mdm_debug_slave_name "mdm/MBDEBUG_[expr $calculative_mb+$j+1]"
            set group_slave_debug_name "$group_name/DEBUG_s$j"
            connect_bd_intf_net [get_bd_intf_pins $group_slave_debug_name] [get_bd_intf_pins $mdm_debug_slave_name]
        }
        set calculative_mb [expr $calculative_mb+$num_mb_per_group+1]
    }
    connect_bd_intf_net [get_bd_intf_pins "mdm/MBDEBUG_[expr "($numOfSlave+1)*$numOfGroup-$numOfHWSlave"]"] [get_bd_intf_pins mutex_manager/DEBUG]

    # Connect intercon_ddr related interfaces
    connect_bd_intf_net [get_bd_intf_pins intercon_ddr/M00_AXI] [get_bd_intf_pins mig_7series_0/S_AXI]
    connect_bd_intf_net [get_bd_intf_pins intercon_ddr/M01_AXI] [get_bd_intf_pins icap_ctrl/S_AXI]
    connect_bd_intf_net [get_bd_intf_pins hapara_burst_icap_0/bram_ctrl] [get_bd_intf_pins icap_ctrl/BRAM_PORTA]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        connect_bd_intf_net [get_bd_intf_pins "$group_name/M00_AXI_data_ddr"] [get_bd_intf_pins "intercon_ddr/S[format "%02d" $i]_AXI"]
    }
    connect_bd_intf_net [get_bd_intf_pins intercon_ddr/S[format "%02d" $numOfGroup]_AXI] [get_bd_intf_pins intercon_zynq/M00_AXI]

    # Connect intercon_mutex_manager and mutex_manager related interfaces
    connect_bd_intf_net [get_bd_intf_pins intercon_mutex_manager/M00_AXI] [get_bd_intf_pins mutex_manager_ctrl/S_AXI]
    connect_bd_intf_net [get_bd_intf_pins mutex_manager_ctrl/BRAM_PORTA] [get_bd_intf_pins mutex_manager_ctrl_bram/BRAM_PORTA]
    connect_bd_intf_net [get_bd_intf_pins htdt_ctrl/BRAM_PORTA] [get_bd_intf_pins htdt_ctrl_bram/BRAM_PORTA]
    connect_bd_intf_net [get_bd_intf_pins htdt_ctrl/S_AXI] [get_bd_intf_pins intercon_htdt/M00_AXI]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        set intercon_mutex_manager_slave_name "intercon_mutex_manager/S[format "%02d" $i]_AXI"
        set intercon_htdt_slave_name "intercon_htdt/S[format "%02d" $i]_AXI"
        connect_bd_intf_net [get_bd_intf_pins "$group_name/M00_AXI_sche"] [get_bd_intf_pins $intercon_mutex_manager_slave_name]
        connect_bd_intf_net [get_bd_intf_pins "$group_name/M01_AXI_sche"] [get_bd_intf_pins $intercon_htdt_slave_name]
    }
    connect_bd_intf_net [get_bd_intf_pins "intercon_mutex_manager/S[format "%02d" $numOfGroup]_AXI"] [get_bd_intf_pins intercon_zynq/M01_AXI]
    connect_bd_intf_net [get_bd_intf_pins "intercon_mutex_manager/S[format "%02d" [expr "$numOfGroup+1"]]_AXI"] [get_bd_intf_pins mutex_manager/M_AXI_DP]  
    connect_bd_intf_net [get_bd_intf_pins "intercon_htdt/S[format "%02d" $numOfGroup]_AXI"] [get_bd_intf_pins intercon_zynq/M02_AXI]

    # Connect mutex_manager related local memory bus
    connect_bd_intf_net [get_bd_intf_pins mutex_manager/DLMB] [get_bd_intf_pins mutex_manager_local_memory/DLMB]
    connect_bd_intf_net [get_bd_intf_pins mutex_manager/ILMB] [get_bd_intf_pins mutex_manager_local_memory/ILMB]

    # Connect intercon_zynq related interfaces
    connect_bd_intf_net [get_bd_intf_pins intercon_zynq/S00_AXI] [get_bd_intf_pins processing_system7_0/M_AXI_GP0]

    # Connect fixed interfaces
    connect_bd_intf_net [get_bd_intf_ports ddr3_sdram] [get_bd_intf_pins mig_7series_0/DDR3]
    connect_bd_intf_net [get_bd_intf_ports DDR] [get_bd_intf_pins processing_system7_0/DDR]
    connect_bd_intf_net [get_bd_intf_ports FIXED_IO] [get_bd_intf_pins processing_system7_0/FIXED_IO]
    connect_bd_intf_net [get_bd_intf_ports sys_diff_clock] [get_bd_intf_pins mig_7series_0/SYS_CLK]

    # Create port connections
    # Connect mig_7series_0_100M related ports which is generated by the mig_7series_0
    connect_bd_net -net mig_7series_0_mmcm_locked [get_bd_pins mig_7series_0/mmcm_locked] [get_bd_pins rst_mig_7series_0_100M/dcm_locked]
    connect_bd_net -net mig_7series_0_ui_clk [get_bd_pins intercon_ddr/M00_ACLK] [get_bd_pins mig_7series_0/ui_clk] [get_bd_pins rst_mig_7series_0_100M/slowest_sync_clk]
    connect_bd_net -net mig_7series_0_ui_clk_sync_rst [get_bd_pins mig_7series_0/ui_clk_sync_rst] [get_bd_pins rst_mig_7series_0_100M/ext_reset_in] 
    connect_bd_net -net mig_7series_0_100M_peripheral_aresetn [get_bd_pins intercon_ddr/M00_ARESETN] [get_bd_pins mig_7series_0/aresetn] [get_bd_pins rst_mig_7series_0_100M/peripheral_aresetn]

    # Connect mig_7series_0 related ports
    connect_bd_net [get_bd_ports reset] [get_bd_pins mig_7series_0/sys_rst]

    # Connect rst_clk_wiz_1_zynq related ports
    connect_bd_net [get_bd_pins mdm/Debug_SYS_Rst] [get_bd_pins rst_clk_wiz_1_zynq/mb_debug_sys_rst]
    connect_bd_net [get_bd_pins processing_system7_0/FCLK_RESET0_N] [get_bd_pins rst_clk_wiz_1_zynq/ext_reset_in]

    # Connect mb_reset
    set mb_reset ""
    lappend mb_reset [get_bd_pins rst_clk_wiz_1_zynq/mb_reset]
    lappend mb_reset [get_bd_pins mutex_manager/Reset]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        lappend mb_reset [get_bd_pins "$group_name/MB_RESET"]
    }
    connect_bd_net -net rst_clk_wiz_1_zynq_mb_reset $mb_reset

    # Connect bus_struct_reset
    set bus_struct_reset ""
    lappend bus_struct_reset [get_bd_pins rst_clk_wiz_1_zynq/bus_struct_reset]
    lappend bus_struct_reset [get_bd_pins mutex_manager_local_memory/SYS_Rst]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        lappend bus_struct_reset [get_bd_pins "$group_name/BUS_STRUCT_RESET"]
    }
    connect_bd_net -net rst_clk_wiz_1_zynq_bus_struct_reset $bus_struct_reset

    # Connect slowest_sync_clk ports
    set slowest_sync_clk ""
    lappend slowest_sync_clk [get_bd_pins rst_clk_wiz_1_zynq/slowest_sync_clk]
    lappend slowest_sync_clk [get_bd_pins processing_system7_0/FCLK_CLK0]
    lappend slowest_sync_clk [get_bd_pins processing_system7_0/M_AXI_GP0_ACLK]
    lappend slowest_sync_clk [get_bd_pins mdm/S_AXI_ACLK]
    lappend slowest_sync_clk [get_bd_pins mutex_manager/Clk] 
    lappend slowest_sync_clk [get_bd_pins mutex_manager_ctrl/s_axi_aclk] 
    lappend slowest_sync_clk [get_bd_pins mutex_manager_local_memory/LMB_Clk] 
    lappend slowest_sync_clk [get_bd_pins htdt_ctrl/s_axi_aclk]
    lappend slowest_sync_clk [get_bd_pins icap_ctrl/s_axi_aclk]
    lappend slowest_sync_clk [get_bd_pins intercon_zynq/ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_zynq/S00_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_zynq/M00_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_zynq/M01_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_zynq/M02_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_mutex_manager/ACLK] 
    lappend slowest_sync_clk [get_bd_pins intercon_mutex_manager/M00_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_mdm/ACLK] 
    lappend slowest_sync_clk [get_bd_pins intercon_mdm/M00_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_htdt/ACLK] 
    lappend slowest_sync_clk [get_bd_pins intercon_htdt/M00_ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_ddr/ACLK]
    lappend slowest_sync_clk [get_bd_pins intercon_ddr/M01_ACLK]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        lappend slowest_sync_clk [get_bd_pins "$group_name/Clk"]
        set intercon_ddr_slave_clk "intercon_ddr/S[format "%02d" $i]_ACLK"
        lappend slowest_sync_clk [get_bd_pins $intercon_ddr_slave_clk]
        set intercon_htdt_slave_clk "intercon_htdt/S[format "%02d" $i]_ACLK"
        lappend slowest_sync_clk [get_bd_pins $intercon_htdt_slave_clk]
        set intercon_mdm_slave_clk "intercon_mdm/S[format "%02d" $i]_ACLK"
        lappend slowest_sync_clk [get_bd_pins $intercon_mdm_slave_clk]
        set intercon_mutex_manager_slave_clk "intercon_mutex_manager/S[format "%02d" $i]_ACLK"
        lappend slowest_sync_clk [get_bd_pins $intercon_mutex_manager_slave_clk]
    }
    lappend slowest_sync_clk [get_bd_pins "intercon_ddr/S[format "%02d" $numOfGroup]_ACLK"]
    lappend slowest_sync_clk [get_bd_pins "intercon_htdt/S[format "%02d" $numOfGroup]_ACLK"]
    lappend slowest_sync_clk [get_bd_pins "intercon_mutex_manager/S[format "%02d" $numOfGroup]_ACLK"]
    lappend slowest_sync_clk [get_bd_pins "intercon_mutex_manager/S[format "%02d" [expr "$numOfGroup+1"]]_ACLK"]
    connect_bd_net -net slowest_sync_Clk $slowest_sync_clk

    # Connect interconnect_aresetn ports
    set interconnect_aresetn ""
    lappend interconnect_aresetn [get_bd_pins rst_clk_wiz_1_zynq/interconnect_aresetn]
    lappend interconnect_aresetn [get_bd_pins intercon_ddr/ARESETN]
    lappend interconnect_aresetn [get_bd_pins intercon_htdt/ARESETN]
    lappend interconnect_aresetn [get_bd_pins intercon_mdm/ARESETN]
    lappend interconnect_aresetn [get_bd_pins intercon_mutex_manager/ARESETN]
    lappend interconnect_aresetn [get_bd_pins intercon_zynq/ARESETN]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        lappend interconnect_aresetn [get_bd_pins "$group_name/INTERCONNECT_ARESETN"]
    }
    connect_bd_net -net rst_clk_wiz_1_zynq_interconnect_aresetn $interconnect_aresetn

    # Connect peripheral_aresetn
    set peripheral_aresetn ""
    lappend peripheral_aresetn [get_bd_pins rst_clk_wiz_1_zynq/peripheral_aresetn]
    lappend peripheral_aresetn [get_bd_pins mutex_manager_ctrl/s_axi_aresetn] 
    lappend peripheral_aresetn [get_bd_pins mdm/S_AXI_ARESETN]
    lappend peripheral_aresetn [get_bd_pins htdt_ctrl/s_axi_aresetn]
    lappend peripheral_aresetn [get_bd_pins icap_ctrl/s_axi_aresetn]
    lappend peripheral_aresetn [get_bd_pins intercon_zynq/S00_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_zynq/M00_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_zynq/M01_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_zynq/M02_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_mutex_manager/M00_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_mdm/M00_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_htdt/M00_ARESETN]
    lappend peripheral_aresetn [get_bd_pins intercon_ddr/M01_ARESETN]
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"
        lappend peripheral_aresetn [get_bd_pins "$group_name/PERIPHERAL_ARESETN"]
        set intercon_ddr_slave_rst "intercon_ddr/S[format "%02d" $i]_ARESETN"
        lappend peripheral_aresetn [get_bd_pins $intercon_ddr_slave_rst]
        set intercon_htdt_slave_rst "intercon_htdt/S[format "%02d" $i]_ARESETN"
        lappend peripheral_aresetn [get_bd_pins $intercon_htdt_slave_rst]
        set intercon_mdm_slave_rst "intercon_mdm/S[format "%02d" $i]_ARESETN"
        lappend peripheral_aresetn [get_bd_pins $intercon_mdm_slave_rst]
        set intercon_mutex_manager_slave_rst "intercon_mutex_manager/S[format "%02d" $i]_ARESETN"
        lappend peripheral_aresetn [get_bd_pins $intercon_mutex_manager_slave_rst]
    }
    lappend peripheral_aresetn [get_bd_pins "intercon_ddr/S[format "%02d" $numOfGroup]_ARESETN"]
    lappend peripheral_aresetn [get_bd_pins "intercon_htdt/S[format "%02d" $numOfGroup]_ARESETN"]
    lappend peripheral_aresetn [get_bd_pins "intercon_mutex_manager/S[format "%02d" $numOfGroup]_ARESETN"]
    lappend peripheral_aresetn [get_bd_pins "intercon_mutex_manager/S[format "%02d" [expr "$numOfGroup+1"]]_ARESETN"]
    connect_bd_net -net rst_clk_wiz_1_zynq_peripheral_aresetn $peripheral_aresetn
 

    # Create address segments
    set mutex_manager_base  "0x40010000"
    set htdt_base           "0x40000000"
    set ddr_base            "0x60000000"
    set local_mem_base      "0xC0000000"
    set dma_elf_base        "0xC2000000"
    set icap_base           "0x42000000"

    set sch_dma_base        "0x44A10000"
    set sch_gen_base        "0x44A00000"
    set sch_mdm_base        "0x41400000"
    # Assign address for mutex_manager
    create_bd_addr_seg -range 0x8000 -offset $mutex_manager_base [get_bd_addr_spaces mutex_manager/Data] [get_bd_addr_segs mutex_manager_ctrl/S_AXI/Mem0] SEG_axi_bram_ctrl_0_Mem0
    create_bd_addr_seg -range 0x8000 -offset 0x0 [get_bd_addr_spaces mutex_manager/Data] [get_bd_addr_segs mutex_manager_local_memory/dlmb_bram_if_cntlr/SLMB/Mem] SEG_dlmb_bram_if_cntlr_Mem
    create_bd_addr_seg -range 0x8000 -offset 0x0 [get_bd_addr_spaces mutex_manager/Instruction] [get_bd_addr_segs mutex_manager_local_memory/ilmb_bram_if_cntlr/SLMB/Mem] SEG_ilmb_bram_if_cntlr_Mem
    
    # Assign address for zynq processing_system7
    create_bd_addr_seg -range 0x8000 -offset $htdt_base [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs htdt_ctrl/S_AXI/Mem0] SEG_htdt_ctrl_Mem0
    create_bd_addr_seg -range 0x20000000 -offset $ddr_base [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs mig_7series_0/memmap/memaddr] SEG_mig_7series_0_memaddr
    create_bd_addr_seg -range 0x8000 -offset $mutex_manager_base [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs mutex_manager_ctrl/S_AXI/Mem0] SEG_mutex_manager_ctrl_Mem0
    create_bd_addr_seg -range 0x800000 -offset $icap_base [get_bd_addr_spaces processing_system7_0/Data] [get_bd_addr_segs icap_ctrl/S_AXI/Mem0] SEG_icap_ctrl_Mem0
    
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set group_name "group$i"

        set num_hw_per_group [hapara_return_hw_number $i $numOfSlave $max_hw_slave]
        set num_mb_per_group [expr $numOfSlave - $num_hw_per_group]
        # CDMA setup
        if {$num_mb_per_group > 0} {
            create_bd_addr_seg -range 0x8000 -offset $dma_elf_base [get_bd_addr_spaces "$group_name/cdma/Data"] [get_bd_addr_segs "$group_name/dma_bram_ctrl/S_AXI/Mem0"] SEG_dma_bram_ctrl_Mem0
        }
        create_bd_addr_seg -range 0x8000 -offset $local_mem_base [get_bd_addr_spaces "$group_name/cdma/Data"] [get_bd_addr_segs "$group_name/local_mem_ctrl/S_AXI/Mem0"] SEG_local_mem_ctrl_Mem0
        create_bd_addr_seg -range 0x20000000 -offset $ddr_base [get_bd_addr_spaces "$group_name/cdma/Data"] [get_bd_addr_segs mig_7series_0/memmap/memaddr] SEG_mig_7series_0_memaddr
        create_bd_addr_seg -range 0x800000 -offset $icap_base [get_bd_addr_spaces "$group_name/cdma/Data"] [get_bd_addr_segs icap_ctrl/S_AXI/Mem0] SEG_icap_ctrl_Mem0

        # Scheduler setup
        create_bd_addr_seg -range 0x10000 -offset $sch_dma_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs "$group_name/cdma/S_AXI_LITE/Reg"] SEG_cdma_Reg
        create_bd_addr_seg -range 0x8000 -offset 0x0 [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs "$group_name/scheduler_local_memory/dlmb_bram_if_cntlr/SLMB/Mem"] SEG_dlmb_bram_if_cntlr_Mem
        create_bd_addr_seg -range 0x10000 -offset $sch_gen_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs "$group_name/hapara_axis_id_generator/S00_AXI/S00_AXI_reg"] SEG_hapara_axis_id_generator_S00_AXI_reg
        create_bd_addr_seg -range 0x8000 -offset $htdt_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs htdt_ctrl/S_AXI/Mem0] SEG_htdt_ctrl_Mem0
        create_bd_addr_seg -range 0x8000 -offset 0x0 [get_bd_addr_spaces "$group_name/scheduler/Instruction"] [get_bd_addr_segs "$group_name/scheduler_local_memory/ilmb_bram_if_cntlr/SLMB/Mem"] SEG_ilmb_bram_if_cntlr_Mem
        create_bd_addr_seg -range 0x8000 -offset $local_mem_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs "$group_name/local_mem_ctrl/S_AXI/Mem0"] SEG_local_mem_ctrl_Mem0
        create_bd_addr_seg -range 0x1000 -offset $sch_mdm_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs mdm/S_AXI/Reg] SEG_mdm_Reg
        create_bd_addr_seg -range 0x20000000 -offset $ddr_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs mig_7series_0/memmap/memaddr] SEG_mig_7series_0_memaddr
        create_bd_addr_seg -range 0x8000 -offset $mutex_manager_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs mutex_manager_ctrl/S_AXI/Mem0] SEG_mutex_manager_ctrl_Mem0
        create_bd_addr_seg -range 0x800000 -offset $icap_base [get_bd_addr_spaces "$group_name/scheduler/Data"] [get_bd_addr_segs icap_ctrl/S_AXI/Mem0] SEG_icap_ctrl_Mem0
        
        for {set j 0} {$j < $num_mb_per_group} {incr j} {
            set slave_name "slave_s$j"
            create_bd_addr_seg -range 0x8000 -offset 0x8000 [get_bd_addr_spaces "$group_name/$slave_name/Data"] [get_bd_addr_segs "$group_name/${slave_name}_local_memory/dlmb_bram_if_cntlr1/SLMB/Mem"] SEG_dlmb_bram_if_cntlr1_Mem
            create_bd_addr_seg -range 0x8000 -offset 0x0 [get_bd_addr_spaces "$group_name/$slave_name/Data"] [get_bd_addr_segs "$group_name/${slave_name}_local_memory/dlmb_bram_if_cntlr/SLMB/Mem"] SEG_dlmb_bram_if_cntlr_Mem
            create_bd_addr_seg -range 0x8000 -offset 0x8000 [get_bd_addr_spaces "$group_name/$slave_name/Instruction"] [get_bd_addr_segs "$group_name/${slave_name}_local_memory/ilmb_bram_if_cntlr1/SLMB/Mem"] SEG_ilmb_bram_if_cntlr1_Mem
            create_bd_addr_seg -range 0x8000 -offset 0x0 [get_bd_addr_spaces "$group_name/$slave_name/Instruction"] [get_bd_addr_segs "$group_name/${slave_name}_local_memory/ilmb_bram_if_cntlr/SLMB/Mem"] SEG_ilmb_bram_if_cntlr_Mem
            create_bd_addr_seg -range 0x8000 -offset $local_mem_base [get_bd_addr_spaces "$group_name/$slave_name/Data"] [get_bd_addr_segs "$group_name/local_mem_ctrl/S_AXI/Mem0"] SEG_local_mem_ctrl_Mem0
            create_bd_addr_seg -range 0x20000000 -offset $ddr_base [get_bd_addr_spaces "$group_name/$slave_name/Data"] [get_bd_addr_segs mig_7series_0/memmap/memaddr] SEG_mig_7series_0_memaddr
            create_bd_addr_seg -range 0x800000 -offset $icap_base [get_bd_addr_spaces "$group_name/$slave_name/Data"] [get_bd_addr_segs icap_ctrl/S_AXI/Mem0] SEG_icap_ctrl_Mem0
        }
        for {set j 0} {$j < $num_hw_per_group} {incr j} {
            set hw_slave_name "${hw_name}_s$j"
            create_bd_addr_seg -range 0x800000 -offset $icap_base [get_bd_addr_spaces "$group_name/$hw_slave_name/Data_m_axi_data"] [get_bd_addr_segs icap_ctrl/S_AXI/Mem0] SEG_icap_ctrl_Mem0
            create_bd_addr_seg -range 0x8000 -offset $local_mem_base [get_bd_addr_spaces "$group_name/$hw_slave_name/Data_m_axi_data"] [get_bd_addr_segs "$group_name/local_mem_ctrl/S_AXI/Mem0"] SEG_local_mem_ctrl_Mem0
            create_bd_addr_seg -range 0x20000000 -offset $ddr_base [get_bd_addr_spaces "$group_name/$hw_slave_name/Data_m_axi_data"] [get_bd_addr_segs mig_7series_0/memmap/memaddr] SEG_mig_7series_0_memaddr            
        }
    }
 
    # Perform GUI Layout
    # regenerate_bd_layout

    # Restore current instance
    current_bd_instance $oldCurInst

    save_bd_design
    return 1
}

################################################################################
# Create HDL wrapper
################################################################################
proc hapara_create_hdl_wrapper {} {
    set project_name [current_project]
    set bd_design_nm [current_bd_design .]
    set curr_dir $::current_dir
    set proj_path "$curr_dir/$project_name"
    set wrapper_name "${bd_design_nm}_wrapper.v"
    generate_target all [get_files "$proj_path/$project_name.srcs/sources_1/bd/$bd_design_nm/$bd_design_nm.bd"]
    # export_ip_user_files -of_objects [get_files "$proj_path/$project_name.srcs/sources_1/bd/$bd_design_nm/$bd_design_nm.bd"] -no_script -force -quiet
    # export_simulation -of_objects [get_files "$proj_path/$project_name.srcs/sources_1/bd/$bd_design_nm/$bd_design_nm.bd"] -directory \
    #     "$proj_path/$project_name.ip_user_files/sim_scripts" -force -quiet
    make_wrapper -files [get_files "$proj_path/$project_name.srcs/sources_1/bd/$bd_design_nm/$bd_design_nm.bd"] -top
    add_files -norecurse "$proj_path/$project_name.srcs/sources_1/bd/$bd_design_nm/hdl/$wrapper_name"
    update_compile_order -fileset sources_1
    update_compile_order -fileset sim_1
    return 1
}
################################################################################
# Generate mmi files for bram and MicroBlaze information
################################################################################
proc hapara_generate_mmi_bram_info {bram type} {
    set temp [get_property bmm_info_memory_device [get_cells $bram]]
    set bmm_info_memory_device [regexp {\[(.+)\]\[(.+)\]} $temp all 1 2]
    if {$type == "bit_lane"} {
        return $1
    } elseif {$type == "range"} {
        return $2
    } else {
        return $all
    }
}
proc hapara_generate_mmi_addspace {fileout brams cell_name begin_addr} {
    set cell_name_bram ""
    for {set i 0} {$i < [llength $brams]} {incr i} {
        if { [regexp -nocase $cell_name [lindex $brams $i]] } {
            lappend cell_name_bram [lindex $brams $i]
        }
    }
    set bram_range 0
    for {set i 0} {$i < [llength $cell_name_bram]} {incr i} {
        set bram_type [get_property REF_NAME [get_cells [lindex $cell_name_bram $i]]]
        if {$bram_type == "RAMB36E1"} {
            set bram_range [expr {$bram_range + 4096}]    
        }
    }
    puts $fileout "  <AddressSpace Name=\"$cell_name\" Begin=\"$begin_addr\" End=\"[expr {$begin_addr+$bram_range-1}]\">"
    set bram [llength $cell_name_bram]
    if {$bram >= 32} {
        set sequence "7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8,23,22,21,20,19,18,17,16,31,30,29,28,27,26,25,24"
        set bus_blocks [expr {$bram / 32}]
    } elseif {$bram >= 16 && $bram < 32} {
        set sequence "7,5,3,1,15,13,11,9,23,21,19,17,31,29,27,25"
        set bus_blocks 1
    } elseif {$bram >= 8 && $bram < 16} {
        set sequence "7,3,15,11,23,19,31,27"
        set bus_blocks 1
    } elseif {$bram >= 4 && $bram < 8} {
        set sequence "7,15,23,31"
        set bus_blocks 1
    } else {
        set sequence "15,31"
        set bus_blocks 1
    }
    set sequence [split $sequence ","]
    for {set b 0} {$b < $bus_blocks} {incr b} {
        puts $fileout "      <BusBlock>"
        for {set i 0} {$i < [llength $sequence]} {incr i} {
            for {set j 0} {$j < [llength $cell_name_bram]} {incr j} {
                set block_start [expr {32768 * $b}]
                set bmm_width [hapara_generate_mmi_bram_info [lindex $cell_name_bram $j] "bit_lane"]
                set bmm_width [split $bmm_width ":"]
                set bmm_msb [lindex $bmm_width 0]
                set bmm_lsb [lindex $bmm_width 1]
                set bmm_range [hapara_generate_mmi_bram_info [lindex $cell_name_bram $j] "range"]
                set split_ranges [split $bmm_range ":"]
                set MSB [lindex $sequence $i]
                if {$MSB == $bmm_msb && $block_start == [lindex $split_ranges 0]} {
                    set bram_type [get_property REF_NAME [get_cells [lindex $cell_name_bram $j]]]
                    set status [get_property STATUS [get_cells [lindex $cell_name_bram $j]]]                                                                     
                    if {$status == "UNPLACED"} {
                        set placed "X0Y0"
                    } else {
                        set placed [get_property LOC [get_cells [lindex $cell_name_bram $j]]]
                        set placed_list [split $placed "_"]
                        set placed [lindex $placed_list 1]
                    }
                    set bram_type [get_property REF_NAME [get_cells [lindex $cell_name_bram $j]]]            
                    if {$bram_type == "RAMB36E1"} {
                        set bram_type "RAMB32"
                    }                                    
                    puts $fileout "        <BitLane MemType=\"$bram_type\" Placement=\"$placed\">"
                    puts $fileout "          <DataWidth MSB=\"$bmm_msb\" LSB=\"$bmm_lsb\"/>"
                    puts $fileout "          <AddressRange Begin=\"[lindex $split_ranges 0]\" End=\"[lindex $split_ranges 1]\"/>"
                    puts $fileout "          <Parity ON=\"false\" NumBits=\"0\"/>"
                    puts $fileout "        </BitLane>"
                }
            }
        }
        puts $fileout "      </BusBlock>"
        puts $fileout "    </AddressSpace>"
    }
}
proc hapara_generate_mmi {numOfGroup numOfSlave} {
    set project_name [current_project]
    set bd_design_nm [current_bd_design .]
    set curr_dir $::current_dir
    set max_hw_slave $::max_hw_slave
    set filename "$curr_dir/$project_name/$project_name.mmi"
    set fileout [open $filename "w"]
    set brams [split [get_cells -hierarchical -filter { PRIMITIVE_TYPE =~ BMEM.bram.* }] " "]

    puts $fileout "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
    puts $fileout "<MemInfo Version=\"1\" Minor=\"0\">"

    # Create AddressSpace for mutex_manager
    set inst_path "/mutex_manager"
    set cell_name "${bd_design_nm}_i/mutex_manager_local_memory/lmb_bram"
    puts $fileout "  <Processor Endianness=\"Little\" InstPath=\"$inst_path\">"
    hapara_generate_mmi_addspace $fileout $brams $cell_name 0
    puts $fileout "  </Processor>"
    
    # Create AddressSpace for schedulers and slaves
    for {set i 0} {$i < $numOfGroup} {incr i} {
        set num_hw_per_group [hapara_return_hw_number $i $numOfSlave $max_hw_slave]
        set num_mb_per_group [expr $numOfSlave - $num_hw_per_group]

        set group_name "group$i"
        # Create AddressSpace for schedulers
        set inst_path "/$group_name/scheduler"
        set cell_name "${bd_design_nm}_i/$group_name/scheduler_local_memory/lmb_bram"
        puts $fileout "  <Processor Endianness=\"Little\" InstPath=\"$inst_path\">"
        hapara_generate_mmi_addspace $fileout $brams $cell_name 0
        puts $fileout "  </Processor>"       
        for {set j 0} {$j < $num_mb_per_group} {incr j} {
            set slave_name "slave_s$j"
            # Create AddressSpace for slaves
            set inst_path "/$group_name/$slave_name"
            puts $fileout "  <Processor Endianness=\"Little\" InstPath=\"$inst_path\">"
            # FIXME here a "/" is used to make sure that bram will not contain bram1
            set cell_name "${bd_design_nm}_i/$group_name/${slave_name}_local_memory/lmb_bram/"
            hapara_generate_mmi_addspace $fileout $brams $cell_name 0
            set cell_name "${bd_design_nm}_i/$group_name/${slave_name}_local_memory/lmb_bram1/"
            # 32768 : 0x8000
            hapara_generate_mmi_addspace $fileout $brams $cell_name 32768
            puts $fileout "  </Processor>"    
        }
    }
    # Writing finish information
    puts $fileout "<Config>"
    puts $fileout "  <Option Name=\"Part\" Val=\"[get_property PART [current_project ]]\"/>"
    puts $fileout "</Config>"
    puts $fileout "</MemInfo>"
    close $fileout
    return 1
}
################################################################################
# Synthesis, place, and route design; write bitstream file
################################################################################
proc hapara_generate_bitstream {{numOfThreads 8}} {
    set project_name [current_project]
    set bd_design_nm [current_bd_design .]
    set curr_dir $::current_dir
    set proj_path "$curr_dir/$project_name"
    set top_module_name "${bd_design_nm}_wrapper"
    set bitstream_name "$project_name.bit"
    # Set maximum number of number of threads to run
    set_param general.maxThreads $numOfThreads
    # Synthesis design
    synth_design -top $top_module_name
    # Place design
    opt_design
    place_design
    phys_opt_design
    # Route design
    route_design
    # Write bitstream
    write_bitstream -force "$proj_path/$bitstream_name"
    return 1
}
################################################################################
# Export to SDK
################################################################################
proc hapara_export_sdk {} {
    set project_name [current_project]
    set bd_design_nm [current_bd_design .]
    # open_project "./$project_name/${project_name}.xpr"
    set curr_dir $::current_dir
    set proj_path "$curr_dir/$project_name"
    set sdk_dir "$proj_path/${project_name}.sdk"
    if {[file exists $sdk_dir] == 0} {
        puts "Creating SDK folder: $sdk_dir"
        file mkdir $sdk_dir
    } else {
        puts "ERROR: Existing SDK directory: $sdk_dir"
        return 0
    }
    set mmi_file "$proj_path/${project_name}.mmi"
    if {[file exists $mmi_file] == 0} {
        puts "ERROR: No mmi file found: $mmi_file."
        return 0
    }
    set bit_file "$proj_path/${project_name}.bit"
    if {[file exists $bit_file] == 0} {
        puts "ERROR: No bitstream file found: $bit_file"
        return 0
    }
    set hwdef_file "$proj_path/${project_name}.srcs/sources_1/bd/$bd_design_nm/hdl/${bd_design_nm}.hwdef"
    if {[file exists $hwdef_file] == 0} {
        puts "ERROR: No hwdef file found: $hwdef_file"
        return 0
    }
    puts "Creating HDF file containing"
    write_sysdef -force -meminfo $mmi_file -hwdef $hwdef_file -bitfile $bit_file -file "$sdk_dir/${bd_design_nm}_wrapper.hdf" 
    return 1
}

################################################################################
# Main procedure 
################################################################################
if {$argc < 4|| $argc > 7} {
    puts "ERROR:Invalid input arguments."
    puts {<Project Name> <Number of Groups> <Number of Slaves per Group> <Total Number of HW Slaves> [Name of HW Slave] [IP Repository] [Block Design Name]}
    puts "Please try again."
    return 0
}
set project_name [lindex $argv 0]
set num_of_group [lindex $argv 1]
set num_of_slave [lindex $argv 2]
set max_hw_slave [lindex $argv 3]
set current_dir [pwd]
set maximum_hw 8
if {$max_hw_slave > $maximum_hw} {
    puts "ERROR: Number of hardware slaves:$max_hw_slave cannot exceed the maximum:$maximum_hw"
    return 0
}
set hw_name "vector_add"
set ip_repo_path "$current_dir/hardware/ip_repo"
set resource_hls "$current_dir/resources/hls_project"
set bd_design_nm "system"
if {$argc >= 5} {
    set hw_name [lindex $argv 4]
}
if {$argc >= 6} {
    set ip_repo_path [lindex $argv 5]
}
if {$argc == 7} {
    set bd_design_nm [lindex $argv 5]
}
if {[hapara_vivado_version_check] == 0} {
    puts "ERROR: When running hapara_vivado_version_check()."
    return 0
}
if {[hapara_create_project $project_name] == 0} {
    puts "ERROR: When running hapara_create_project()."
    return 0
}
if {[hapara_create_bd $bd_design_nm] == 0} {
    puts "ERROR: When running hapara_create_bd()."
    return 0
}
if {[hapara_update_ip_repo $ip_repo_path $resource_hls] == 0} {
    puts "ERROR: When running hapara_update_ip_repo()."
    return 0
}
if {[hapara_create_root_design $num_of_group $num_of_slave $max_hw_slave $hw_name] == 0} {
    puts "ERROR: When running hapara_create_root_design()."
    return 0
}
# if {[hapara_create_hdl_wrapper] == 0} {
#     puts "ERROR: When running hapara_create_hdl_wrapper()."
#     return 0
# }
# if {[hapara_generate_bitstream] == 0} {
#     puts "ERROR: When running hapara_generate_bitstream()."
#     return 0
# }
# if {[hapara_generate_mmi $num_of_group $num_of_slave] == 0} {
#     puts "ERROR: When running hapara_generate_mmi()."
#     return 0
# }
# if {[hapara_export_sdk] == 0} {
#     puts "ERROR: When running hapara_export_sdk()."
#     return 0
# }

