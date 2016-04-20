proc hapara_set_workspace {proj_name} {
    set cur_dir $::current_dir
    set workspace_name "$cur_dir/${proj_name}/${proj_name}.sdk"
    sdk set_workspace workspace_name
    return 1
}

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
    file cp "$source_repo/microblaze/scheduler/lscript.ld" "$sdk_dir/scheduler/src"
    # Create mutex_manager project
    sdk create_bsp_project -name mutex_manager_bsp -hwproject $hw_mame -proc mutex_manager -os standalone
    sdk create_app_project -name mutex_manager -hwproject $hw_mame -proc mutex_manager -os standalone -lang C -app {Empty Application}
    sdk import_sources -name mutex_manager -path "$source_repo/microblaze/mutex_manager"
    file cp "$source_repo/microblaze/mutex_manager/lscript.ld" "$sdk_dir/mutex_manager/src"
    # Create slave 
    sdk create_bsp_project -name slave_kernel_bsp -hwproject $hw_mame -proc group0_slave_s0 -os standalone
    sdk create_app_project -name slave_kernel -hwproject $hwproject -proc group0_slave_s0 -os standalone -lang C -app {Empty Application}
    sdk import_sources -name slave_kernel -path "$source_repo/microblaze/slave_kernel"
    file cp "$source_repo/microblaze/slave_kernel/lscript.ld" "$sdk_dir/slave_kernel/src"
    # Build projects
    sdk build_project -type all
    return 1
}

proc hapara_update_bitstream {} {
    
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

if {[hapara_set_workspace $project_name] == 0} {
    puts "ERROR: When running hapara_set_workspace()."
    return 0
}

if {[hapara_create_hw $project_name] == 0} {
    puts "ERROR: When running hapara_create_hw()."
    return 0
}

