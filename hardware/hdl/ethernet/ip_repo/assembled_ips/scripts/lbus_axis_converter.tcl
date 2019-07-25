set ip_root     $::env(SNAP_HARDWARE_ROOT)/ip
set fpga_chip   $::env(FPGACHIP)

set project_dir [file dirname [file dirname [file normalize [info script]]]]
set project_name "lbus_axis_converter"
source ${project_dir}/scripts/util.tcl

create_project $project_name $ip_root/$project_name -part $fpga_chip
create_bd_design "${project_name}"

set_property ip_repo_paths [list "${ip_root}" "${project_dir}/interfaces/lbus_definition"] [current_project]
update_ip_catalog -rebuild

addip lbus_fifo_read lbus_fifo_read_0
addip lbus_fifo_write lbus_fifo_write_0

addip fifo_generator fifo_generator_0
set_property -dict [list CONFIG.Fifo_Implementation {Common_Clock_Distributed_RAM} CONFIG.Input_Data_Width {544} CONFIG.Input_Depth {16} CONFIG.Output_Data_Width {544} CONFIG.Output_Depth {16} CONFIG.Use_Embedded_Registers {false} CONFIG.Valid_Flag {true} CONFIG.Data_Count_Width {4} CONFIG.Write_Data_Count_Width {4} CONFIG.Read_Data_Count_Width {4} CONFIG.Full_Threshold_Assert_Value {14} CONFIG.Full_Threshold_Negate_Value {13}] [get_bd_cells fifo_generator_0]
set_property -dict [list CONFIG.Reset_Type {Asynchronous_Reset} CONFIG.Full_Flags_Reset_Value {1} CONFIG.Enable_Safety_Circuit {false}] [get_bd_cells fifo_generator_0]

addip fifo_generator fifo_generator_1
set_property -dict [list CONFIG.Fifo_Implementation {Common_Clock_Distributed_RAM} CONFIG.Input_Data_Width {408} CONFIG.Input_Depth {16} CONFIG.Output_Data_Width {408} CONFIG.Output_Depth {16} CONFIG.Use_Embedded_Registers {false} CONFIG.Reset_Type {Asynchronous_Reset} CONFIG.Full_Flags_Reset_Value {1} CONFIG.Valid_Flag {true} CONFIG.Data_Count_Width {4} CONFIG.Write_Data_Count_Width {4} CONFIG.Read_Data_Count_Width {4} CONFIG.Full_Threshold_Assert_Value {14} CONFIG.Full_Threshold_Negate_Value {13} CONFIG.Enable_Safety_Circuit {false}] [get_bd_cells fifo_generator_1]
addip axis2lbus axis2lbus_0

addip util_vector_logic util_vector_logic_0
set_property -dict [list CONFIG.C_SIZE {1}] [get_bd_cells util_vector_logic_0]

make_bd_pins_external  [get_bd_pins util_vector_logic_0/Res]
make_bd_pins_external  [get_bd_pins lbus_fifo_write_0/ap_clk]
make_bd_pins_external  [get_bd_pins lbus_fifo_write_0/ap_rst]

make_bd_pins_external  [get_bd_pins lbus_fifo_read_0/m_axis_data_V]
make_bd_pins_external  [get_bd_pins lbus_fifo_read_0/m_axis_keep_V]
make_bd_pins_external  [get_bd_pins lbus_fifo_read_0/m_axis_last_V]
make_bd_pins_external  [get_bd_pins lbus_fifo_read_0/m_axis_valid_V]

make_bd_pins_external  [get_bd_pins axis2lbus_0/s_axis_data_V]
make_bd_pins_external  [get_bd_pins axis2lbus_0/s_axis_keep_V]
make_bd_pins_external  [get_bd_pins axis2lbus_0/s_axis_last_V]
make_bd_pins_external  [get_bd_pins axis2lbus_0/s_axis_valid_V]
make_bd_pins_external  [get_bd_pins axis2lbus_0/axis_ready_V]

set_property name "error" [get_bd_ports Res_0]
set_property name clk [get_bd_ports ap_clk_0]
set_property name rst [get_bd_ports ap_rst_0]
foreach port [get_bd_ports *_V_0] {
        set_property name [regsub "_V_0" [regsub "/" $port ""] ""] $port
}

