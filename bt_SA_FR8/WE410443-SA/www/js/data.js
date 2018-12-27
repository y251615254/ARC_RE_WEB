// DMZ
var DMZ_NUMBER_PER_INTERFACE = 16;
var DMZ_TOTAL_COUNT = 16;

var PORT_MAX = 5, IP_MAX = 1;
var MAX_PPPOE_SESSION = 4;

// b_port, e_port
String.prototype.trimall = function()
{
	return this.replace(/\s/g, '');
}
String.prototype.trim = function()
{
	return this.replace(/^\s+|\s+$/g,"");
}
String.prototype.ltrim = function()
{
	return this.replace(/^\s+/,"");
}
String.prototype.rtrim = function()
{
	return this.replace(/\s+$/,"");
}

function PORT_RANGE()
{
	this.protocol = 0; // 0(both), tcp or udp
	this.b_port = 0;
	this.e_port = 0;
};

function IP_RANGE()
{
	this.ip = ""; // start ip
	this.count = 0;
};

// ex: strIP = "192.168.010.1"
/* we no use it,marked by Jack
function parseIP(strIP)
{
	var val1, val2, val3, val4;
	var IP = strIP.split(/\./);
	val1 = new Number(IP[0]);
	val2 = new Number(IP[1]);
	val3 = new Number(IP[2]);
	val4 = new Number(IP[3]);
	return (val1.valueOf() + '.' + val2.valueOf() + '.' + val3.valueOf() + '.' + val4.valueOf() )
}

function IpToLong(addr) {
	var	IP;
	var val1, val2, val3, val4, longIP;
	IP = addr.split(/\./);
	val1 = new Number(IP[0]);
	val2 = new Number(IP[1]);
	val3 = new Number(IP[2]);
	val4 = new Number(IP[3]);
    longIP = val1 * 0x1000000 + val2 * 0x10000 + val3 * 0x100 + val4;
	return longIP;
}
*/
function isValidMASK (addr)
{
    var sub_addr;
	data = addr.match(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/);
	if (!data || !addr) return false;

    if (addr.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/) == -1)
        return false;
    sub_addr = addr.split(/\./);
    if(sub_addr.length != 4) return false;
    if (addr.lastIndexOf(".") == (addr.length-1))
		return false;

    ///if(sub_addr[3] == "*")
    ///	sub_addr[3] = "1";
    ///else
    ///{
    	if(isNValidIP(sub_addr[0]) == true) return false;
    	if(isNValidIP(sub_addr[1]) == true) return false;
    	if(isNValidIP(sub_addr[2]) == true) return false;
    	if(isNValidIP(sub_addr[3]) == true) return false;
    ///}

    return true;
}

function isValidIP (addr)
{
    var sub_addr;
    var net_id;
    var host_id;

	data = addr.match(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/);
	if (!data || !addr) return false;

    if (addr.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\./) == -1)
        return false;
    sub_addr = addr.split(/\./);
    if(sub_addr.length != 4) return false;
    if (addr.lastIndexOf(".") == (addr.length-1))
		return false;

    ///if(sub_addr[3] == "*")
    ///	sub_addr[3] = "1";
    ///else
    ///{
    	if(isNValidIP(sub_addr[0]) == true) return false;
    	if(isNValidIP(sub_addr[1]) == true) return false;
    	if(isNValidIP(sub_addr[2]) == true) return false;
    	if(isNValidIP(sub_addr[3]) == true) return false;
    ///}

    if (sub_addr[0] > 0xff || sub_addr[1] > 0xff || sub_addr[2] > 0xff || sub_addr[3] > 0xff)
        return false;

    if(sub_addr[0] < 128) /* A class */
    {
        if(sub_addr[0] == 0 || sub_addr[0] == 127)
            return false;
        host_id = sub_addr[1] * 0x10000 + sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(host_id == 0 || host_id == 0xffffff)
            return false;
    }
    else if(sub_addr[0] < 192) /* B class */
    {
        host_id = sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(host_id == 0 || host_id == 0xffff)
            return false;
    }
    else if(sub_addr[0] < 224) /* C class */
    {
        host_id = sub_addr[3] * 0x1;
        if(host_id == 0 || host_id == 0xff)
            return false;
    }
    else  /* Limit broadcast, Multicast net */
    {
        return false;
    }

    return true;
}

