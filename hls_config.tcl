proc hls_config_update {args} {
	set current_dir [pwd]
	set resources "$current_dir/resources"
	set repo "$current_dir/hardware/hw_apps"
	set new_app_name_list ""
	if {[file exists $resources] == 0} {
		puts "hls_config_update: Create $resources"
		file mkdir $resources
	}
	set args_list [string range $args 1 [expr [string length $args] - 2]]

	if {([llength $args_list] == 2) || ([llength $args_list] == 3 && [lindex $args_list 2] == "all")} {
		set app_list [glob -nocomplain -type d "$repo/*"]
		puts "$app_list"
		set app_name_list ""
		if {$app_list == ""} {
			puts "ERROR: There are no HLS apps under $repo"
			set ::ret 0
			return 0
		}
		foreach dir $app_list {
	        set temp [string range $dir [expr {[string last "/" $dir] + 1}] end]
	        lappend app_name_list $temp
		}
		if {[llength $args_list] == 3} {
			set new_app_name_list $app_name_list
		} else {
			set res_app_list [glob -nocomplain -type d "$resources/hls_project/*"]
			set res_app_name_list ""
			foreach dir $res_app_list {
		        set temp [string range $dir [expr {[string last "/" $dir] + 1}] end]
		        lappend res_app_name_list $temp		
			}

			foreach app $app_name_list {
				if {[lsearch $res_app_name_list $app] < 0}  {
					lappend new_app_name_list $app
				}
			}				
		}
	} else {
		for {set i 2} {$i < [llength $args_list]} {incr i} {
			lappend new_app_name_list [lindex $args_list $i]
		}
	}
	# puts $new_app_name_list
	set resources_hls_project "$resources/hls_project"
	if {[file exists $resources_hls_project] == 0} {
		puts "hls_config_update: Create $resources_hls_project"
		file mkdir $resources_hls_project
	}
	set hls_app_dir ""
	cd $resources_hls_project
	foreach app $new_app_name_list {
		set hls_app_dir "$resources_hls_project/$app"
		set hls_app_repo_dir "$repo/$app"
		if {[file exists $hls_app_repo_dir] == 0} {
			puts "Critical Warning: hls_config_update: $hls_app_repo_dir do not exist."
			continue
		}

		if {[file exists $hls_app_dir] == 0} {
			# No project dir exists
			open_project $app
			set_top $app
			add_files "$hls_app_repo_dir/${app}.c"
			open_solution "sol_ip"
			set_part {xc7z045ffg900-2}
			create_clock -period 10 -name default
			csynth_design
			export_design -format ip_catalog
			open_solution "sol_dcp"
			set_part {xc7z045ffg900-2}
			create_clock -period 10 -name default
			csynth_design
			export_design -format syn_dcp
			close_project	
		} else {
			# Project dir exists, and update project
			open_project $app
			open_solution "sol_ip"
			csynth_design
			export_design -format ip_catalog
			open_solution "sol_dcp"
			csynth_design
			export_design -format syn_dcp
			close_project
		}
	}
	cd $current_dir
	exit
}

hls_config_update $argv