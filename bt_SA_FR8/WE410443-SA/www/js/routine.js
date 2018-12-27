

/******************Qsetup***DELETE************/
function Interface_T(){
	this.ip = new Array(4);
	this.mask = new Array(4);
	this.gateway = new Array(4);

	var i;
	for(i=0; i<4; i++)
	{
		this.ip[i] = 0;
		this.mask[i] = 0;
		this.gateway[i] = 0;
	}
};
/******************************************************/
function convert_specific_char_common(object) {
	var c;
	var i;
	var specific_char1;
	var specific_char2;
	var str = object.toString();

	for(i = 0;i < object.length; i++)
	{
		c = object.charCodeAt(i);
		if (c == (34 + 127))
		{
			specific_char1 = String.fromCharCode(34);
			specific_char2 = String.fromCharCode(34 + 127);
			str = str.replace(specific_char2, specific_char1);
		}
	}

	//console.log("----> convert_specific_char_common" + str);
	return str;
}

function convert_common_char_specific(object) {
	var c;
	var i;
	var specfic_char;
	var specific_int;
	var str = object.toString();

	for(i = 0;i < object.length; i++)
	{
		c = object.charAt(i);
		if ('\"' == c)
		{
			specific_int = object.charCodeAt(i);
			specfic_char = String.fromCharCode(specific_int + 127);
			str = str.replace("\"", specfic_char);
		}
	}

	//console.log("----> convert_common_char_specific" + str);
	return str;
}

function isNvalidChar(s){
	var expChar=new RegExp('^[._!~*\'()=+$,?:#%-\/A-Za-z0-9]+$');
	if (expChar.test(s) == false) {
			return true;
		}
	return false;
}

function isBlank(s){
	var i;
	var c;
	for(i=0;i<s.length;i++)
	{
		c=s.charAt(i);
		if((c!=' ')&&(c!='\n')&&(c!='\t'))return false;
	}
	return true;
}

function isIncludeDQuote(s){
	var i;
	var c;
	for(i=0;i<s.length;i++)
	{
		c=s.charAt(i);
		if((c=='"'))return true;
	}
	return false;
}

function isIncludeBSlash(s){
	var i;
	var c;
	for(i=0;i<s.length;i++)
	{
		c=s.charAt(i);
		if((c=='\\'))return true;
	}
	return false;
}

function isBlank_Zero(s)
{
	var i;
	var c;
	for(i=0;i<s.length;i++){
		c=s.charAt(i);
		if((c!=' ')&&(c!='\n')&&(c!='\t')&&(c!='0'))return false;
	}
	return true;
}

function isNvaliduser(s) {
	var j,x = 0;
	for(var i = 0; i < s.length; i++) {
		var c = s.charAt(i);
		if ((c=='<') || (c=='>') || (c=='&') || (c==' '))
		{
			//alert(c);
			return true;
		}
	}

	return false;
}

function isNValidInt(s)
{
	var i, c;
	var str = s.toString();
	if(str.match(/^0+\d/))//not allow 0x or 00x
		return true;
	for (i=0; i<s.length; i++) {
		c = s.charCodeAt(i);
		if ((c < 48) || (c > 57))
			return true;
	}
	return false;
}

function isNPinValidInt(s)
{
	var i, c;
	var str = s.toString();
	for (i=0; i<s.length; i++) {
		c = s.charCodeAt(i);
		if ((c < 48) || (c > 57))
			return true;
	}
	return false;
}
function isNegInt(s)
{
	if (s<0)
		return true;
	else
		return false;
}

function isNValidIP(s) {
	if((isBlank(s))||(isNaN(s))||(isNValidInt(s))||(isNegInt(s))||(s<0||s>255))
		return true;
	else
		return false;
}
function isNValidFirstIP(_s) {
	var s=parseInt(_s,10);
	if((isBlank(''+_s))||(isNaN(''+_s))||(isNValidInt(s))||(isNegInt(s))||(s<1||s>223))
		return true;
	else
		return false;
}

function isNValidLastIP(s) {
	if((isBlank(s))||(isNaN(s))||(isNValidInt(s))||(isNegInt(s))||(s<1||s>254))
		return true;
	else
		return false;
}

//	Add new tool which accept l for lower bound, u for upper bound of IP entry limit
function isNValidIP_p(s, l, u) {
	if((isBlank(s)||(isNaN(s))||(isNValidInt(s))||(isNegInt(s))||(s<l||s>u)))
		return true;
	else
		return false;
}