// allow 0.0.0.0
function isValid_Zero_IP (addr)
{
    var sub_addr;
    var net_id;
    var host_id;
	data = addr.match(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/);
	if (!data || !addr) return false;

    if (addr.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/) == -1)
        return false;
    sub_addr = addr.split(/\./);
    if(sub_addr.length != 4) return false;
    if (addr.lastIndexOf(".") == (addr.length-1))
			return false;

   	if(isNValidIP(sub_addr[0]) == true) return false;
   	if(isNValidIP(sub_addr[1]) == true) return false;
   	if(isNValidIP(sub_addr[2]) == true) return false;
   	if(isNValidIP(sub_addr[3]) == true) return false;

    if (sub_addr[0] > 0xff || sub_addr[1] > 0xff || sub_addr[2] > 0xff || sub_addr[3] >= 0xff)
        return false;
	if(sub_addr[0] == 0 && sub_addr[1] == 0 && sub_addr[2] == 0 && sub_addr[3] == 0)
		return true;

		if(sub_addr[3] == 0) //could not allow last field is 0 when other fields are none 0.
			return false;

    if(sub_addr[0] < 128) /* A class */
    {
        if(sub_addr[0] == 0 || sub_addr[0] == 127)
            return false;
        host_id = sub_addr[1] * 0x10000 + sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(host_id == 0xffffff)
            return false;
    }
    else if(sub_addr[0] < 192) /* B class */
    {
        host_id = sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(host_id == 0xffff)
            return false;
    }
    else if(sub_addr[0] < 224) /* C class */
    {
        host_id = sub_addr[3] * 0x1;
        if(host_id == 0xff)
            return false;
    }
    else  /* Limit broadcast, Multicast net */
    {
        return false;
    }

    return true;
}

function isValid_Zero_IP2 (addr)
{
    var sub_addr;
    var net_id;
    var host_id;
	data = addr.match(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/);
	if (!data || !addr) return false;

    if (addr.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/) == -1)
        return false;
    sub_addr = addr.split(/\./);
    if(sub_addr.length != 4) return false;
    if (addr.lastIndexOf(".") == (addr.length-1))
			return false;

   	if(isNValidIP(sub_addr[0]) == true) return false;
   	if(isNValidIP(sub_addr[1]) == true) return false;
   	if(isNValidIP(sub_addr[2]) == true) return false;
   	if(isNValidIP(sub_addr[3]) == true) return false;

    if (sub_addr[0] > 0xff || sub_addr[1] > 0xff || sub_addr[2] > 0xff || sub_addr[3] > 0xff)
        return false;
	if(sub_addr[0] == 0 && sub_addr[1] == 0 && sub_addr[2] == 0 && sub_addr[3] == 0)
		return true;

    if(sub_addr[0] < 128) /* A class */
    {
        if(sub_addr[0] == 0 || sub_addr[0] == 127)
            return false;
        host_id = sub_addr[1] * 0x10000 + sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(host_id == 0xffffff)
            return false;
    }
    else if(sub_addr[0] < 192) /* B class */
    {
        host_id = sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(host_id == 0xffff)
            return false;
    }
    else if(sub_addr[0] < 224) /* C class */
    {
        host_id = sub_addr[3] * 0x1;
        if(host_id == 0xff)
            return false;
    }
    else  /* Limit broadcast, Multicast net */
    {
        return false;
    }

    return true;
}

function isNValidNum_ZERO(s) {
	if((isNaN(s))||(isNValidInt(s))||(isNegInt(s)))
		return true;
	else
		return false;
}

// virtual server
var VIRTUAL_SERVER_NUMBER_PER_INTERFACE = 20;
var VIRTUAL_SERVER_TOTAL_COUNT = 20;

function VIRTUAL_SERVER_ENTRY()
{
	var i;
	this.index = -1; // ignore
	this.enable = 0;
	this.protocol = 0; // ignore
	this.ip_count = 0;
	this.ip = new Array(IP_MAX);
	for(i=0; i < IP_MAX; i++)
		this.ip[i] = new IP_RANGE();

	this.lan_ip = "";

	this.port_count = 0;
	this.port = new Array(PORT_MAX);
	for(i=0; i < PORT_MAX; i++)
		this.port[i] = new PORT_RANGE();

	this.lan_port_count = 0;
	this.lan_port = new Array(PORT_MAX);
	for(i=0; i < PORT_MAX; i++)
		this.lan_port[i] = new PORT_RANGE();
};

