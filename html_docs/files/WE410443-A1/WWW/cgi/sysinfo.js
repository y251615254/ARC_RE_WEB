/*REAL
//serial_number="<% nvram_get("ARC_SYS_SerialNum"); %>";
serial_number="<%ABS_GET("ARC_SYS_SerialNum")%>";
runtime_code_version="<% get_firmware_version(); %> <% compile_date(); %>";
boot_code_version="<%ABS_GET("ARC_SYS_BootVersion")%>";
lan_mac_addr="<% nvram_get("ARC_LAN_0_MACaddr"); %>";
hardware_version="<%ABS_GET("ARC_SYS_HWVersion")%>";

//format: 'dev':'totalsize(KB)|usedsize(KB)|freesize(KB)|sharedname'
USB Storage:
<% get_file_info("disk_info"); %>
<% get_disk_info_files(); %>
REAL*/