function isNValidMask(s) {
	if ((s == 255) || (s == 254) || (s == 252) || (s == 248) || (s == 240) || (s == 224) || (s == 192) || (s == 128) || (s == 0))
		return false;
	var str = s.toString();
  if(str.match(/^0+\d/))//not allow 0x or 00x
		return true; 
	return true;
}

function isNValidLastMask(s) {
	if ( (s == 252) || (s == 248) || (s == 240) || (s == 224) || (s == 192) || (s == 128) || (s == 0) )
		return false;
	var str = s.toString();
	if(str.match(/^0+\d/))//not allow 0x or 00x
		return true; 
	return true;
}

// =========================================================
//  Convert input object or string into IP long format
//  Parameter:
//     IPlong(<form filed>)    e.x: IPlong(f.lan_ip);
//     IPlong(<arry>)          e.x: IPlong([192,168,2,1]);(drop now 2012/8/20)
//     IPlong(<string>)        e.x: IPlong('192.168.2.1');
//  Return:
//    unsigned long format
//    192.168.2.1       --> 0xc0a80201 (3232236033)
//    255.255.255.0     --> 0xffffff00 (4294967040)
//
//   >= 0: long value
//     -1: error
// =========================================================
function IP2long(){
	var ip=[];
	var obj=IP2long.arguments;
	if(obj.length == 1){
		if("string" == typeof(obj[0])){
			ip=obj[0].split(".");
		}else{
		//if("object" == typeof(obj)) {
			if(obj.length != 4) return -1;
			for(var i=0; i < 4; i++){
				ip[i]=(obj[0][i].value)? obj[0][i].value:obj[0][i];
			}
		//}else{
		//	return -1;
		}
	}else{
		ip=obj;
	}
	if(ip.length != 4) return -1;
    for(var i=0; i < 4; i++){
    	ip[i]=parseInt(ip[i],10);
    }
   var iplong = (Number(ip[0]) * 0x1000000) + (Number(ip[1]) * 0x10000) + (Number(ip[2]) * 0x100) + Number(ip[3]);
   return iplong;
}
// =========================================================
//  Get the Brocast IP address (long format)
//  parameter:
//       <ip string>, <mask string>
//  return
//      unsigned long format
// =========================================================
function BrocastLong(_ip, _msk){
	var ip=_ip.split(".");
	var msk=_msk.split(".");
	if((ip.length != 4) || (msk.length !=4)) return 0;
	for(var i=0; i < 4; i++) ip[i]=ip[i] | (~ msk[i] & 0xFF);
	return IP2long(ip[0],ip[1],ip[2],ip[3]);
}

// =========================================================
//  Check the input object or string is valid Net Mask or not
//  Parameter:
//   _obj
//      [array] type,             ex: [255,255,255,0] (drop now 2012/8/20)
//      [fields] (MUST 4 fields)  e.x: f.mask
//      [string] (with ".")       e.x: "255.255.255.0"
//  Return:
//   >= 0: long value
//     -1: error
// =========================================================

// return 0 mean ok
// 1~4 means erro fields
function isNValidSubnetMask1(_obj) {
	var msk=[];
	if("string" == typeof(_obj)){
		msk=_obj.split(".");
	}else{
	//if("object" == typeof(_obj)){
		if(_obj.length != 4) return 1;
	    for(var i=0; i < _obj.length; i++){
			msk[i]=(_obj[i].value)? _obj[i].value:_obj[i];
		}
	//}else{
	//	msk=_obj;
	}
	if(msk.length != 4){
		return 1; //point to first
	}
	for(var i=0; i < 4; i++){
		if(isNValidInt(msk[i])){
			return (i+1);
		}
	}
    var ulMask = IP2long(msk[0],msk[1],msk[2],msk[3]);
    var j=0;
    var ok=0;

    if (Number(msk[0]) != 255) return 1;
    if (isNValidMask(msk[1])) return 2;
    if (isNValidMask(msk[2])) return 3;
    if (isNValidLastMask(msk[3])) return 4;

    for(var i = 31; i >= 0; i--) {
		j = j + Math.pow(2,i);
		if (ulMask==j) ok=1;
    }
    return (ok)? 0: 1;
}
function isNValidSubnetMask(s1, s2, s3, s4) {
	var ulMask = Number(s1) * 0x1000000 + Number(s2) * 0x10000 + Number(s3) * 0x100 + Number(s4);
	var j=0;
	var ok=0;

	if (s1 != 255)
		return true;

	if ((isNValidMask(s2)) || (isNValidMask(s3)) || (isNValidLastMask(s4)))
		return true;
	for(var i = 31; i >= 0; i--) {
			j = j + Math.pow(2,i);
			if (ulMask==j) ok=1;
	}
	return (ok)? false: true;

/*
	if (s2 != 255)
	{
		if ((s3 != 0) || (s4 != 0))
			return true;
	}

	if ((s3 != 255) && (s3 != 0))
	{
		if ((s2 != 255) || (s4 != 0))
			return true;
	}
*/
	return false;
}