function VIRTUAL_SERVER_TABLE()
{
	var i;
	this.interface_num = 0;
	this.count = 0; // ignore
	this.entry = new Array(VIRTUAL_SERVER_NUMBER_PER_INTERFACE);
	for(i=0; i<VIRTUAL_SERVER_NUMBER_PER_INTERFACE; i++)
		this.entry[i] = new VIRTUAL_SERVER_ENTRY();
}

var tcp_proto = 6;
var udp_proto = 17;
var both_proto = 0;
var icmp_proto = 1;
function protocolIsSame( protocol1, protocol2 )
{
	if( protocol1==both_proto || protocol2==both_proto )  return 1 ; // TCP&UDP <-> TCP or UDP
	if( protocol1==protocol2 )  return 1 ; // TCP<->TCP, UDP<->UDP
	return 0 ;
}

function PortRangeOverlap( port1, port2 )
{
	// check arr2.b_port
	if( port1.b_port <= port2.b_port && port2.b_port <= port1.e_port )
		return true ;
	// check arr2.e_port
	if( port1.b_port <= port2.e_port && port2.e_port <= port1.e_port )
		return true ;
	// check arr1.b_port
	if( port2.b_port <= port1.b_port && port1.b_port <= port2.e_port )
		return true ;
	// check arr1.e_port
	if( port2.b_port <= port1.e_port && port1.e_port <= port2.e_port )
		return true ;
	return false ;
}

function IsServicePort( protocol_type, port_arr, service_port_arr, ignore_service_name_arr,
							msg_callback, field_title  )
{
	var i, j, k ;

	if( port_arr==null || service_port_arr==null ) return false ;

	for( i = 0 ; i < port_arr.length ; i++ )
	{
		if( port_arr[i].b_port==0 || port_arr[i].e_port==0 ) continue ;

		for( j = 0 ; j < service_port_arr.length ; j++ )
		{
			if( service_port_arr[j].port_range.b_port==0 || service_port_arr[j].port_range.e_port==0 )
				continue ;
			if(	!protocolIsSame(protocol_type, service_port_arr[j].port_range.protocol) )
				continue;
			if( !PortRangeOverlap( port_arr[i], service_port_arr[j].port_range ) )
				continue;
			if( ignore_service_name_arr != null ){
				for( k=0 ; k < ignore_service_name_arr.length ; k++ ){
					if( ignore_service_name_arr[k]==service_port_arr[j].service_name ) // the same service, ignore
						break;
				}
				if( k != ignore_service_name_arr.length ) // found
					continue ;
			}

			var port_s = service_port_arr[j].port_range.b_port ;
			if(service_port_arr[j].port_range.e_port!=service_port_arr[j].port_range.b_port)
				port_s += ' ~ '+ service_port_arr[j].port_range.e_port ;
			var pro_s;
			if(service_port_arr[j].port_range.protocol==tcp_proto) pro_s = 'TCP'
			else if(service_port_arr[j].port_range.protocol==udp_proto) pro_s = 'UDP'
			//else						 pro_s = 'TCP&UDP'

			if( msg_callback )
				msg_callback( field_title, pro_s, port_s ) ;
			else
				; //alert( field_title+" can't include "+pro_s+" Port "+port_s+" ! Please configure another port range." );

			return true ;
		}
	}

	return false;
}

function IsUsedByPortMapping( protocol_type, port_arr, virtual_server )
{
	var i;

	if( port_arr==null || virtual_server==null )
		return false ;

	for( i=0 ; i < virtual_server.entry.length ; i++ ){
		if ( virtual_server.entry[i].enable==0 ) continue ;
		if (!protocolIsSame(protocol_type, virtual_server.entry[i].protocol)) continue ;

   		if( containDuplicatePort( port_arr, virtual_server.entry[i].port ) )
   			return true ;
	}

	return false ;
}

// parse port range string

