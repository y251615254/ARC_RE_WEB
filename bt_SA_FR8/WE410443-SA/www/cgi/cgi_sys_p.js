/*DEMO*/
addCfg("timeout",1,"5");
addCfg("http_pwd",100, "d94ec66daf606c54b4a8b423bf45009d");
addCfg("password_reminder",0, "wps-key"); 

/*END_DEMO*/
/*REAL
//put timeout before http_pwd due to the function '_CFG_setValue' of the file 'Nvram_cfg.c'. when setting new password, it will call init_user_config().
addCfg("chk","chk","");
addCfg("pws","pws","");
addCfg("usr","usr","");
addCfg("tr64h1","tr64h1","");
//<% ABS_MAP("http_pwd","ARC_SYS_Password") %>
//<% ABS_MAP("password_reminder","ARC_SYS_PWDReminder") %>
//<% ABS_MAP("password_reminder2","ARC_SYS_PWDReminder2") %>
//<% ABS_MAP("sys_username","ARC_SYS_LoginName") %>
//<% ABS_MAP("sys_username2","ARC_SYS_LoginName2") %>
REAL*/