//  Parameter:
//     isNValidAddress(<form filed>)    e.x: IPlong(f.lan_ip);
//     isNValidAddress(<string>)        e.x: IPlong('192.168.2.1');
// return:
// 0: no error
//    1~4: <1~4>th digits error
//
function isNValidAddress(_obj){
	var ip=[];
	if("string" == typeof(_obj)){
		ip=_obj.split(".");
	}else{
	    if(_obj.length != 4) return 1;
	    for(var i=0; i < _obj.length; i++){
			ip[i]=_obj[i].value;
		}
	}
	if(ip.length != 4){
		return 1; //point to first
	}
	// 1th digits
	if(isNValidFirstIP(ip[0])) { return 1;}
	if(isNValidIP(ip[1])) {return 2;}
	if(isNValidIP(ip[2])) {return 3;}
	if(isNValidLastIP(ip[3])) {return 4;}
	return 0;
}

function isNValidPort(s) {
	/* If the port number starts with a 0, treat as invalid port */
	if (s.charAt(0) == '0')
		return true;

	if((isBlank(s))||(isNaN(s))||(isNValidInt(s))||(isNegInt(s))||(s<1||s>65535))
		return true;
	else
		return false;
}

function isNValidPortAllowZero(s) {
	if((isBlank(s))||(isNaN(s))||(isNValidInt(s))||(isNegInt(s))||(s<0||s>65535))
		return true;
	else
		return false;
}

function is2Hex(s) {
	var j,x = 0;
	for(var i = 0; i < s.length; i++) {
		var c = s.charAt(i);
		j = parseInt(c,16);
		if( (j>=0) && (j <=16)){
			if(x==1) return true;
			x=1;
		}
	}
	return false;
}

function isHex(s) {
	var j, x = 0;
	for (var i = 0 ; i < s.length; i++) {
		var c = s.charAt(i);
		j = parseInt(c, 16);
		if (!((j == 0) || (j == 1)|| (j == 2)|| (j == 3)|| (j == 4)|| (j == 5)|| (j == 6)
			|| (j == 7)|| (j == 8)|| (j == 9)|| (j == 10)|| (j == 11)|| (j == 12)|| (j == 13)
			|| (j == 14)|| (j == 15))) {
			x = 1;
		}
		if (x == 1) return false;
	}
	return true;
}

function isNValid(s) {
	if( isBlank(s) || is2Hex(s))
		return true;
	else
		return false;
}

function isNValids(s) {
	if( isBlank(s) || isHex(s))
		return true;
	else
		return false;
}

function isValidMacAddress(address) {
   var c = '';
   var i = 0, j = 0;

   if ( address == 'ff:ff:ff:ff:ff:ff' ) return false;
   if(address == '00:00:00:00:00:00' || address == '0:0:0:0:0:0' ) return false;

   addrParts = address.split(':');
   if ( addrParts.length != 6 ) return false;

   for (i = 0; i < 6; i++) {
      for ( j = 0; j < addrParts[i].length; j++ ) {
         c = addrParts[i].toLowerCase().charAt(j);
         if ( (c >= '0' && c <= '9') ||
              (c >= 'a' && c <= 'f') )
            continue;
         else
            return false;
      }
   }
	 var m2 = parseInt(addrParts[0].charAt(1), 16);
	 if((m2 & 1) == 1){
//			alert('The second character of MAC must be even number : [0, 2, 4, 6, 8, A, C, E]');
		 return false;
	 }

   return true;
}

/* we no use it, 20091218
function MM_openBrWindow(theURL,winName,features){
  window.open(theURL,winName,features);
}

function getElementsByFieldName(target_form, field)
{
 var i;
 var form;
 var value;
 if(target_form == null || field == null) return -1;
 for(i=0; i<target_form.length; i++)
 {
  if(target_form.elements[i].name == field)
   return i;
 }
 return -1;
}
*/
// if invalide number
function isNValidNum(s) {
	if((isBlank(s)) || (isNaN(s))||(isNValidInt(s))||(isNegInt(s)))
		return true;
	else
		return false;
}
// if invalide number and not under the range(not equal to)
function isNValidNumRange(s, min, max) {
	if(isNValidNum(s)) return true;
    var v=parseInt(s,10);
    if(v < min || v > max) return true;
	return false;
}