function parseValueRange(value)
{
	var sub_value, range, sub_range;
	var i;
	var value1, value2;
    var dash;

	// check '*'
	if(value == null || value.length == 0) return null;
	if(value.length == 1)
	{
		if(value == '*')
		{
			range = new Array(1);
			range[0] = new PORT_RANGE();
			range[0].b_port = 1;
			range[0].e_port = 65535;
			return range;
		}
	}

	sub_value = value.split(/\,/);
	range = new Array(sub_value.length);
	for(i=0; i < sub_value.length; i++)
	{
		range[i] = new PORT_RANGE();
        dash = sub_value[i].indexOf("-");
		sub_range = sub_value[i].split(/\-/);
		if(sub_range.length == 1)
		{
            if(dash != -1) return null;
			if(isNaN(sub_range[0]) == true) return null;
			range[i].b_port = range[i].e_port = sub_range[0];
		}
		else if(sub_range.length == 2)
		{
			if(isNaN(sub_range[0]) == false && sub_range[1] == '*')
			{
				value1 = new Number(sub_range[0]);
				range[i].b_port = sub_range[0];
				range[i].e_port = 65535;
			}
			else
			{
				if(isNaN(sub_range[0]) == true || isNaN(sub_range[1]) == true) return null;
				value1 = new Number(sub_range[0]);
				value2 = new Number(sub_range[1]);
				if(value1.valueOf() > value2.valueOf())
				{
					range[i].b_port = sub_range[1];
					range[i].e_port = sub_range[0];
				}
				else
				{
					range[i].b_port = sub_range[0];
					range[i].e_port = sub_range[1];
				}
			}
		}
		else
			return null;
	}
	return range;
}

/* we no use it,marked by Jack
function parseIPValueRange(value)
{
	var sub_value, range, sub_range;
	var i;
	var value1, value2;
	var sub_addr;

	// check '*'
	if(value == null || value.length == 0) return null;
	if(value.length == 1)
	{
		if(value == '*')
		{
			range = new Array(1);
			range[0] = new IP_RANGE();
			range[0].ip = "0.0.0.0";
			range[0].count = 1;
			return range;
		}
		return null;
	}

	sub_value = value.split(/\,/);
	range = new Array(sub_value.length);
	for(i=0; i < sub_value.length; i++)
	{
		range[i] = new IP_RANGE();
		sub_range = sub_value[i].split(/\-/);
		if(sub_range.length == 1)
		{
			if(isValidIP(sub_range[0]) == false) return null;
    		sub_addr = sub_range[0].split(/\./);
    		if(sub_addr[3] == "*")
    		{
    			sub_addr[3] = "1";
				range[i].ip = sub_addr[0] + "." + sub_addr[1] + "." + sub_addr[2] + "." + sub_addr[3];
				range[i].ip = parseIP(range[i].ip);
				range[i].count = 254;
    		}
    		else
    		{
				range[i].ip = sub_addr[0] + "." + sub_addr[1] + "." + sub_addr[2] + "." + sub_addr[3];
				range[i].ip = parseIP(range[i].ip);
				range[i].count = 1;
    		}
		}
		else if(sub_range.length == 2)
		{
			if(isValidIP(sub_range[0]) == false || isNaN(sub_range[1]) == true ) return null;
    		sub_addr = sub_range[0].split(/\./);
			value1 = new Number(sub_addr[3]);
			value2 = new Number(sub_range[1]);
			if(value1.valueOf() > value2.valueOf())
			{
				range[i].ip = sub_addr[0] + "." + sub_addr[1] + "." + sub_addr[2] + "." + value2.valueOf();
				range[i].count = value1.valueOf() - value2.valueOf() + 1;
			}
			else
			{
				range[i].ip = sub_addr[0] + "." + sub_addr[1] + "." + sub_addr[2] + "." + value1.valueOf();
				range[i].count = value2.valueOf() - value1.valueOf() + 1;
			}
		}
		else
			return null;
	}
	return range;
}
*/
/* we no use it,marked by Jack
function trueValueToInt(inValue) {

	//alert("inValue:" + inValue);
	if (inValue)
		return 1;
	else
		return 0;
}
*/
/* we no use it,marked by Jack
function intValueToBool(inValue) {

	//alert("inValue:" + inValue);
	if (inValue == 0)
		return false;
	else
		return true;
}
*/
function PortOverlap( port_arr1, port_arr2 )
{
	var x, y ;

	if( port_arr1==null || port_arr2==null ) return -1 ;
	for( x = 0 ; x < port_arr1.length ; x++ ){
		if( port_arr1[x].b_port==0 || port_arr1[x].e_port==0 ) continue ;

		for( y = 0 ; y < port_arr2.length ; y++ ){
			if( port_arr2[y].b_port==0 || port_arr2[y].e_port==0 ) continue ;

			// check arr2.b_port
			if( port_arr1[x].b_port<=port_arr2[y].b_port && port_arr2[y].b_port <= port_arr1[x].e_port )
				return y ;
			// check arr2.e_port
			if( port_arr1[x].b_port<=port_arr2[y].e_port && port_arr2[y].e_port <= port_arr1[x].e_port )
				return y ;
			// check arr1.b_port
			if( port_arr2[y].b_port<=port_arr1[x].b_port && port_arr1[x].b_port <= port_arr2[y].e_port )
				return y ;
			// check arr1.e_port
			if( port_arr2[y].b_port<=port_arr1[x].e_port && port_arr1[x].e_port <= port_arr2[y].e_port )
				return y ;
		}
	}
	return -1 ;
}
function containDuplicatePort( port_arr1, port_arr2 )
{
	if( PortOverlap( port_arr1, port_arr2 ) >= 0 )
		return true;

	return false;
}

