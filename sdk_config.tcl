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
    return 0
}

proc hapara_create_functional_app {proj_name source_repo {hw_mame system_wrapper_hw_platform_0}} {
    set cur_dir $::current_dir
    set sdk_dir "$cur_dir/$proj_name/${proj_name}.sdk"
    # Copy generic folder
    file cp -r "$source_repo/generic" $sdk_dir
    # Create scheduler project
    sdk create_bsp_project -name scheduler_bsp -hwproject $hw_mame -proc group0_scheduler -os standalone
    sdk create_app_project -name scheduler -hwproject $hw_mame -proc group0_scheduler -os standalone -lang C -app {Empty Application} -bsp "${group_name}_scheduler_bsp"
    sdk import_sources -name scheduler -path "$source_repo/microblaze/scheduler"
    file cp "$source_repo/microblaze/lscript/lscript00.ld" "$sdk_dir/scheduler/src/lscript.ld"
    # Create mutex_manager project
    sdk create_bsp_project -name mutex_manager_bsp -hwproject $hw_mame -proc mutex_manager -os standalone
    sdk create_app_project -name mutex_manager -hwproject $hw_mame -proc mutex_manager -os standalone -lang C -app {Empty Application}
    sdk import_sources -name mutex_manager -path "$source_repo/microblaze/mutex_manager"
    file cp "$source_repo/microblaze/lscript/lscript00.ld" "$sdk_dir/mutex_manager/src/lscript.ld"
    # Create slave
    sdk create_bsp_project -name slave_kernel_bsp -hwproject $hw_mame -proc group0_slave_s0 -os standalone
    sdk create_app_project -name slave_kernel -hwproject $hwproject -proc group0_slave_s0 -os standalone -lang C -app {Empty Application}
    sdk import_sources -name slave_kernel -path "$source_repo/microblaze/slave_kernel"
    file cp "$source_repo/microblaze/lscript/lscript00.ld" "$sdk_dir/slave_kernel/src/lscript.ld"
    # Build projects
    sdk build_project -type all
    return 1
}

proc hapara_create_opencl_app {proj_name source_repo {hw_mame system_wrapper_hw_platform_0}} {
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
        sdk create_bsp_project -name "${app}_bsp" -hwproject $hw_mame -proc group0_slave_s0 -os standalone
        sdk create_app_project -name $app -hwproject $hw_mame -proc group0_slave_s0 -os standalone -lang C -app {Empty Application}
        sdk import_sources -name $app -path "$source_repo/microblaze/apps/$app"
        file cp "$source_repo/microblaze/lscript/lscript80.ld" "$sdk_dir/$app/src/lscript.ld"  
        sdk build_project -type bsp "${app}_bsp"
        sdk build_project -type app $app
        file cp "$sdk_dir/$app/Debug/${app}.elf" "$sdk_dir/app_elfs"
    }
    return 1
}

proc hapara_update_bitstream {proj_name num_of_group num_of_slave {hw_mame system_wrapper_hw_platform_0}} {
    set cur_dir $::current_dir
    set sdk_dir "$cur_dir/$proj_name/${proj_name}.sdk"
    set mem_path "$sdk_dir/$hw_mame/${proj_name}.mmi"
    # Create temp bitstream folder
    file mkdir "$sdk_dir/bit_temp"
    set counter 0
    # Create mutex_manager bit
    updatemem -force -meminfo \
        $mem_path \
        -bit \
        "$sdk_dir/$hw_mame/${proj_name}.bit" \
        -data \
        "$sdk_dir/mutex_manager/Debug/mutex_manager.elf" \
        -proc \
        mutex_manager \
        -out \
        "$sdk_dir/bit_temp/temp$counter.bit"
    incr counter
    # Create scheduler and slave bit
    for {set i 0} {$i < $num_of_group} {incr i} {
        set group_name "group$i"
        # Create scheduler bit
        updatemem -force -meminfo \
            $mem_path \
            -bit \
            "$sdk_dir/bit_temp/temp${[expr $counter - 1]}.bit" \
            -data \
            "$sdk_dir/scheduler/Debug/scheduler.elf" \
            -proc \
            "$group_name/scheduler" \
            -out \
            "$sdk_dir/bit_temp/temp$counter.bit"
        file rm "$sdk_dir/bit_temp/temp${[expr $counter - 1]}.bit"
        incr counter
        for {set j 0} {$j < $num_of_slave} {incr j} {
            set slave_name "slave_s$j"
            # Create scheduler bit
            updatemem -force -meminfo \
                $mem_path \
                -bit \
                "$sdk_dir/bit_temp/temp${[expr $counter - 1]}.bit" \
                -data \
                "$sdk_dir/$slave_kernel/Debug/${slave_kernel}.elf" \
                -proc \
                "$group_name/$slave_name" \
                -out \
                "$sdk_dir/bit_temp/temp$counter.bit"
            file rm "$sdk_dir/bit_temp/temp${[expr $counter - 1]}.bit"
            incr counter
        }
    }
    file mv "$sdk_dir/bit_temp/temp${[expr $counter - 1]}.bit" "$sdk_dir/$hw_mame/download.bit"
    file rm -rf "$sdk_dir/bit_temp"
    return 1
}

set current_dir [pwd]
set source_repo "/hding/Dropbox/hdl/HaPara"
if {$argc < 3 || $argc > 4} {
    puts "ERROR:Invalid input arguments."
    puts {<Project Name> <Number of Groups> <Number of Slaves> [Source Codes Repository]}
    puts "Please try again."
    return 0
}
set project_name [lindex $argv 0]
set num_of_group [lindex $argv 1]
set num_of_slave [lindex $argv 2]
if {$argc == 4} {
    set source_repo [lindex $argv 3]
}

# if {[hapara_set_workspace $project_name] == 0} {
#     puts "ERROR: When running hapara_set_workspace()."
#     return 0
# }
set workspace_name "$current_dir/${project_name}/${project_name}.sdk"
sdk set_workspace $workspace_name

if {[hapara_create_hw $project_name] == 0} {
    puts "ERROR: When running hapara_create_hw()."
    return 0
}

if {[hapara_create_functional_app $project_name $source_repo] == 0} {
    puts "ERROR: When running hapara_create_functional_app()."
    return 0
}

if {[hapara_update_bitstream $project_name $num_of_group $num_of_slave] == 0} {
    puts "ERROR: When running hapara_update_bitstream()."
    return 0
}

if {[hapara_create_opencl_app $project_name $source_repo] == 0} {
    puts "ERROR: When running hapara_create_opencl_app()."
    return 0
}