make_bd_pins_external  [get_bd_pins lbus_fifo_write_0/lbus_0_data_V] [get_bd_pins lbus_fifo_write_0/lbus_2_eop_V] [get_bd_pins lbus_fifo_write_0/lbus_0_sop_V] [get_bd_pins lbus_fifo_write_0/lbus_3_eop_V] [get_bd_pins lbus_fifo_write_0/lbus_1_sop_V] [get_bd_pins lbus_fifo_write_0/lbus_2_ena_V] [get_bd_pins lbus_fifo_write_0/lbus_3_ena_V] [get_bd_pins lbus_fifo_write_0/lbus_0_ena_V] [get_bd_pins lbus_fifo_write_0/lbus_2_mty_V] [get_bd_pins lbus_fifo_write_0/lbus_1_ena_V] [get_bd_pins lbus_fifo_write_0/lbus_3_mty_V] [get_bd_pins lbus_fifo_write_0/lbus_2_data_V] [get_bd_pins lbus_fifo_write_0/lbus_0_mty_V] [get_bd_pins lbus_fifo_write_0/lbus_3_data_V] [get_bd_pins lbus_fifo_write_0/lbus_1_mty_V] [get_bd_pins lbus_fifo_write_0/lbus_2_err_V] [get_bd_pins lbus_fifo_write_0/lbus_0_eop_V] [get_bd_pins lbus_fifo_write_0/lbus_1_data_V] [get_bd_pins lbus_fifo_write_0/lbus_3_err_V] [get_bd_pins lbus_fifo_write_0/lbus_1_eop_V] [get_bd_pins lbus_fifo_write_0/lbus_0_err_V] [get_bd_pins lbus_fifo_write_0/lbus_2_sop_V] [get_bd_pins lbus_fifo_write_0/lbus_1_err_V] [get_bd_pins lbus_fifo_write_0/lbus_3_sop_V]

make_bd_pins_external  [get_bd_pins axis2lbus_0/lbus_3_sop_V] [get_bd_pins axis2lbus_0/lbus_1_ena_V] [get_bd_pins axis2lbus_0/lbus_ready_V] [get_bd_pins axis2lbus_0/lbus_0_eop_V] [get_bd_pins axis2lbus_0/lbus_2_ena_V] [get_bd_pins axis2lbus_0/lbus_1_eop_V] [get_bd_pins axis2lbus_0/lbus_3_ena_V] [get_bd_pins axis2lbus_0/lbus_2_eop_V] [get_bd_pins axis2lbus_0/lbus_0_sop_V] [get_bd_pins axis2lbus_0/lbus_3_eop_V] [get_bd_pins axis2lbus_0/lbus_1_sop_V] [get_bd_pins axis2lbus_0/lbus_0_err_V] [get_bd_pins axis2lbus_0/lbus_1_err_V] [get_bd_pins axis2lbus_0/lbus_2_err_V] [get_bd_pins axis2lbus_0/lbus_3_err_V] [get_bd_pins axis2lbus_0/lbus_0_data_V] [get_bd_pins axis2lbus_0/lbus_0_mty_V] [get_bd_pins axis2lbus_0/lbus_1_data_V] [get_bd_pins axis2lbus_0/lbus_1_mty_V] [get_bd_pins axis2lbus_0/lbus_2_data_V] [get_bd_pins axis2lbus_0/lbus_2_mty_V] [get_bd_pins axis2lbus_0/lbus_3_data_V] [get_bd_pins axis2lbus_0/lbus_3_mty_V] [get_bd_pins axis2lbus_0/lbus_2_sop_V] [get_bd_pins axis2lbus_0/lbus_0_ena_V]

connect_bd_net [get_bd_ports clk] [get_bd_pins lbus_fifo_read_0/ap_clk]
connect_bd_net [get_bd_ports rst] [get_bd_pins lbus_fifo_read_0/ap_rst]
connect_bd_net [get_bd_ports clk] [get_bd_pins fifo_generator_0/clk]
connect_bd_net [get_bd_ports rst] [get_bd_pins fifo_generator_0/rst]
connect_bd_net [get_bd_ports clk] [get_bd_pins fifo_generator_1/clk]
connect_bd_net [get_bd_ports rst] [get_bd_pins fifo_generator_1/rst]
connect_bd_net [get_bd_pins lbus_fifo_write_0/lbus_fifo] [get_bd_pins fifo_generator_0/din]
connect_bd_net [get_bd_pins lbus_fifo_write_0/lbus_fifo_we_V] [get_bd_pins fifo_generator_0/wr_en]
connect_bd_net [get_bd_pins lbus_fifo_write_0/lbus_fifo_pkt_end] [get_bd_pins fifo_generator_1/din]
connect_bd_net [get_bd_pins lbus_fifo_write_0/lbus_fifo_pkt_end_we_V] [get_bd_pins fifo_generator_1/wr_en]
connect_bd_net [get_bd_pins lbus_fifo_read_0/lbus_fifo] [get_bd_pins fifo_generator_0/dout]
connect_bd_net [get_bd_pins lbus_fifo_read_0/lbus_fifo_empty_V] [get_bd_pins fifo_generator_0/empty]
connect_bd_net [get_bd_pins lbus_fifo_read_0/lbus_fifo_re_V] [get_bd_pins fifo_generator_0/rd_en]
connect_bd_net [get_bd_pins lbus_fifo_read_0/lbus_fifo_pkt_end] [get_bd_pins fifo_generator_1/dout]
connect_bd_net [get_bd_pins lbus_fifo_read_0/lbus_fifo_pkt_end_empty_V] [get_bd_pins fifo_generator_1/empty]
connect_bd_net [get_bd_pins lbus_fifo_read_0/lbus_fifo_pkt_end_re_V] [get_bd_pins fifo_generator_1/rd_en]
connect_bd_net [get_bd_pins fifo_generator_0/valid] [get_bd_pins lbus_fifo_read_0/lbus_fifo_valid_V]
connect_bd_net [get_bd_pins fifo_generator_1/valid] [get_bd_pins lbus_fifo_read_0/lbus_fifo_pkt_end_valid_V]
connect_bd_net [get_bd_ports clk] [get_bd_pins axis2lbus_0/ap_clk]
connect_bd_net [get_bd_ports rst] [get_bd_pins axis2lbus_0/ap_rst]
connect_bd_net [get_bd_pins lbus_fifo_write_0/error_V] [get_bd_pins util_vector_logic_0/Op1]
connect_bd_net [get_bd_pins lbus_fifo_read_0/error_V] [get_bd_pins util_vector_logic_0/Op2]