function isSpecialChar(s, name) {
	var src = new String(s);
	var lst = new String("\\\"\'");
	var i, n;

	n = lst.length;
	for (i=0; i<n; i++) {
		var c = lst.charAt(i);
		var tmpS = new String(c);
		if (src.indexOf(tmpS) != -1) {
			alert(name + ": \\ , \" and \' are invalid.");
			return true;
		}
	}

	return false;
}

function isSpecialChar2(s, name) {
	var src = new String(s);
	var lst = new String("\\\"\';");
	var i, n;

	n = lst.length;
	for (i=0; i<n; i++) {
		var c = lst.charAt(i);
		var tmpS = new String(c);
		if (src.indexOf(tmpS) != -1) {
			alert(name + ": \\ , \", ; and \' are invalid.");
			return true;
		}
	}

	return false;
}
/* we no use it,marked by Jack
ipmULastIP='The first entry of IP address must be within range 1 - 254.';
*/
function MM_swapImgRestore() { //v3.0
  var i,x,a=document.MM_sr; for(i=0;a&&i<a.length&&(x=a[i])&&x.oSrc;i++) x.src=x.oSrc;
}
/* we no use it,marked by Jack
function MM_preloadImages() { //v3.0
  var d=document; if(d.images){ if(!d.MM_p) d.MM_p=new Array();
    var i,j=d.MM_p.length,a=MM_preloadImages.arguments; for(i=0; i<a.length; i++)
    if (a[i].indexOf("#")!=0){ d.MM_p[j]=new Image; d.MM_p[j++].src=a[i];}}
}*/
function MM_findObj(n, d) { //v4.0
  var p,i,x;  if(!d) d=document; if((p=n.indexOf("?"))>0&&parent.frames.length) {
    d=parent.frames[n.substring(p+1)].document; n=n.substring(0,p);}
  if(!(x=d[n])&&d.all) x=d.all[n]; for (i=0;!x&&i<d.forms.length;i++) x=d.forms[i][n];
  for(i=0;!x&&d.layers&&i<d.layers.length;i++) x=MM_findObj(n,d.layers[i].document);
  if(!x && document.getElementById) x=document.getElementById(n); return x;
}

function MM_swapImage() { //v3.0
  var i,j=0,x,a=MM_swapImage.arguments; document.MM_sr=new Array; for(i=0;i<(a.length-2);i+=3)
   if ((x=MM_findObj(a[i]))!=null){document.MM_sr[j++]=x; if(!x.oSrc) x.oSrc=x.src; x.src=a[i+2];}
}

function isOnebyte(x)
{
	return (!isNaN(x) && x >= 0 && x <= 255);
}

function isValidSubnetMask(){
	var args = isValidSubnetMask.arguments;
	var j=0;
	var s;

	if(args.length < 1)
		return false;
	switch (args.length)
	{
		case 4: // for complete subnet mask, check for special last section
			s = args[0] + "." + args[1] + "." + args[2] + "." + args[3];
			break;
		case 1:
			s = args[0];
	}

	s = String(s).replace(/^\s+|\s+$/g,"");//remove space
	var result = s.match(/^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/);

	if (result && result.length == 5 &&  isOnebyte(result[1]) && isOnebyte(result[2]) && isOnebyte(result[3]) && isOnebyte(result[4]))
	{
		var last_bit = 1; // accept 255.255.255.255
		var cnt = 0;
		for (var i=1; i <= 4; i++)
		{
			var mask = parseInt(result[i], 10);

			for (var j=7; j >= 0; j--)
			{
				var n = Math.pow(2, j);

				var bit_on = (mask & n) ? 1 : 0;
				if (last_bit != bit_on)
				{
					last_bit = bit_on;
					cnt++;    //no change or Just change once,from 1 to 0  : subnet mask
				}
			}
		}
		if(cnt > 1)
		{
			//alert("Invalid NetMask");
			return false;
		}
		else
		{
			return true;
		}
	}

	//alert("Invalid NetMask");
	return false;
}