function isNValidUnicastIP(s) {
	if((isBlank(s))||(isNaN(s))||(isNValidInt(s))||(isNegInt(s))||(s<=0||s>=224))
		return true;
	else
		return false;
}
/* we no use it, 20091218
ipmUcast='The first entry of IP address must be within range 1 - 223.';

function MM_jumpMenu(targ,selObj,restore){ //v3.0
  eval(targ+".location='"+selObj.options[selObj.selectedIndex].value+"'");
  if (restore) selObj.selectedIndex=0;
}
*/
/*
check 4 digits range:
 1st : < 128(A), <192 (B), < 224 (C)
 2nd : 0-255
 3nd : 0-255
 4nd : 1-254(!nobypass), 0-255 (nobypass)
 all : Not allow 0x or 00x
*/
function isValidIP (addr, _nobypass)
{
    var sub_addr;
    var net_id;
    var host_id;
    var nobypass=_nobypass;

    if (addr.search(/^\d{1,3}\.\d{1,3}\.\d{1,3}\./) == -1)
        return false;
    sub_addr = addr.split(/\./);
    if(sub_addr.length < 4) return false;
    
    for (var x=0;x<4;x++)
    {
    	if(sub_addr[x].match(/^0+\d/))//not allow 0x or 00x
    	return false; 
    }
    
    if(sub_addr[3] == "*")
    	sub_addr[3] = "1";
    else
    {
    	if(isNaN(sub_addr[3]) == true) return false;
    }

    if (sub_addr[0] > 0xff || sub_addr[1] > 0xff || sub_addr[2] > 0xff || sub_addr[3] > 0xff)
        return false;

    if(sub_addr[0] < 128){ /* A class */
        if(sub_addr[0] == 0 || sub_addr[0] == 127)
            return false;
        host_id = sub_addr[1] * 0x10000 + sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(!nobypass && (host_id == 0 || host_id == 0xffffff))
            return false;
    }else if(sub_addr[0] < 192) { /* B class */
        host_id = sub_addr[2] * 0x100 + sub_addr[3] * 0x1;
        if(!nobypass && (host_id == 0 || host_id == 0xffff))
            return false;
    }else if(sub_addr[0] < 224){ /* C class */
        host_id = sub_addr[3] * 0x1;
        if(!nobypass && (host_id == 0 || host_id == 0xff))
            return false;
    }else{
        return false;
    }

    return true;
}
//heyu_20070227 +++ for #329
// isValidIPAddress
// Allows to check one or more sections of an IP address to be numerical and within the correct range
// Expects the references to the textbox input control objects as arguments
// Used in all pages with IP address input
function isValidIPAddress(){
  var net_id;
  var host_id;
  var args = isValidIPAddress.arguments;
  if(args.length < 1)
    return false;
  switch (args.length) {
    case 4: // for complete IP address, check for special first and last section


	  if(isNValidIP(args[0].value) == true) return 1;
      if(isNValidIP(args[1].value) == true) return 2;
      if(isNValidIP(args[2].value) == true) return 3;
      if(isNValidIP(args[3].value) == true) return 4;

      if (args[0].value > 0xff || args[1].value > 0xff || args[2].value > 0xff || args[3].value > 0xff)

        return 1;//heyu 20060227 add for valid IP NTC bug 16

      if(args[0].value < 128) /* A class */
      {
        if(args[0].value == 0 || args[0].value == 127)
            return 1;
        host_id = parseInt(args[1].value, 10) * 0x10000 + parseInt(args[2].value, 10) * 0x100 + parseInt(args[3].value, 10) * 0x1; //heyu_20070227 for #329

        if(host_id == 0 || host_id == 0xffffff)
            return 2;
      }
      else if(args[0].value < 192) /* B class */
      {
        host_id = parseInt(args[2].value, 10) * 0x100 + parseInt(args[3].value, 10) * 0x1;//heyu_20070227 for #329

        if(host_id == 0 || host_id == 0xffff)
            return 3;
      }
      else if(args[0].value < 224) /* C class */
      {
        host_id = args[3].value * 0x1;
        if(host_id == 0 || host_id == 0xff)
            return 4;
      }
      else  /* Limit broadcast, Multicast net */
      {
        return 1;//heyu 20060227 revise for valid IP NTC bug 16
      }
      if (args[0].value == 10 && args[1].value == 1 &&  args[2].value == 1 &&  args[3].value == 1) //heyu_20060227 for '10.1.1.1' is not valid IP

      {
     	return 1;//heyu 20060227 add for valid IP NTC bug 16
    }
      break;
    default:
      for(var i=0; i<args.length; i++) {
        if(intCheck(args[i], 10, 0, 255) == false) {
          return (i+1);
        }
      }
  }
  return 0;
}