save_bd_design

make_wrapper -files [get_files $ip_root/$project_name/${project_name}.srcs/sources_1/bd/${project_name}/${project_name}.bd] -top
add_files -norecurse $ip_root/$project_name/${project_name}.srcs/sources_1/bd/${project_name}/hdl/${project_name}_wrapper.v

ipx::package_project -root_dir $ip_root/$project_name/${project_name}.srcs/sources_1/bd/${project_name} -vendor clarkshen.com -library user -taxonomy /UserIP
set_property vendor_display_name {clarkshen.com} [ipx::current_core]
set_property name $project_name [ipx::current_core]
set_property display_name $project_name [ipx::current_core]
set_property description $project_name [ipx::current_core]

ipx::add_bus_interface s_axis [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:interface:axis_rtl:1.0 [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:axis:1.0 [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property display_name s_axis [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
ipx::add_port_map TDATA [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property physical_name s_axis_data [ipx::get_port_maps TDATA -of_objects [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TLAST [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property physical_name s_axis_last [ipx::get_port_maps TLAST -of_objects [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TVALID [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property physical_name s_axis_valid [ipx::get_port_maps TVALID -of_objects [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TKEEP [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property physical_name s_axis_keep [ipx::get_port_maps TKEEP -of_objects [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TREADY [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]
set_property physical_name axis_ready [ipx::get_port_maps TREADY -of_objects [ipx::get_bus_interfaces s_axis -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif s_axis -clock clk [ipx::current_core]

ipx::add_bus_interface m_axis [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:interface:axis_rtl:1.0 [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:interface:axis:1.0 [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property interface_mode master [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property display_name m_axis [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
ipx::add_port_map TDATA [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property physical_name m_axis_data [ipx::get_port_maps TDATA -of_objects [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TLAST [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property physical_name m_axis_last [ipx::get_port_maps TLAST -of_objects [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TVALID [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property physical_name m_axis_valid [ipx::get_port_maps TVALID -of_objects [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]]
ipx::add_port_map TKEEP [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]
set_property physical_name m_axis_keep [ipx::get_port_maps TKEEP -of_objects [ipx::get_bus_interfaces m_axis -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif m_axis -clock clk [ipx::current_core]

set port_prefix "lbus"
ipx::add_bus_interface lbus_tx [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:display_cmac_usplus:lbus_ports:2.0 [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:user:lbus:2.0 [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property interface_mode master [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property display_name lbus_tx [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]

ipx::add_port_map lbus_seg0_data [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_data_V_1 [ipx::get_port_maps lbus_seg0_data -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_data [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_data_V_1 [ipx::get_port_maps lbus_seg1_data -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_data [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_data_V_1 [ipx::get_port_maps lbus_seg2_data -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_data [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_data_V_1 [ipx::get_port_maps lbus_seg3_data -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_ena [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_ena_V_1 [ipx::get_port_maps lbus_seg0_ena -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_ena [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_ena_V_1 [ipx::get_port_maps lbus_seg1_ena -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_ena [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_ena_V_1 [ipx::get_port_maps lbus_seg2_ena -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_ena [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_ena_V_1 [ipx::get_port_maps lbus_seg3_ena -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_err [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_err_V_1 [ipx::get_port_maps lbus_seg0_err -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_err [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_err_V_1 [ipx::get_port_maps lbus_seg1_err -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_err [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_err_V_1 [ipx::get_port_maps lbus_seg2_err -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_err [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_err_V_1 [ipx::get_port_maps lbus_seg3_err -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_sop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_sop_V_1 [ipx::get_port_maps lbus_seg0_sop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_sop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_sop_V_1 [ipx::get_port_maps lbus_seg1_sop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_sop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_sop_V_1 [ipx::get_port_maps lbus_seg2_sop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_sop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_sop_V_1 [ipx::get_port_maps lbus_seg3_sop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_eop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_eop_V_1 [ipx::get_port_maps lbus_seg0_eop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_eop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_eop_V_1 [ipx::get_port_maps lbus_seg1_eop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_eop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_eop_V_1 [ipx::get_port_maps lbus_seg2_eop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_eop [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_eop_V_1 [ipx::get_port_maps lbus_seg3_eop -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_mty [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_mty_V_1 [ipx::get_port_maps lbus_seg0_mty -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_mty [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_mty_V_1 [ipx::get_port_maps lbus_seg1_mty -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_mty [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_mty_V_1 [ipx::get_port_maps lbus_seg2_mty -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_mty [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_mty_V_1 [ipx::get_port_maps lbus_seg3_mty -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::add_port_map tx_rdyout [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]
set_property physical_name lbus_ready_V_0 [ipx::get_port_maps tx_rdyout -of_objects [ipx::get_bus_interfaces lbus_tx -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif lbus_tx -clock clk [ipx::current_core]


set port_prefix "lbus"
ipx::add_bus_interface lbus_rx [ipx::current_core]
set_property abstraction_type_vlnv xilinx.com:display_cmac_usplus:lbus_ports:2.0 [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property bus_type_vlnv xilinx.com:user:lbus:2.0 [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property interface_mode slave [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property display_name lbus_rx [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]

ipx::add_port_map lbus_seg0_data [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_data_V_0 [ipx::get_port_maps lbus_seg0_data -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_data [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_data_V_0 [ipx::get_port_maps lbus_seg1_data -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_data [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_data_V_0 [ipx::get_port_maps lbus_seg2_data -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_data [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_data_V_0 [ipx::get_port_maps lbus_seg3_data -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_ena [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_ena_V_0 [ipx::get_port_maps lbus_seg0_ena -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_ena [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_ena_V_0 [ipx::get_port_maps lbus_seg1_ena -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_ena [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_ena_V_0 [ipx::get_port_maps lbus_seg2_ena -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_ena [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_ena_V_0 [ipx::get_port_maps lbus_seg3_ena -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_err [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_err_V_0 [ipx::get_port_maps lbus_seg0_err -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_err [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_err_V_0 [ipx::get_port_maps lbus_seg1_err -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_err [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_err_V_0 [ipx::get_port_maps lbus_seg2_err -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_err [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_err_V_0 [ipx::get_port_maps lbus_seg3_err -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_sop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_sop_V_0 [ipx::get_port_maps lbus_seg0_sop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_sop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_sop_V_0 [ipx::get_port_maps lbus_seg1_sop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_sop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_sop_V_0 [ipx::get_port_maps lbus_seg2_sop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_sop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_sop_V_0 [ipx::get_port_maps lbus_seg3_sop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_eop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_eop_V_0 [ipx::get_port_maps lbus_seg0_eop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_eop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_eop_V_0 [ipx::get_port_maps lbus_seg1_eop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_eop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_eop_V_0 [ipx::get_port_maps lbus_seg2_eop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_eop [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_eop_V_0 [ipx::get_port_maps lbus_seg3_eop -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg0_mty [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_0_mty_V_0 [ipx::get_port_maps lbus_seg0_mty -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg1_mty [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_1_mty_V_0 [ipx::get_port_maps lbus_seg1_mty -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg2_mty [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_2_mty_V_0 [ipx::get_port_maps lbus_seg2_mty -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::add_port_map lbus_seg3_mty [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]
set_property physical_name ${port_prefix}_3_mty_V_0 [ipx::get_port_maps lbus_seg3_mty -of_objects [ipx::get_bus_interfaces lbus_rx -of_objects [ipx::current_core]]]
ipx::associate_bus_interfaces -busif lbus_rx -clock clk [ipx::current_core]
set_property supported_families {virtexu Beta virtexuplus Beta virtexuplusHBM Beta zynquplus Beta kintexu Beta kintexuplus Beta} [ipx::current_core]

set_property core_revision 0 [ipx::current_core]
ipx::create_xgui_files [ipx::current_core]
ipx::update_checksums [ipx::current_core]
ipx::save_core [ipx::current_core]
set_property  ip_repo_paths [list "$ip_root/$project_name/${project_name}.srcs/sources_1/bd/${project_name}" $ip_root "${project_dir}/interfaces/lbus_definition"] [current_project]
update_ip_catalog
ipx::check_integrity -quiet [ipx::current_core]
ipx::archive_core $ip_root/$project_name/${project_name}.srcs/sources_1/bd/${project_name}/${project_name}_1.0.zip [ipx::current_core]
close_project
exit