function isValidNetID(addr, mask)
{
	var sub_addr;
	var data = addr.match(/^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/);
	if (!data || !addr)
	{
		return false;
	}

	if (addr.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\./) == -1)
	{
		return false;
	}

	if (!isValidSubnetMask(mask)) //validate netmask
	{
		return false;
	};

	sub_addr = addr.split(/\./);
	if(sub_addr.length != 4)
	{
		return false;
	}
	if (addr.lastIndexOf(".") == (addr.length-1))
	{
		return false;
	}

	if(isNValidIP(sub_addr[0]) == true)
	{
		return false;
	}
	if(isNValidIP(sub_addr[1]) == true)
	{
		return false;
	}
	if(isNValidIP(sub_addr[2]) == true)
	{
		return false;
	}
	if(isNValidIP(sub_addr[3]) == true)
	{
		return false
	}

	if (sub_addr[0] > 0xff || sub_addr[1] > 0xff || sub_addr[2] > 0xff || sub_addr[3] > 0xff)
	{
		return false;
	}

	if(sub_addr[0] == 0 && sub_addr[1] == 0 && sub_addr[2] == 0 && sub_addr[3] == 0)
	{
		return false;
	}

	if(sub_addr[0] == 0 || sub_addr[0] == 127 || sub_addr[0] > 223)
	{
		return false;
	}

	var sub_mask = mask.split(/\./);
	for(var i = 0; i < sub_mask.length; i++)
	{
		if((parseInt(sub_mask[i]) & parseInt(sub_addr[i])) !== parseInt(sub_addr[i]))
		{
			//console.log("ipaddr is not a valid netip.");
			return false;
		}
	}

	return true;
}

function get_larger_netmask(mask1, mask2)
{
	var sub_mask1 = mask1.split(/\./);
	var sub_mask2 = mask2.split(/\./);
	var result_mask = [];
	if(sub_mask1.length != 4 || sub_mask2.length != 4)
	{
		return "0.0.0.0"; //error
	}

	for(var i = 0; i < 4; i++)
	{
		result_mask.push(parseInt(sub_mask1[i]) & parseInt(sub_mask2[i]));
	}
	return result_mask[0]+"."+result_mask[1]+"."+result_mask[2]+"."+result_mask[3];
}

function validate_ip_conflict(ip1, ip2, mask)
{
	var sub_addr1 = ip1.split(/\./);
	var sub_addr2 = ip2.split(/\./);
	var sub_mask = mask.split(/\./);

	if(sub_addr1.length != 4 || sub_addr2.length != 4 || sub_mask.length != 4)
	{
		return false;
	}
	if((( parseInt(sub_addr1[0]) & parseInt(sub_mask[0]) ) == (parseInt(sub_addr2[0]) & parseInt(sub_mask[0])))
	 && (( parseInt(sub_addr1[1]) & parseInt(sub_mask[1]) ) == (parseInt(sub_addr2[1]) & parseInt(sub_mask[1])))
	 && (( parseInt(sub_addr1[2]) & parseInt(sub_mask[2]) ) == (parseInt(sub_addr2[2]) & parseInt(sub_mask[2])))
	 && (( parseInt(sub_addr1[3]) & parseInt(sub_mask[3]) ) == (parseInt(sub_addr2[3]) & parseInt(sub_mask[3])))
	)
	{
		return false;
	}
	//for(var i = 0; i < 4; i++)
	//{
		//if((parseInt(sub_addr1[i]) & parseInt(sub_mask[i])) === (parseInt(sub_addr2[i]) & parseInt(sub_mask[i])))
		//{
			//console.log("ip1:"+sub_addr1[i] + " ip2:"+sub_addr2[i] +" mask:"+sub_mask[i]);
			//return false;
		//}
	//}
	return true;
}