function isValidIPAddress3(a1,a2,a3,a4,m1,m2,m3,m4){
  var net_id;
  var host_id;
	  if(isNValidIP(a1) == true) return 1;
      if(isNValidIP(a2) == true) return 2;
      if(isNValidIP(a3) == true) return 3;
      if(isNValidIP(a4) == true) return 4;
      net_id =parseInt(m1, 10) * 0x1000000 +parseInt(m2, 10) *
0x10000 + parseInt(m3, 10) * 0x100 + parseInt(m4, 10) * 0x1;

      host_id =parseInt(a1, 10) * 0x1000000 +parseInt(a2, 10) *
0x10000 + parseInt(a3, 10) * 0x100 + parseInt(a4, 10) * 0x1;

		if ((host_id|net_id)==-1) {//heyu_20070227 for #329
			return 4;//heyu_20070227 for #329
		}//heyu_20070227 for #329
		if ((host_id&net_id)==host_id) {//heyu_20070227 for #329
			return 4;//heyu_20070227 for #329
		}//heyu_20070227 for #329
	return 0;
}
//heyu_20070227 +++ for #329


function IpToLong(_ip){
  return IP2long(_ip); //IP2long is string mode
}
// #########################################
// Convert IP/Mask into long format subnet
// input : str
// return: IP long value
//
// #########################################

function IPSubNet(_ip, _mask){
	var ip  =IpToLong(_ip);
	var mask=IpToLong(_mask);
	var subnet = ip & mask;
	if (subnet < 0) subnet += 0x100000000;
	return subnet;
}
// #########################################
// Convert IP/Mask to find Brocast long format subnet
// input : str
// return: IP long value
//
// #########################################
function IPBrocastNet(_ip, _mask){
	var ip  =IpToLong(_ip);
	var mask=IpToLong(_mask);
	var subnet = ip & mask;
	if (subnet < 0) subnet += 0x100000000;
	return (subnet + (0xffffffff ^ mask));

}
/******************************/

// check _s is between 32~126,
// if _lt=1, then allow whitespace at leading and trailing
//
function isPrintable(_s, _lt){

	var c,len=_s.length-1;
	for(var i=0;i <=len;i++)	{
			c=_s.charCodeAt(i);
			// check if 32-126
		    if((c < 32) ||(c > 126)) return false;
		    // check leading character
		    if(!_lt){
		    	if((i==0) && (c== 32)) return false;
		    	if((i==len) && (c== 32)) return false;
		    }
	}
	return true;

}
function isNValidSpecialChar(_s) {
	var src = new String(_s);
	var lst = new String("\\\"\';");
	var i, n;

	n = lst.length;
	for (i=0; i<n; i++) {
		var c = lst.charAt(i);
		var tmpS = new String(c);
		if (src.indexOf(tmpS) != -1) {
			//alert(name + ": \\ , \", ; and \' are invalid.");
			return true;
		}
	}
	return false;
}
// #########################################
// Convert mask lengh into Mask IP
// input : long value
// return: str
//
// #########################################
function LenToMask(_len){
	var ip = "0.0.0.0";
	var i=0, j=0;
	var len, mod;
	var mask;

	ip = ip.split(".");
	len = _len / 8;
	mod = _len % 8;
	for (i=0; i<4; i++) {
		if (len > 0) {
			ip[i] = 255;
			len = len - 1;
		}
		else if (mod == 0) {
			ip[i] = 0;
		}
		else {
			ip[i] = 0;
			for(j=7; j>0; j--)
			{
				if(mod > 0)
				{
					ip[i] = ip[i] + Math.pow(2,j);
					mod = mod - 1;
				}
			}
		}
	}
	mask = ip[0]+"."+ip[1]+"."+ip[2]+"."+ip[3]+""

	return mask;
}
// #########################################
// Convert Mask IP into mask lengh
// input : str
// return: long value
//
// #########################################
function MaskToLen(_mask){
	var ulmask=IpToLong(_mask);
	var tmp = 0;
	var j = 0;

	for (j=31; j>0; j--) {
		tmp = tmp + Math.pow(2,j);
		if (ulmask==tmp)
			break;
	}
	var len = 32-j;

	return len;
}