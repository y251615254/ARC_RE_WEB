/*DEMO*/
var login_counter=10;

var product_name="LTE HGW";
var def_product_name="";
/*END_DEMO*/
/*REAL
var def_product_name=(<!--DEF#@PROJ_UI_PRODUCT_NAME@-->);
var login_counter="<% ABS_GET("ARC_SYS_LoginTimeout") %>";
REAL*/

if(def_product_name != "") product_name=def_product_name;
