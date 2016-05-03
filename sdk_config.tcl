# proc hapara_set_workspace {proj_name} {
#     set cur_dir $::current_dir
#     set workspace_name "$cur_dir/${proj_name}/${proj_name}.sdk"
#     sdk set_workspace workspace_name
#     return 1
# }

proc hapara_create_hw {proj_name {wrapper_name system_wrapper} {hw_mame system_wrapper_hw_platform_0}} {
    set cur_dir $::current_dir
    set wrapper_path "$cur_dir/${proj_name}/${proj_name}.sdk/${wrapper_name}.hdf"
    sdk create_hw_project -name $hw_mame -hwspec $wrapper_path
    return 1
}

proc hapara_find_first_mb {num_of_group num_of_slave total_num_of_hw_slave} {
    if {[expr $num_of_group * $num_of_slave] <= $total_num_of_hw_slave} {
        return ""
    }
    set q [expr $total_num_of_hw_slave / $num_of_slave]
    set r [expr $total_num_of_hw_slave % $num_of_slave]
    return "group${q}_slave_s${r}"
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

proc hapara_create_functional_app {proj_name source_repo first_mb {hw_mame system_wrapper_hw_platform_0}} {
    set cur_dir $::current_dir
    set sdk_dir "$cur_dir/$proj_name/${proj_name}.sdk"
    # Copy generic folder
    file copy -force "$source_repo/generic" $sdk_dir
    # Create scheduler project
    sdk create_bsp_project -name scheduler_bsp -hwproject $hw_mame -proc group0_scheduler -os standalone
    sdk create_app_project -name scheduler -hwproject $hw_mame -proc group0_scheduler -os standalone -lang C -app {Empty Application} -bsp scheduler_bsp
    sdk import_sources -name scheduler -path "$source_repo/microblaze/scheduler"
    file copy -force "$source_repo/microblaze/lscript/lscript00.ld" "$sdk_dir/scheduler/src/lscript.ld"
    # Create mutex_manager project
    sdk create_bsp_project -name mutex_manager_bsp -hwproject $hw_mame -proc mutex_manager -os standalone
    sdk create_app_project -name mutex_manager -hwproject $hw_mame -proc mutex_manager -os standalone -lang C -app {Empty Application} -bsp mutex_manager_bsp
    sdk import_sources -name mutex_manager -path "$source_repo/microblaze/mutex_manager"
    file copy -force "$source_repo/microblaze/lscript/lscript00.ld" "$sdk_dir/mutex_manager/src/lscript.ld"
    # Create slave
    if {$first_mb != ""} {
        sdk create_bsp_project -name slave_kernel_bsp -hwproject $hw_mame -proc $first_mb -os standalone
        sdk create_app_project -name slave_kernel -hwproject $hw_mame -proc $first_mb -os standalone -lang C -app {Empty Application} -bsp slave_kernel_bsp
        sdk import_sources -name slave_kernel -path "$source_repo/microblaze/slave_kernel"
        file copy -force "$source_repo/microblaze/lscript/lscript00.ld" "$sdk_dir/slave_kernel/src/lscript.ld"        
    }
    # Build projects
    sdk build_project -type all
    return 1
}

proc hapara_create_opencl_app {proj_name source_repo first_mb {type Debug} {hw_mame system_wrapper_hw_platform_0}} {
    if {$first_mb == ""} {
        puts "Warning: hapara_create_opencl_app: there is no microblaze slaves in this project."
        return 1
    }
    set cur_dir $::current_dir
    set sdk_dir "$cur_dir/$proj_name/${proj_name}.sdk"
    set app_list [glob -nocomplain -type d "$source_repo/microblaze/apps/*"]
    if {$app_list == ""} {
        puts "There are no OpenCL apps under $source_repo/microblaze/apps/"
        return 0
    }
    set app_name_list ""
    foreach dir $app_list {
        set temp [string range $dir [expr {[string last "/" $dir] + 1}] end]
        lappend app_name_list $temp
    }
    file mkdir "$sdk_dir/app_elfs"
    foreach app $app_name_list {
        sdk create_bsp_project -name "${app}_bsp" -hwproject $hw_mame -proc $first_mb -os standalone
        sdk create_app_project -name $app -hwproject $hw_mame -proc $first_mb -os standalone -lang C -app {Empty Application} -bsp "${app}_bsp"
        sdk import_sources -name $app -path "$source_repo/microblaze/apps/$app"
        file copy -force "$source_repo/microblaze/lscript/lscript80.ld" "$sdk_dir/$app/src/lscript.ld"  
        sdk build_project -type bsp -name "${app}_bsp"
        sdk build_project -type app -name $app
        file copy -force "$sdk_dir/$app/$type/${app}.elf" "$sdk_dir/app_elfs/"
    }
    return 1
}

proc hapara_update_bitstream {proj_name num_of_group num_of_slave num_of_hw_slave {type Debug} {hw_mame system_wrapper_hw_platform_0}} {
    set cur_dir $::current_dir
    set sdk_dir "$cur_dir/$proj_name/${proj_name}.sdk"
    set mem_path "$sdk_dir/$hw_mame/${proj_name}.mmi"
    # Create temp bitstream folder
    file mkdir "$sdk_dir/bit_temp"
    set counter 0
    # Create mutex_manager bit
    puts "Creating /mutex_manager bits."
    exec updatemem -force -meminfo \
        $mem_path \
        -bit \
        "$sdk_dir/$hw_mame/${proj_name}.bit" \
        -data \
        "$sdk_dir/mutex_manager/$type/mutex_manager.elf" \
        -proc \
        /mutex_manager \
        -out \
        "$sdk_dir/bit_temp/temp$counter.bit"
    incr counter
    # Create scheduler and slave bit
    for {set i 0} {$i < $num_of_group} {incr i} {
        set num_hw_per_group [hapara_return_hw_number $i $num_of_slave $num_of_hw_slave]
        set num_mb_per_group [expr $num_of_slave - $num_hw_per_group]

        set group_name "group$i"
        # Create scheduler bit
        puts "Creating /$group_name/scheduler bits."
        exec updatemem -force -meminfo \
            $mem_path \
            -bit \
            "$sdk_dir/bit_temp/temp[expr $counter - 1].bit" \
            -data \
            "$sdk_dir/scheduler/$type/scheduler.elf" \
            -proc \
            "/$group_name/scheduler" \
            -out \
            "$sdk_dir/bit_temp/temp$counter.bit"
        file delete "$sdk_dir/bit_temp/temp[expr $counter - 1].bit"
        incr counter
        for {set j 0} {$j < $num_mb_per_group} {incr j} {
            set slave_name "slave_s$j"
            # Create slave bit
            puts "Creating /$group_name/$slave_name bits."
            exec updatemem -force -meminfo \
                $mem_path \
                -bit \
                "$sdk_dir/bit_temp/temp[expr $counter - 1].bit" \
                -data \
                "$sdk_dir/slave_kernel/$type/slave_kernel.elf" \
                -proc \
                "/$group_name/$slave_name" \
                -out \
                "$sdk_dir/bit_temp/temp$counter.bit"
            file delete "$sdk_dir/bit_temp/temp[expr $counter - 1].bit"
            incr counter
        }
    }
    file copy -force "$sdk_dir/bit_temp/temp[expr $counter - 1].bit" "$sdk_dir/$hw_mame/download.bit"
    file delete -force "$sdk_dir/bit_temp"
    return 1
}

set current_dir [pwd]
set source_repo "$current_dir/software"
if {$argc < 4 || $argc > 5} {
    puts "ERROR:Invalid input arguments."
    puts {<Project Name> <Number of Groups> <Number of Slaves> <Total Number of HW Slaves> [Source Codes Repository]}
    puts "Please try again."
    return 0
}
set project_name [lindex $argv 0]
set num_of_group [lindex $argv 1]
set num_of_slave [lindex $argv 2]
set num_of_hw_slave [lindex $argv 3]
if {$argc == 5} {
    set source_repo [lindex $argv 4]
}

# if {[hapara_set_workspace $project_name] == 0} {
#     puts "ERROR: When running hapara_set_workspace()."
#     return 0
# }
set workspace_name "$current_dir/${project_name}/${project_name}.sdk"
sdk set_workspace $workspace_name
set first_mb [hapara_find_first_mb $num_of_group $num_of_slave $num_of_hw_slave]

if {[hapara_create_hw $project_name] == 0} {
    puts "ERROR: When running hapara_create_hw()."
    return 0
}

if {[hapara_create_functional_app $project_name $source_repo $first_mb] == 0} {
    puts "ERROR: When running hapara_create_functional_app()."
    return 0
}

if {[hapara_update_bitstream $project_name $num_of_group $num_of_slave $num_of_hw_slave] == 0} {
    puts "ERROR: When running hapara_update_bitstream()."
    return 0
}

if {[hapara_create_opencl_app $project_name $source_repo $first_mb] == 0} {
    puts "ERROR: When running hapara_create_opencl_app()."
    return 0
}
