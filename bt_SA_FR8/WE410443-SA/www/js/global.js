/*** check top*/
var non_top='index.htm';
// ++++++++++++++++++++
//  developer use
// ++++++++++++++++++++
var L_dbg=1;

var G_arcTrans=1;
/*DEMO*/
var cgi_lang_list='EN TW ES';
//var _httoken = "";
/*END_DEMO*/
/*REAL
var cgi_lang_list=<!--DEF#@PROJ_UI_LANG_LST@-->;
REAL*/


/*DEMO*/
var Frm_F={
"radio":'INPUT',
"checkbox":'INPUT',
"select-one":'SELECT',
"select-multiple":'SELECT',
"password":'INPUT',
"textarea":'TEXTAREA',
"text":'INPUT',
"hidden":'INPUT',
"file":'INPUT'
};
function getFormField(f){

  var h='<FORM NAME="'+f.name+'" METHOD='+f.method+'" ACTION="'+ f.action+'" TARGET="'+f.target+'">\n';
  var s;
  for(var x=0; x < f.elements.length;x++){
  	var obj=f.elements[x];
  	if(obj && obj.type ){
		op='';
		s=1;
		switch(obj.type){
		case "radio":
		case "checkbox":
			if(!obj.checked){s=0; break;}
		case "select-one":
		case "select-multiple":
		case "password":
		case "textarea":
		case "text":
		case "file":
		case "hidden":
			break;
		default:
			s=0;
			break;
		}
		if(s){
		h+='   <'+Frm_F[obj.type]+' TYPE="'+obj.type+'" '+
		   ' NAME="'+obj.name+'" VALUE="'+obj.value+'" '+op+' />\n';
		}
     }
  }
  h+='</FORM>';
  return h;
}
function Show_dbg(msg){
	var win = window.open("","devdbg");
	win.document.open();
	win.document.writeln('<textarea cols=100 rows=40>'+msg+'</textarea>');
	win.document.close();
	win.focus();
	return true;
}
/*END_DEMO*/
var CGI_PREFIX=("undefined" == typeof(CGI_PREFIX))? "":CGI_PREFIX;

/*****************************************************************************
 * md5.js
 *
 * A JavaScript implementation of the RSA Data Security, Inc. MD5
 * Message-Digest Algorithm.
 *
 * Copyright (C) Paul Johnston 1999. Distributed under the LGPL.
 *****************************************************************************/
/* to convert strings to a list of ascii values */
var sAscii = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ";
var sAscii = sAscii + "[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

/* convert integer to hex string !!! CGI use lowercase */
var sHex = "0123456789abcdef";
function hex(i) {
	h = "";
	for(j = 0; j <= 3; j++) {
			h += sHex.charAt((i >> (j * 8 + 4)) & 0x0F) +
					sHex.charAt((i >> (j * 8)) & 0x0F);
	}
	return h;
}

/* add, handling overflows correctly */
function add(x, y) {
	return ((x&0x7FFFFFFF) + (y&0x7FFFFFFF)) ^ (x&0x80000000) ^ (y&0x80000000);
}

/* MD5 rounds functions */
function R1(A, B, C, D, X, S, T) {
	q = add(add(A, (B & C) | (~B & D)), add(X, T));
	return add((q << S) | ((q >> (32 - S)) & (Math.pow(2, S) - 1)), B);
}
function R2(A, B, C, D, X, S, T) {
	q = add(add(A, (B & D) | (C & ~D)), add(X, T));
	return add((q << S) | ((q >> (32 - S)) & (Math.pow(2, S) - 1)), B);
}
function R3(A, B, C, D, X, S, T) {
	q = add(add(A, B ^ C ^ D), add(X, T));
	return add((q << S) | ((q >> (32 - S)) & (Math.pow(2, S) - 1)), B);
}
function R4(A, B, C, D, X, S, T) {
	q = add(add(A, C ^ (B | ~D)), add(X, T));
	return add((q << S) | ((q >> (32 - S)) & (Math.pow(2, S) - 1)), B);
}
/* main entry point */
function calcMD5(sInp) {
	/* Calculate length in machine words, including padding */
	wLen = (((sInp.length + 8) >> 6) + 1) << 4;
	var X = new Array(wLen);

	/* Convert string to array of words */
	j = 4;
	for (i = 0; (i * 4) < sInp.length; i++) {
			X[i] = 0;
			for (j = 0; (j < 4) && ((j + i * 4) < sInp.length); j++) {
					X[i] += (sAscii.indexOf(sInp.charAt((i * 4) + j)) + 32) << (j * 8);
			}
	}
	/* Append padding bits and length */
	if (j == 4) {
			X[i++] = 0x80;
	}else {
			X[i - 1] += 0x80 << (j * 8);
	}
	for(; i < wLen; i++) {
			X[i] = 0;
	}
	X[wLen - 2] = sInp.length * 8;

	/* hard-coded initial values */
	a = 0x67452301;
	b = 0xefcdab89;
	c = 0x98badcfe;
	d = 0x10325476;

	/* Process each 16-word block in turn */
	for (i = 0; i < wLen; i += 16) {
			aO = a;
			bO = b;
			cO = c;
			dO = d;

			a = R1(a, b, c, d, X[i+ 0], 7 , 0xd76aa478); d = R1(d, a, b, c, X[i+ 1], 12, 0xe8c7b756); c = R1(c, d, a, b, X[i+ 2], 17, 0x242070db); b = R1(b, c, d, a, X[i+ 3], 22, 0xc1bdceee); a = R1(a, b, c, d, X[i+ 4], 7 , 0xf57c0faf); d = R1(d, a, b, c, X[i+ 5], 12, 0x4787c62a); c = R1(c, d, a, b, X[i+ 6], 17, 0xa8304613); b = R1(b, c, d, a, X[i+ 7], 22, 0xfd469501); a = R1(a, b, c, d, X[i+ 8], 7 , 0x698098d8); d = R1(d, a, b, c, X[i+ 9], 12, 0x8b44f7af); c = R1(c, d, a, b, X[i+10], 17, 0xffff5bb1); b = R1(b, c, d, a, X[i+11], 22, 0x895cd7be); a = R1(a, b, c, d, X[i+12], 7 , 0x6b901122); d = R1(d, a, b, c, X[i+13], 12, 0xfd987193); c = R1(c, d, a, b, X[i+14], 17, 0xa679438e); b = R1(b, c, d, a, X[i+15], 22, 0x49b40821);

			a = R2(a, b, c, d, X[i+ 1], 5 , 0xf61e2562); d = R2(d, a, b, c, X[i+ 6], 9 , 0xc040b340); c = R2(c, d, a, b, X[i+11], 14, 0x265e5a51); b = R2(b, c, d, a, X[i+ 0], 20, 0xe9b6c7aa); a = R2(a, b, c, d, X[i+ 5], 5 , 0xd62f105d); d = R2(d, a, b, c, X[i+10], 9 , 0x2441453); c = R2(c, d, a, b, X[i+15], 14, 0xd8a1e681); b = R2(b, c, d, a, X[i+ 4], 20, 0xe7d3fbc8); a = R2(a, b, c, d, X[i+ 9], 5 , 0x21e1cde6); d = R2(d, a, b, c, X[i+14], 9 , 0xc33707d6); c = R2(c, d, a, b, X[i+ 3], 14, 0xf4d50d87); b = R2(b, c, d, a, X[i+ 8], 20, 0x455a14ed); a = R2(a, b, c, d, X[i+13], 5 , 0xa9e3e905); d = R2(d, a, b, c, X[i+ 2], 9 , 0xfcefa3f8); c = R2(c, d, a, b, X[i+ 7], 14, 0x676f02d9); b = R2(b, c, d, a, X[i+12], 20, 0x8d2a4c8a);

			a = R3(a, b, c, d, X[i+ 5], 4 , 0xfffa3942); d = R3(d, a, b, c, X[i+ 8], 11, 0x8771f681); c = R3(c, d, a, b, X[i+11], 16, 0x6d9d6122); b = R3(b, c, d, a, X[i+14], 23, 0xfde5380c); a = R3(a, b, c, d, X[i+ 1], 4 , 0xa4beea44); d = R3(d, a, b, c, X[i+ 4], 11, 0x4bdecfa9); c = R3(c, d, a, b, X[i+ 7], 16, 0xf6bb4b60); b = R3(b, c, d, a, X[i+10], 23, 0xbebfbc70); a = R3(a, b, c, d, X[i+13], 4 , 0x289b7ec6); d = R3(d, a, b, c, X[i+ 0], 11, 0xeaa127fa); c = R3(c, d, a, b, X[i+ 3], 16, 0xd4ef3085); b = R3(b, c, d, a, X[i+ 6], 23, 0x4881d05); a = R3(a, b, c, d, X[i+ 9], 4 , 0xd9d4d039); d = R3(d, a, b, c, X[i+12], 11, 0xe6db99e5); c = R3(c, d, a, b, X[i+15], 16, 0x1fa27cf8); b = R3(b, c, d, a, X[i+ 2], 23, 0xc4ac5665);

			a = R4(a, b, c, d, X[i+ 0], 6 , 0xf4292244); d = R4(d, a, b, c, X[i+ 7], 10, 0x432aff97); c = R4(c, d, a, b, X[i+14], 15, 0xab9423a7); b = R4(b, c, d, a, X[i+ 5], 21, 0xfc93a039); a = R4(a, b, c, d, X[i+12], 6 , 0x655b59c3); d = R4(d, a, b, c, X[i+ 3], 10, 0x8f0ccc92); c = R4(c, d, a, b, X[i+10], 15, 0xffeff47d); b = R4(b, c, d, a, X[i+ 1], 21, 0x85845dd1); a = R4(a, b, c, d, X[i+ 8], 6 , 0x6fa87e4f); d = R4(d, a, b, c, X[i+15], 10, 0xfe2ce6e0); c = R4(c, d, a, b, X[i+ 6], 15, 0xa3014314); b = R4(b, c, d, a, X[i+13], 21, 0x4e0811a1); a = R4(a, b, c, d, X[i+ 4], 6 , 0xf7537e82); d = R4(d, a, b, c, X[i+11], 10, 0xbd3af235); c = R4(c, d, a, b, X[i+ 2], 15, 0x2ad7d2bb); b = R4(b, c, d, a, X[i+ 9], 21, 0xeb86d391);

			a = add(a, aO);
			b = add(b, bO);
			c = add(c, cO);
			d = add(d, dO);
	}

	return hex(a) + hex(b) + hex(c) + hex(d);
}
/* A JavaScript implementation of the SHA family of hashes, as defined in FIPS
 * PUB 180-2 as well as the corresponding HMAC implementation as defined in
 * FIPS PUB 198a
 *
 * Version 1.3 Copyright Brian Turek 2008-2010
 * Distributed under the BSD License
 * See http://jssha.sourceforge.net/ for more information
 *
 * Several functions taken from Paul Johnson
 */
(function(){var charSize=8,b64pad="",hexCase=0,Int_64=function(a,b){this.highOrder=a;this.lowOrder=b},str2binb=function(a){var b=[],mask=(1<<charSize)-1,length=a.length*charSize,i;for(i=0;i<length;i+=charSize){b[i>>5]|=(a.charCodeAt(i/charSize)&mask)<<(32-charSize-(i%32))}return b},hex2binb=function(a){var b=[],length=a.length,i,num;for(i=0;i<length;i+=2){num=parseInt(a.substr(i,2),16);if(!isNaN(num)){b[i>>3]|=num<<(24-(4*(i%8)))}else{return"INVALID HEX STRING"}}return b},binb2hex=function(a){var b=(hexCase)?"0123456789ABCDEF":"0123456789abcdef",str="",length=a.length*4,i,srcByte;for(i=0;i<length;i+=1){srcByte=a[i>>2]>>((3-(i%4))*8);str+=b.charAt((srcByte>>4)&0xF)+b.charAt(srcByte&0xF)}return str},binb2b64=function(a){var b="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"+"0123456789+/",str="",length=a.length*4,i,j,triplet;for(i=0;i<length;i+=3){triplet=(((a[i>>2]>>8*(3-i%4))&0xFF)<<16)|(((a[i+1>>2]>>8*(3-(i+1)%4))&0xFF)<<8)|((a[i+2>>2]>>8*(3-(i+2)%4))&0xFF);for(j=0;j<4;j+=1){if(i*8+j*6<=a.length*32){str+=b.charAt((triplet>>6*(3-j))&0x3F)}else{str+=b64pad}}}return str},rotr=function(x,n){if(n<=32){return new Int_64((x.highOrder>>>n)|(x.lowOrder<<(32-n)),(x.lowOrder>>>n)|(x.highOrder<<(32-n)))}else{return new Int_64((x.lowOrder>>>n)|(x.highOrder<<(32-n)),(x.highOrder>>>n)|(x.lowOrder<<(32-n)))}},shr=function(x,n){if(n<=32){return new Int_64(x.highOrder>>>n,x.lowOrder>>>n|(x.highOrder<<(32-n)))}else{return new Int_64(0,x.highOrder<<(32-n))}},ch=function(x,y,z){return new Int_64((x.highOrder&y.highOrder)^(~x.highOrder&z.highOrder),(x.lowOrder&y.lowOrder)^(~x.lowOrder&z.lowOrder))},maj=function(x,y,z){return new Int_64((x.highOrder&y.highOrder)^(x.highOrder&z.highOrder)^(y.highOrder&z.highOrder),(x.lowOrder&y.lowOrder)^(x.lowOrder&z.lowOrder)^(y.lowOrder&z.lowOrder))},sigma0=function(x){var a=rotr(x,28),rotr34=rotr(x,34),rotr39=rotr(x,39);return new Int_64(a.highOrder^rotr34.highOrder^rotr39.highOrder,a.lowOrder^rotr34.lowOrder^rotr39.lowOrder)},sigma1=function(x){var a=rotr(x,14),rotr18=rotr(x,18),rotr41=rotr(x,41);return new Int_64(a.highOrder^rotr18.highOrder^rotr41.highOrder,a.lowOrder^rotr18.lowOrder^rotr41.lowOrder)},gamma0=function(x){var a=rotr(x,1),rotr8=rotr(x,8),shr7=shr(x,7);return new Int_64(a.highOrder^rotr8.highOrder^shr7.highOrder,a.lowOrder^rotr8.lowOrder^shr7.lowOrder)},gamma1=function(x){var a=rotr(x,19),rotr61=rotr(x,61),shr6=shr(x,6);return new Int_64(a.highOrder^rotr61.highOrder^shr6.highOrder,a.lowOrder^rotr61.lowOrder^shr6.lowOrder)},safeAdd_2=function(x,y){var a,msw,lowOrder,highOrder;a=(x.lowOrder&0xFFFF)+(y.lowOrder&0xFFFF);msw=(x.lowOrder>>>16)+(y.lowOrder>>>16)+(a>>>16);lowOrder=((msw&0xFFFF)<<16)|(a&0xFFFF);a=(x.highOrder&0xFFFF)+(y.highOrder&0xFFFF)+(msw>>>16);msw=(x.highOrder>>>16)+(y.highOrder>>>16)+(a>>>16);highOrder=((msw&0xFFFF)<<16)|(a&0xFFFF);return new Int_64(highOrder,lowOrder)},safeAdd_4=function(a,b,c,d){var e,msw,lowOrder,highOrder;e=(a.lowOrder&0xFFFF)+(b.lowOrder&0xFFFF)+(c.lowOrder&0xFFFF)+(d.lowOrder&0xFFFF);msw=(a.lowOrder>>>16)+(b.lowOrder>>>16)+(c.lowOrder>>>16)+(d.lowOrder>>>16)+(e>>>16);lowOrder=((msw&0xFFFF)<<16)|(e&0xFFFF);e=(a.highOrder&0xFFFF)+(b.highOrder&0xFFFF)+(c.highOrder&0xFFFF)+(d.highOrder&0xFFFF)+(msw>>>16);msw=(a.highOrder>>>16)+(b.highOrder>>>16)+(c.highOrder>>>16)+(d.highOrder>>>16)+(e>>>16);highOrder=((msw&0xFFFF)<<16)|(e&0xFFFF);return new Int_64(highOrder,lowOrder)},safeAdd_5=function(a,b,c,d,e){var f,msw,lowOrder,highOrder;f=(a.lowOrder&0xFFFF)+(b.lowOrder&0xFFFF)+(c.lowOrder&0xFFFF)+(d.lowOrder&0xFFFF)+(e.lowOrder&0xFFFF);msw=(a.lowOrder>>>16)+(b.lowOrder>>>16)+(c.lowOrder>>>16)+(d.lowOrder>>>16)+(e.lowOrder>>>16)+(f>>>16);lowOrder=((msw&0xFFFF)<<16)|(f&0xFFFF);f=(a.highOrder&0xFFFF)+(b.highOrder&0xFFFF)+(c.highOrder&0xFFFF)+(d.highOrder&0xFFFF)+(e.highOrder&0xFFFF)+(msw>>>16);msw=(a.highOrder>>>16)+(b.highOrder>>>16)+(c.highOrder>>>16)+(d.highOrder>>>16)+(e.highOrder>>>16)+(f>>>16);highOrder=((msw&0xFFFF)<<16)|(f&0xFFFF);return new Int_64(highOrder,lowOrder)},coreSHA2=function(j,k,l){var a,b,c,d,e,f,g,h,T1,T2,H,lengthPosition,i,t,K,W=[],appendedMessageLength;if(l==="SHA-384"||l==="SHA-512"){lengthPosition=(((k+128)>>10)<<5)+31;K=[new Int_64(0x428a2f98,0xd728ae22),new Int_64(0x71374491,0x23ef65cd),new Int_64(0xb5c0fbcf,0xec4d3b2f),new Int_64(0xe9b5dba5,0x8189dbbc),new Int_64(0x3956c25b,0xf348b538),new Int_64(0x59f111f1,0xb605d019),new Int_64(0x923f82a4,0xaf194f9b),new Int_64(0xab1c5ed5,0xda6d8118),new Int_64(0xd807aa98,0xa3030242),new Int_64(0x12835b01,0x45706fbe),new Int_64(0x243185be,0x4ee4b28c),new Int_64(0x550c7dc3,0xd5ffb4e2),new Int_64(0x72be5d74,0xf27b896f),new Int_64(0x80deb1fe,0x3b1696b1),new Int_64(0x9bdc06a7,0x25c71235),new Int_64(0xc19bf174,0xcf692694),new Int_64(0xe49b69c1,0x9ef14ad2),new Int_64(0xefbe4786,0x384f25e3),new Int_64(0x0fc19dc6,0x8b8cd5b5),new Int_64(0x240ca1cc,0x77ac9c65),new Int_64(0x2de92c6f,0x592b0275),new Int_64(0x4a7484aa,0x6ea6e483),new Int_64(0x5cb0a9dc,0xbd41fbd4),new Int_64(0x76f988da,0x831153b5),new Int_64(0x983e5152,0xee66dfab),new Int_64(0xa831c66d,0x2db43210),new Int_64(0xb00327c8,0x98fb213f),new Int_64(0xbf597fc7,0xbeef0ee4),new Int_64(0xc6e00bf3,0x3da88fc2),new Int_64(0xd5a79147,0x930aa725),new Int_64(0x06ca6351,0xe003826f),new Int_64(0x14292967,0x0a0e6e70),new Int_64(0x27b70a85,0x46d22ffc),new Int_64(0x2e1b2138,0x5c26c926),new Int_64(0x4d2c6dfc,0x5ac42aed),new Int_64(0x53380d13,0x9d95b3df),new Int_64(0x650a7354,0x8baf63de),new Int_64(0x766a0abb,0x3c77b2a8),new Int_64(0x81c2c92e,0x47edaee6),new Int_64(0x92722c85,0x1482353b),new Int_64(0xa2bfe8a1,0x4cf10364),new Int_64(0xa81a664b,0xbc423001),new Int_64(0xc24b8b70,0xd0f89791),new Int_64(0xc76c51a3,0x0654be30),new Int_64(0xd192e819,0xd6ef5218),new Int_64(0xd6990624,0x5565a910),new Int_64(0xf40e3585,0x5771202a),new Int_64(0x106aa070,0x32bbd1b8),new Int_64(0x19a4c116,0xb8d2d0c8),new Int_64(0x1e376c08,0x5141ab53),new Int_64(0x2748774c,0xdf8eeb99),new Int_64(0x34b0bcb5,0xe19b48a8),new Int_64(0x391c0cb3,0xc5c95a63),new Int_64(0x4ed8aa4a,0xe3418acb),new Int_64(0x5b9cca4f,0x7763e373),new Int_64(0x682e6ff3,0xd6b2b8a3),new Int_64(0x748f82ee,0x5defb2fc),new Int_64(0x78a5636f,0x43172f60),new Int_64(0x84c87814,0xa1f0ab72),new Int_64(0x8cc70208,0x1a6439ec),new Int_64(0x90befffa,0x23631e28),new Int_64(0xa4506ceb,0xde82bde9),new Int_64(0xbef9a3f7,0xb2c67915),new Int_64(0xc67178f2,0xe372532b),new Int_64(0xca273ece,0xea26619c),new Int_64(0xd186b8c7,0x21c0c207),new Int_64(0xeada7dd6,0xcde0eb1e),new Int_64(0xf57d4f7f,0xee6ed178),new Int_64(0x06f067aa,0x72176fba),new Int_64(0x0a637dc5,0xa2c898a6),new Int_64(0x113f9804,0xbef90dae),new Int_64(0x1b710b35,0x131c471b),new Int_64(0x28db77f5,0x23047d84),new Int_64(0x32caab7b,0x40c72493),new Int_64(0x3c9ebe0a,0x15c9bebc),new Int_64(0x431d67c4,0x9c100d4c),new Int_64(0x4cc5d4be,0xcb3e42b6),new Int_64(0x597f299c,0xfc657e2a),new Int_64(0x5fcb6fab,0x3ad6faec),new Int_64(0x6c44198c,0x4a475817)];if(l==="SHA-384"){H=[new Int_64(0xcbbb9d5d,0xc1059ed8),new Int_64(0x0629a292a,0x367cd507),new Int_64(0x9159015a,0x3070dd17),new Int_64(0x0152fecd8,0xf70e5939),new Int_64(0x67332667,0xffc00b31),new Int_64(0x98eb44a87,0x68581511),new Int_64(0xdb0c2e0d,0x64f98fa7),new Int_64(0x047b5481d,0xbefa4fa4)]}else{H=[new Int_64(0x6a09e667,0xf3bcc908),new Int_64(0xbb67ae85,0x84caa73b),new Int_64(0x3c6ef372,0xfe94f82b),new Int_64(0xa54ff53a,0x5f1d36f1),new Int_64(0x510e527f,0xade682d1),new Int_64(0x9b05688c,0x2b3e6c1f),new Int_64(0x1f83d9ab,0xfb41bd6b),new Int_64(0x5be0cd19,0x137e2179)]}}j[k>>5]|=0x80<<(24-k%32);j[lengthPosition]=k;appendedMessageLength=j.length;for(i=0;i<appendedMessageLength;i+=32){a=H[0];b=H[1];c=H[2];d=H[3];e=H[4];f=H[5];g=H[6];h=H[7];for(t=0;t<80;t+=1){if(t<16){W[t]=new Int_64(j[t*2+i],j[t*2+i+1])}else{W[t]=safeAdd_4(gamma1(W[t-2]),W[t-7],gamma0(W[t-15]),W[t-16])}T1=safeAdd_5(h,sigma1(e),ch(e,f,g),K[t],W[t]);T2=safeAdd_2(sigma0(a),maj(a,b,c));h=g;g=f;f=e;e=safeAdd_2(d,T1);d=c;c=b;b=a;a=safeAdd_2(T1,T2)}H[0]=safeAdd_2(a,H[0]);H[1]=safeAdd_2(b,H[1]);H[2]=safeAdd_2(c,H[2]);H[3]=safeAdd_2(d,H[3]);H[4]=safeAdd_2(e,H[4]);H[5]=safeAdd_2(f,H[5]);H[6]=safeAdd_2(g,H[6]);H[7]=safeAdd_2(h,H[7])}switch(l){case"SHA-384":return[H[0].highOrder,H[0].lowOrder,H[1].highOrder,H[1].lowOrder,H[2].highOrder,H[2].lowOrder,H[3].highOrder,H[3].lowOrder,H[4].highOrder,H[4].lowOrder,H[5].highOrder,H[5].lowOrder];case"SHA-512":return[H[0].highOrder,H[0].lowOrder,H[1].highOrder,H[1].lowOrder,H[2].highOrder,H[2].lowOrder,H[3].highOrder,H[3].lowOrder,H[4].highOrder,H[4].lowOrder,H[5].highOrder,H[5].lowOrder,H[6].highOrder,H[6].lowOrder,H[7].highOrder,H[7].lowOrder];default:return[]}},jsSHA=function(a,b){this.sha384=null;this.sha512=null;this.strBinLen=null;this.strToHash=null;if("HEX"===b){if(0!==(a.length%2)){return"TEXT MUST BE IN BYTE INCREMENTS"}this.strBinLen=a.length*4;this.strToHash=hex2binb(a)}else if(("ASCII"===b)||('undefined'===typeof(b))){this.strBinLen=a.length*charSize;this.strToHash=str2binb(a)}else{return"UNKNOWN TEXT INPUT TYPE"}};jsSHA.prototype={getHash:function(a,b){var c=null,message=this.strToHash.slice();switch(b){case"HEX":c=binb2hex;break;case"B64":c=binb2b64;break;default:return"FORMAT NOT RECOGNIZED"}switch(a){case"SHA-384":if(null===this.sha384){this.sha384=coreSHA2(message,this.strBinLen,a)}return c(this.sha384);case"SHA-512":if(null===this.sha512){this.sha512=coreSHA2(message,this.strBinLen,a)}return c(this.sha512);default:return"HASH NOT RECOGNIZED"}},getHMAC:function(a,b,c,d){var e,keyToUse,i,retVal,keyBinLen,hashBitSize,keyWithIPad=[],keyWithOPad=[];switch(d){case"HEX":e=binb2hex;break;case"B64":e=binb2b64;break;default:return"FORMAT NOT RECOGNIZED"}switch(c){case"SHA-384":hashBitSize=384;break;case"SHA-512":hashBitSize=512;break;default:return"HASH NOT RECOGNIZED"}if("HEX"===b){if(0!==(a.length%2)){return"KEY MUST BE IN BYTE INCREMENTS"}keyToUse=hex2binb(a);keyBinLen=a.length*4}else if("ASCII"===b){keyToUse=str2binb(a);keyBinLen=a.length*charSize}else{return"UNKNOWN KEY INPUT TYPE"}if(128<(keyBinLen/8)){keyToUse=coreSHA2(keyToUse,keyBinLen,c);keyToUse[31]&=0xFFFFFF00}else if(128>(keyBinLen/8)){keyToUse[31]&=0xFFFFFF00}for(i=0;i<=31;i+=1){keyWithIPad[i]=keyToUse[i]^0x36363636;keyWithOPad[i]=keyToUse[i]^0x5C5C5C5C}retVal=coreSHA2(keyWithIPad.concat(this.strToHash),1024+this.strBinLen,c);retVal=coreSHA2(keyWithOPad.concat(retVal),1024+hashBitSize,c);return(e(retVal))}};window.jsSHA=jsSHA}());

ArcAES = {
	//aes:null,
    cipher:null,
	mode:'CBC',
	key:null,
	iv:null,
	init: function(){
		this.key=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
		this.iv =[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
		this.cipher=null;
		this.aes=null;
	},
	create:function(mode, key, iv){
		var aes= new ArcAES.AES(key);
		if (mode == 'ECB'){
			this.cipher=new ArcAES.ECBMode(aes);
		}else if (mode == 'CBC') {
			this.cipher=new ArcAES.CBCMode(aes, iv);
		}
		this.cipher.mode=mode;
		//this.cipher.setMode=ArcAES.setMode;
		this.cipher.AES=ArcAES;

		return this.cipher;
	},
	setMode: function(mode, key, iv){
		this.AES.cipher=null;

		this.AES.cipher=new this.AES.create(mode,key, iv);
		//}}
		return this.AES.cipher;
	},
	IntToByteArray: function(iS,wd) {
	  var bA = [];
      var i;
      var sz=iS.length;

      for(i = 0; i < sz; i++) {
        bA[i] = iS.charCodeAt(i);
      }
      // padding
      if(!wd)
      	wd=( Math.ceil((sz)/16))*16;
      for(;i < wd;i++){bA[i] =0;}
      return bA;
    },
	hexToByteArray: function(hS){
		var bA=[];
		// if %xx format truncate it
		if(hS.indexOf("%")==0)
			hS=hS.replace(/%/g,"");
		if(hS.length%2)return;
		if(hS.indexOf("0x")==0||hS.indexOf("0X")==0)
			hS = hS.substring(2);
		for (var i=0;i<hS.length;i+=2)
			bA[Math.floor(i/2)]=parseInt(hS.slice(i,i+2),16);
		return bA;
	},
	byteArrayToString: function(byteArray) {
		  var result = "";
		  for(var i=0; i<byteArray.length; i++)
			if (byteArray[i] != 0)
			  result += String.fromCharCode(byteArray[i]);
		  return result;
	},
	byteArrayToHex: function(byteArray) {
		  var result = "";
		  if (!byteArray)
			return;
		  for (var i=0; i<byteArray.length; i++)
			result += ((byteArray[i]<16) ? "0" : "") + byteArray[i].toString(16);

		  return result;
	},
	byteArrayToHTML: function(byteArray) {
		  var result = "";
		  if (!byteArray)
			return;
		  for (var i=0; i<byteArray.length; i++)
			result += "%"+((byteArray[i]<16) ? "0" : "") + byteArray[i].toString(16);
		  return result;
	},
	// AES cipher class
	AES : function AES(key) {
		this.blockSize=16,
		// Sets the key and performs key expansion
		this.setKey = function setKey(key) {
			this.key = key;
			this.keySize = key.length;

			if (this.keySize == 16)
				this.rounds=10;
			else if (this.keySize == 24)
				this.rounds=12;
			else if (this.keySize == 32)
				this.rounds=14;
			else
				throw new Error("Key length must be 16, 24 or 32 bytes");

			this.expandKey();
		},
		this.expandKey = function expandKey() {
		    var exKey;
		    var j, z;

		    // The expanded key starts with the actual key itself; copy it
		    exKey = this.key.slice();

		    // extra key expansion steps
		    var extraCount;
		    if (this.keySize == 16)
		        extraCount = 0;
		    else if (this.keySize == 24)
		        extraCount = 2;
		    else
		        extraCount = 3;

		    // 4-byte temporary variable for key expansion
		    var word = exKey.slice(-4);

		    // Each expansion cycle uses 'i' once for Rcon table lookup
		    for(var i = 1; i < 11; i++) {
		        //// key schedule core:
		        // left-rotate by 1 byte
		        word = word.slice(1,4).concat(word.slice(0,1));

		        // apply S-box to all bytes
		        for(j = 0; j < 4; j++) {
		            word[j] = ArcAES.sbox[(word[j])];
		        }

		        // apply the Rcon table to the leftmost byte
		        word[0] ^= ArcAES.rcon[i];

		        for(z = 0; z < 4; z++) {
		            for(j = 0; j < 4; j++) {
		                // mix in bytes from the last subkey
		                word[j] ^= exKey[exKey.length - this.keySize + j];
		            }
		            exKey = exKey.concat(word);
		        }

		        // Last key expansion cycle always finishes here
		        if (exKey.length >= ((this.rounds+1) * this.blockSize)) {
		            break;
		        }

		        // Special substitution step for 256-bit key
		        if (this.keySize == 32) {
		            for(j=0;j<4;j++) {
		                // mix in bytes from the last subkey XORed with S-box of
		                // current word bytes
		                word[j] = ArcAES.sbox[word[j]] ^ exKey[exKey.length - this.keySize + j];
		            }
		            exKey = exKey.concat(word);
		        }

		        // Twice for 192-bit key, thrice for 256-bit key
		        for(z=0; z < extraCount; z++) {
		            for(j = 0; j < 4; j++) {
		                // mix in bytes from the last subkey
		                word[j] ^= exKey[exKey.length - this.keySize + j];
		            }
		            exKey = exKey.concat(word);
		        }
		    }
		    this.exKey=exKey;
		},
		// Encrypts a single block. This is the main AES function
		this.encryptBlock = function encryptBlock(block) {
					this.addRoundKey(block, 0);

					// For efficiency reasons, the state between steps is transmitted via a
					// mutable array, not returned
					for(var round = 1; round < this.rounds; round++) {
						this.subBytes(block, ArcAES.sbox);
						this.shiftRows(block);
						this.mixColumns(block);
						this.addRoundKey(block, round);
					}
					this.subBytes(block, ArcAES.sbox);
					this.shiftRows(block);
					// no mix_columns step in the last round
					this.addRoundKey(block, this.rounds);
		},

		// Decrypts a single block. This is the main AES decryption function
		this.decryptBlock = function decryptBlock(block) {
					this.addRoundKey(block, this.rounds);

					// For efficiency reasons, the state between steps is transmitted via a
					// mutable array, not returned
					for(var round = (this.rounds - 1); round > 0; round--) {
						this.shiftRowsInv(block);
						this.subBytes(block, ArcAES.invSbox);
						this.addRoundKey(block, round);
						this.mixColumnsInv(block);
					}
					this.shiftRowsInv(block);
					this.subBytes(block, ArcAES.invSbox);
					this.addRoundKey(block, 0);
					// no mix_columns step in the last round
		},

		// AddRoundKey step in AES. This is where the key is mixed into plaintext
		this.addRoundKey = function addRoundKey(block, round) {
					var offset = round * 16;
					var exKey = this.exKey;

					for(var i = 0; i < 16; i++) {
						block[i] ^= exKey[offset + i];
					}

					//console.log('AddRoundKey: ' + block)
		},

		/* SubBytes step, apply S-box to all bytes
		 *
		 * Depending on whether encrypting or decrypting, a different sbox array
		 * is passed in.
		 */
		 this.subBytes = function subBytes(block, sbox) {
					for(var i = 0; i < 16; i++) {
						block[i] = sbox[block[i]];
					}

					//console.log('SubBytes   : ' + block)
		},

		/* ShiftRows step. Shifts 2nd row to left by 1, 3rd row by 2, 4th row by 3
		 *
		 * Since we're performing this on a transposed matrix, cells are numbered
		 * from top to bottom first:
		 *
		 * 0  4  8 12   ->    0  4  8 12    -- 1st row doesn't change
		 * 1  5  9 13   ->    5  9 13  1    -- row shifted to left by 1 (wraps around)
		 * 2  6 10 14   ->   10 14  2  6    -- shifted by 2
		 * 3  7 11 15   ->   15  3  7 11    -- shifted by 3
		 */
		 this.shiftRows = function shiftRows(b) {
					var tmp;

					// 2nd row
					tmp  = b[1];
					b[1] = b[5];
					b[5] = b[9];
					b[9] = b[13];
					b[13]= tmp;

					// 3rd row
					tmp  = b[2];
					b[2] = b[10];
					b[10]= tmp;
					tmp  = b[6];
					b[6] = b[14];
					b[14]= tmp;

					// 4th row
					tmp  = b[15];
					b[15]= b[11];
					b[11]= b[7];
					b[7] = b[3];
					b[3] = tmp;

					//console.log('ShiftRows  : ' + b)
	     },

		 // Similar to shiftRows above, but performed in inverse for decryption
		 this.shiftRowsInv = function shiftRowsInv(b) {
					var tmp;

					// 2nd row
					tmp  = b[13];
					b[13]= b[9];
					b[9] = b[5];
					b[5] = b[1];
					b[1] = tmp;

					// 3rd row
					tmp  = b[2];
					b[2] = b[10];
					b[10]= tmp;
					tmp  = b[6];
					b[6] = b[14];
					b[14]= tmp;

					// 4th row
					tmp  = b[3];
					b[3] = b[7];
					b[7] = b[11];
					b[11]= b[15];
					b[15]= tmp;

					//console.log('ShiftRows  : ' + b)
		 },

		 // MixColumns step. Mixes the values in each column
		 this.mixColumns = function mixColumns(block) {
					var mulBy2 = ArcAES.gfMulBy2;
					var mulBy3 = ArcAES.gfMulBy3;

					for(var col = 0; col < 16; col += 4) {
						var v0 = block[col  ];
						var v1 = block[col+1];
						var v2 = block[col+2];
						var v3 = block[col+3];

						block[col  ] = mulBy2[v0] ^ v3 ^ v2 ^ mulBy3[v1];
						block[col+1] = mulBy2[v1] ^ v0 ^ v3 ^ mulBy3[v2];
						block[col+2] = mulBy2[v2] ^ v1 ^ v0 ^ mulBy3[v3];
						block[col+3] = mulBy2[v3] ^ v2 ^ v1 ^ mulBy3[v0];
					}

					//console.log('MixColumns : ' + block)
		 },

		 // Similar to mixColumns above, but performed in inverse for decryption.
		 this.mixColumnsInv = function mixColumnsInv(block) {
					var mul9 = ArcAES.gfMulBy9;
					var mul11 = ArcAES.gfMulBy11;
					var mul13 = ArcAES.gfMulBy13;
					var mul14 = ArcAES.gfMulBy14;

					for(var col = 0; col < 16; col += 4) {
						var v0 = block[col  ];
						var v1 = block[col+1];
						var v2 = block[col+2];
						var v3 = block[col+3];

						block[col  ] = mul14[v0] ^ mul9[v3] ^ mul13[v2] ^ mul11[v1];
						block[col+1] = mul14[v1] ^ mul9[v0] ^ mul13[v3] ^ mul11[v2];
						block[col+2] = mul14[v2] ^ mul9[v1] ^ mul13[v0] ^ mul11[v3];
						block[col+3] = mul14[v3] ^ mul9[v2] ^ mul13[v1] ^ mul11[v0];
					}

					//console.log('MixColumns : ' + block)
		 }
		 this.setKey(key);
	},
	/********
	 * Class for Electronic CodeBook (ECB) mode encryption.
	 *
	 * Basically this mode applies the cipher function to each block individually;
	 * no feedback is done. NB! This is insecure for almost all purposes
	 */

	ECBMode : function (cipher) {
	    this.cipher = cipher;
	    this.blockSize = cipher.blockSize;
		this.setkey = function (key){
			this.cipher=null;
			this.cipher=new ArcAES.AES(key);
		},
		// Encrypt data in ECB mode
		this.encrypt = function encrypt(data) {
    		return this.ecb(data, this.cipher.encryptBlock);
		},
		// Decrypt data in ECB mode
		this.decrypt = function decrypt(data) {
		    return this.ecb(data, this.cipher.decryptBlock);
		},
		// Perform ECB mode with the given function
		this.ecb = function ecb(data, blockFunc) {
			var blockSize = this.blockSize;

			if (data.length % blockSize != 0)
				throw new Error("Input length must be multiple of 16");

			var result = new Array;

			for(var offset = 0; offset < data.length; offset += blockSize) {
				var block = data.slice(offset, offset + blockSize);

				blockFunc(block);

				for(var i = 0; i < blockSize; i++) {
					result.push(block[i]);
				}
			}

			return result;
		}
	},
	/********
	 * Cipher Block Chaining (CBC) mode encryption. This mode avoids content leaks.
	 *
	 * In CBC encryption, each plaintext block is XORed with the ciphertext block
	 * preceding it; decryption is simply the inverse.
	 *
	 * A better explanation of CBC can be found here:
	 * http://en.wikipedia.org/wiki/Block_cipher_modes_of_operation#Cipher-block_chaining_.28CBC.29
	 */
	 CBCMode : function CBCMode(cipher, iv) {
		if (!iv)
			throw new Error("CBC mode needs an IV value!");

		if (iv.length != cipher.blockSize)
			throw new Error("IV must be exactly 16 bytes long");

		this.cipher = cipher;
		this.blockSize = cipher.blockSize;
		this.iv = iv;
		this.setkey = function (key,iv){
			this.cipher=null;
			this.cipher=new ArcAES.AES(key);
			this.iv=iv;
		},
		// Encrypt data in CBC mode
		this.encrypt = function encrypt(data) {
			var blockSize = this.blockSize;

			if (data.length % blockSize != 0)
				throw new Error("Input length must be multiple of 16");

			var result = new Array;
			var iv = this.iv;

			for(var offset = 0; offset < data.length; offset += blockSize) {
				var block = data.slice(offset, offset + blockSize);

				// Perform CBC chaining
				for(var i = 0; i < blockSize; i++) {
					block[i] ^= iv[i];
				}
				this.cipher.encryptBlock(block);

				for(var i = 0; i < blockSize; i++) {
					result.push(block[i]);
				}

				iv = block;
			}
			this.iv = iv;

			return result;
		},

		// Decrypt data in CBC mode
		this.decrypt = function decrypt(data) {
			var blockSize = this.blockSize;

			if (data.length % blockSize != 0){
				//throw new Error("Input length must be multiple of 16");
				return data;
			}
			var result = new Array;
			var iv = this.iv;

			for(var offset = 0; offset < data.length; offset += blockSize) {
				var ctext = data.slice(offset, offset + blockSize);

				// copy array, we'll need ctext later
				var block = ctext.slice();
				this.cipher.decryptBlock(block);

				// Perform CBC chaining
				for(var i = 0; i < blockSize; i++) {
					result.push(block[i] ^ iv[i])
				}

				iv = ctext;
			}

			this.iv = iv;
			return result;
		}
	},
	/********
	 * The S-box is a 256-element array, that maps a single byte value to another
	 * byte value. Since it's designed to be reversible, each value occurs only once
	 * in the S-box
	 *
	 * More information: http://en.wikipedia.org/wiki/Rijndael_S-box
	 */
	sbox : [
		 99,124,119,123,242,107,111,197, 48,  1,103, 43,254,215,171,118,
		202,130,201,125,250, 89, 71,240,173,212,162,175,156,164,114,192,
		183,253,147, 38, 54, 63,247,204, 52,165,229,241,113,216, 49, 21,
		  4,199, 35,195, 24,150,  5,154,  7, 18,128,226,235, 39,178,117,
		  9,131, 44, 26, 27,110, 90,160, 82, 59,214,179, 41,227, 47,132,
		 83,209,  0,237, 32,252,177, 91,106,203,190, 57, 74, 76, 88,207,
		208,239,170,251, 67, 77, 51,133, 69,249,  2,127, 80, 60,159,168,
		 81,163, 64,143,146,157, 56,245,188,182,218, 33, 16,255,243,210,
		205, 12, 19,236, 95,151, 68, 23,196,167,126, 61,100, 93, 25,115,
		 96,129, 79,220, 34, 42,144,136, 70,238,184, 20,222, 94, 11,219,
		224, 50, 58, 10, 73,  6, 36, 92,194,211,172, 98,145,149,228,121,
		231,200, 55,109,141,213, 78,169,108, 86,244,234,101,122,174,  8,
		186,120, 37, 46, 28,166,180,198,232,221,116, 31, 75,189,139,138,
		112, 62,181,102, 72,  3,246, 14, 97, 53, 87,185,134,193, 29,158,
		225,248,152, 17,105,217,142,148,155, 30,135,233,206, 85, 40,223,
		140,161,137, 13,191,230, 66,104, 65,153, 45, 15,176, 84,187, 22,
	],

	/* This is the inverse of the above. In other words:
	 * invSbox[sbox[val]] == val
	 */
	invSbox : [
		 82,  9,106,213, 48, 54,165, 56,191, 64,163,158,129,243,215,251,
		124,227, 57,130,155, 47,255,135, 52,142, 67, 68,196,222,233,203,
		 84,123,148, 50,166,194, 35, 61,238, 76,149, 11, 66,250,195, 78,
		  8, 46,161,102, 40,217, 36,178,118, 91,162, 73,109,139,209, 37,
		114,248,246,100,134,104,152, 22,212,164, 92,204, 93,101,182,146,
		108,112, 72, 80,253,237,185,218, 94, 21, 70, 87,167,141,157,132,
		144,216,171,  0,140,188,211, 10,247,228, 88,  5,184,179, 69,  6,
		208, 44, 30,143,202, 63, 15,  2,193,175,189,  3,  1, 19,138,107,
		 58,145, 17, 65, 79,103,220,234,151,242,207,206,240,180,230,115,
		150,172,116, 34,231,173, 53,133,226,249, 55,232, 28,117,223,110,
		 71,241, 26,113, 29, 41,197,137,111,183, 98, 14,170, 24,190, 27,
		252, 86, 62, 75,198,210,121, 32,154,219,192,254,120,205, 90,244,
		 31,221,168, 51,136,  7,199, 49,177, 18, 16, 89, 39,128,236, 95,
		 96, 81,127,169, 25,181, 74, 13, 45,229,122,159,147,201,156,239,
		160,224, 59, 77,174, 42,245,176,200,235,187, 60,131, 83,153, 97,
		 23, 43,  4,126,186,119,214, 38,225,105, 20, 99, 85, 33, 12,125,
	],

	/* The Rcon table is used in AES's key schedule (key expansion)
	 * It's a pre-computed table of exponentation of 2 in AES's finite field
	 *
	 * More information: http://en.wikipedia.org/wiki/Rijndael_key_schedule
	 */
	rcon : [
		141,  1,  2,  4,  8, 16, 32, 64,128, 27, 54,108,216,171, 77,154,
		 47, 94,188, 99,198,151, 53,106,212,179,125,250,239,197,145, 57,
		114,228,211,189, 97,194,159, 37, 74,148, 51,102,204,131, 29, 58,
		116,232,203,141,  1,  2,  4,  8, 16, 32, 64,128, 27, 54,108,216,
		171, 77,154, 47, 94,188, 99,198,151, 53,106,212,179,125,250,239,
		197,145, 57,114,228,211,189, 97,194,159, 37, 74,148, 51,102,204,
		131, 29, 58,116,232,203,141,  1,  2,  4,  8, 16, 32, 64,128, 27,
		 54,108,216,171, 77,154, 47, 94,188, 99,198,151, 53,106,212,179,
		125,250,239,197,145, 57,114,228,211,189, 97,194,159, 37, 74,148,
		 51,102,204,131, 29, 58,116,232,203,141,  1,  2,  4,  8, 16, 32,
		 64,128, 27, 54,108,216,171, 77,154, 47, 94,188, 99,198,151, 53,
		106,212,179,125,250,239,197,145, 57,114,228,211,189, 97,194,159,
		 37, 74,148, 51,102,204,131, 29, 58,116,232,203,141,  1,  2,  4,
		  8, 16, 32, 64,128, 27, 54,108,216,171, 77,154, 47, 94,188, 99,
		198,151, 53,106,212,179,125,250,239,197,145, 57,114,228,211,189,
		 97,194,159, 37, 74,148, 51,102,204,131, 29, 58,116,232,203,
	],

	// Lookup table for AES Galois Field multiplication by 2
	gfMulBy2 : [
		  0,  2,  4,  6,  8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30,
		 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62,
		 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 94,
		 96, 98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
		128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,
		160,162,164,166,168,170,172,174,176,178,180,182,184,186,188,190,
		192,194,196,198,200,202,204,206,208,210,212,214,216,218,220,222,
		224,226,228,230,232,234,236,238,240,242,244,246,248,250,252,254,
		 27, 25, 31, 29, 19, 17, 23, 21, 11,  9, 15, 13,  3,  1,  7,  5,
		 59, 57, 63, 61, 51, 49, 55, 53, 43, 41, 47, 45, 35, 33, 39, 37,
		 91, 89, 95, 93, 83, 81, 87, 85, 75, 73, 79, 77, 67, 65, 71, 69,
		123,121,127,125,115,113,119,117,107,105,111,109, 99, 97,103,101,
		155,153,159,157,147,145,151,149,139,137,143,141,131,129,135,133,
		187,185,191,189,179,177,183,181,171,169,175,173,163,161,167,165,
		219,217,223,221,211,209,215,213,203,201,207,205,195,193,199,197,
		251,249,255,253,243,241,247,245,235,233,239,237,227,225,231,229,
	],

	// GF multiplication by 3
	gfMulBy3 : [
		  0,  3,  6,  5, 12, 15, 10,  9, 24, 27, 30, 29, 20, 23, 18, 17,
		 48, 51, 54, 53, 60, 63, 58, 57, 40, 43, 46, 45, 36, 39, 34, 33,
		 96, 99,102,101,108,111,106,105,120,123,126,125,116,119,114,113,
		 80, 83, 86, 85, 92, 95, 90, 89, 72, 75, 78, 77, 68, 71, 66, 65,
		192,195,198,197,204,207,202,201,216,219,222,221,212,215,210,209,
		240,243,246,245,252,255,250,249,232,235,238,237,228,231,226,225,
		160,163,166,165,172,175,170,169,184,187,190,189,180,183,178,177,
		144,147,150,149,156,159,154,153,136,139,142,141,132,135,130,129,
		155,152,157,158,151,148,145,146,131,128,133,134,143,140,137,138,
		171,168,173,174,167,164,161,162,179,176,181,182,191,188,185,186,
		251,248,253,254,247,244,241,242,227,224,229,230,239,236,233,234,
		203,200,205,206,199,196,193,194,211,208,213,214,223,220,217,218,
		 91, 88, 93, 94, 87, 84, 81, 82, 67, 64, 69, 70, 79, 76, 73, 74,
		107,104,109,110,103,100, 97, 98,115,112,117,118,127,124,121,122,
		 59, 56, 61, 62, 55, 52, 49, 50, 35, 32, 37, 38, 47, 44, 41, 42,
		 11,  8, 13, 14,  7,  4,  1,  2, 19, 16, 21, 22, 31, 28, 25, 26,
	],

	// GF multiplication by 9
	gfMulBy9 : [
		  0,  9, 18, 27, 36, 45, 54, 63, 72, 65, 90, 83,108,101,126,119,
		144,153,130,139,180,189,166,175,216,209,202,195,252,245,238,231,
		 59, 50, 41, 32, 31, 22, 13,  4,115,122, 97,104, 87, 94, 69, 76,
		171,162,185,176,143,134,157,148,227,234,241,248,199,206,213,220,
		118,127,100,109, 82, 91, 64, 73, 62, 55, 44, 37, 26, 19,  8,  1,
		230,239,244,253,194,203,208,217,174,167,188,181,138,131,152,145,
		 77, 68, 95, 86,105, 96,123,114,  5, 12, 23, 30, 33, 40, 51, 58,
		221,212,207,198,249,240,235,226,149,156,135,142,177,184,163,170,
		236,229,254,247,200,193,218,211,164,173,182,191,128,137,146,155,
		124,117,110,103, 88, 81, 74, 67, 52, 61, 38, 47, 16, 25,  2, 11,
		215,222,197,204,243,250,225,232,159,150,141,132,187,178,169,160,
		 71, 78, 85, 92, 99,106,113,120, 15,  6, 29, 20, 43, 34, 57, 48,
		154,147,136,129,190,183,172,165,210,219,192,201,246,255,228,237,
		 10,  3, 24, 17, 46, 39, 60, 53, 66, 75, 80, 89,102,111,116,125,
		161,168,179,186,133,140,151,158,233,224,251,242,205,196,223,214,
		 49, 56, 35, 42, 21, 28,  7, 14,121,112,107, 98, 93, 84, 79, 70,
	],

	// GF multiplication by 11
	gfMulBy11 : [
		  0, 11, 22, 29, 44, 39, 58, 49, 88, 83, 78, 69,116,127, 98,105,
		176,187,166,173,156,151,138,129,232,227,254,245,196,207,210,217,
		123,112,109,102, 87, 92, 65, 74, 35, 40, 53, 62, 15,  4, 25, 18,
		203,192,221,214,231,236,241,250,147,152,133,142,191,180,169,162,
		246,253,224,235,218,209,204,199,174,165,184,179,130,137,148,159,
		 70, 77, 80, 91,106, 97,124,119, 30, 21,  8,  3, 50, 57, 36, 47,
		141,134,155,144,161,170,183,188,213,222,195,200,249,242,239,228,
		 61, 54, 43, 32, 17, 26,  7, 12,101,110,115,120, 73, 66, 95, 84,
		247,252,225,234,219,208,205,198,175,164,185,178,131,136,149,158,
		 71, 76, 81, 90,107, 96,125,118, 31, 20,  9,  2, 51, 56, 37, 46,
		140,135,154,145,160,171,182,189,212,223,194,201,248,243,238,229,
		 60, 55, 42, 33, 16, 27,  6, 13,100,111,114,121, 72, 67, 94, 85,
		  1, 10, 23, 28, 45, 38, 59, 48, 89, 82, 79, 68,117,126, 99,104,
		177,186,167,172,157,150,139,128,233,226,255,244,197,206,211,216,
		122,113,108,103, 86, 93, 64, 75, 34, 41, 52, 63, 14,  5, 24, 19,
		202,193,220,215,230,237,240,251,146,153,132,143,190,181,168,163,
	],

	// GF multiplication by 13
	gfMulBy13 : [
		  0, 13, 26, 23, 52, 57, 46, 35,104,101,114,127, 92, 81, 70, 75,
		208,221,202,199,228,233,254,243,184,181,162,175,140,129,150,155,
		187,182,161,172,143,130,149,152,211,222,201,196,231,234,253,240,
		107,102,113,124, 95, 82, 69, 72,  3, 14, 25, 20, 55, 58, 45, 32,
		109, 96,119,122, 89, 84, 67, 78,  5,  8, 31, 18, 49, 60, 43, 38,
		189,176,167,170,137,132,147,158,213,216,207,194,225,236,251,246,
		214,219,204,193,226,239,248,245,190,179,164,169,138,135,144,157,
		  6, 11, 28, 17, 50, 63, 40, 37,110, 99,116,121, 90, 87, 64, 77,
		218,215,192,205,238,227,244,249,178,191,168,165,134,139,156,145,
		 10,  7, 16, 29, 62, 51, 36, 41, 98,111,120,117, 86, 91, 76, 65,
		 97,108,123,118, 85, 88, 79, 66,  9,  4, 19, 30, 61, 48, 39, 42,
		177,188,171,166,133,136,159,146,217,212,195,206,237,224,247,250,
		183,186,173,160,131,142,153,148,223,210,197,200,235,230,241,252,
		103,106,125,112, 83, 94, 73, 68, 15,  2, 21, 24, 59, 54, 33, 44,
		 12,  1, 22, 27, 56, 53, 34, 47,100,105,126,115, 80, 93, 74, 71,
		220,209,198,203,232,229,242,255,180,185,174,163,128,141,154,151,
	],

	// GF multiplication by 14
	gfMulBy14 : [
		  0, 14, 28, 18, 56, 54, 36, 42,112,126,108, 98, 72, 70, 84, 90,
		224,238,252,242,216,214,196,202,144,158,140,130,168,166,180,186,
		219,213,199,201,227,237,255,241,171,165,183,185,147,157,143,129,
		 59, 53, 39, 41,  3, 13, 31, 17, 75, 69, 87, 89,115,125,111, 97,
		173,163,177,191,149,155,137,135,221,211,193,207,229,235,249,247,
		 77, 67, 81, 95,117,123,105,103, 61, 51, 33, 47,  5, 11, 25, 23,
		118,120,106,100, 78, 64, 82, 92,  6,  8, 26, 20, 62, 48, 34, 44,
		150,152,138,132,174,160,178,188,230,232,250,244,222,208,194,204,
		 65, 79, 93, 83,121,119,101,107, 49, 63, 45, 35,  9,  7, 21, 27,
		161,175,189,179,153,151,133,139,209,223,205,195,233,231,245,251,
		154,148,134,136,162,172,190,176,234,228,246,248,210,220,206,192,
		122,116,102,104, 66, 76, 94, 80, 10,  4, 22, 24, 50, 60, 46, 32,
		236,226,240,254,212,218,200,198,156,146,128,142,164,170,184,182,
		 12,  2, 16, 30, 52, 58, 40, 38,124,114, 96,110, 68, 74, 88, 86,
		 55, 57, 43, 37, 15,  1, 19, 29, 71, 73, 91, 85,127,113, 99,109,
		215,217,203,197,239,225,243,253,167,169,187,181,159,145,131,141,
	]
};
var g_AES_Mode=null;
function do_decode(_s){
	var iv     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
	var k=ArcBase._t();
	var ct=ArcAES.hexToByteArray(_s);
	if(!ct || isNaN(ct[0]) || (ct.length%16!=0) ) return _s;

	var key=ArcAES.IntToByteArray(k,32);
	if(!g_AES_Mode){
		g_AES_Mode=new ArcAES.create('CBC', key, iv);
	}else{
		g_AES_Mode.setkey(key,iv);
	}
	var pt = g_AES_Mode.decrypt(ct);
	return ArcAES.byteArrayToString(pt);
}
function do_encode(_s){
	var iv     = [0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0];
	var k=ArcBase._t()
	var key=ArcAES.IntToByteArray(k,32);
	var ct =ArcAES.IntToByteArray(_s);
	if(!ct || isNaN(ct[0])) return _s;
	if(!g_AES_Mode){
		g_AES_Mode=ArcAES.create('CBC', key, iv);
	}else{
		g_AES_Mode.setkey(key, iv);
	}
	var pt = g_AES_Mode.encrypt(ct);
	return ArcAES.byteArrayToHTML(pt);
}
function Arcdecode(_idx){
	var _s=CA[_idx].v;
	// if no necessary decode, skip it
	if(CA[_idx].e == null || CA[_idx].e==0) return _s;

	_s=CA[_idx].v=do_decode(_s);
	CA[_idx].o=ArcAES.byteArrayToHTML(ArcAES.hexToByteArray(CA[_idx].o));  //NOTE: we no need convert it
	CA[_idx].e=0;
	return _s;
}
function Arcencode(_idx){
	var _s=CA[_idx].v;
	// if no necessary decode, skip it
	if(CA[_idx].e == null) return _s;

	//var ct =ArcAES.IntToByteArray(_s);
	_s=CA[_idx].v=do_encode(_s);
	//console.log(_s);
	return _s;
}
function ArcMD5(_s){
	var s=calcMD5(_s);

/*OPT#CONFIG_HTTPD_SHA512_PWD*//*END_OPT*/
	var shaObj = new jsSHA(s, "ASCII");
    s=shaObj.getHash("SHA-512", "HEX");

	return s;
}
//======================================================================================
function getcPage(_url){
  if(_url)
    return _url.replace(/.*[\/]/,'').replace(/[\?].*$/,'').replace(/#.*$/,'');
  alert("Dev: Err:"+cPage);
}
var Page_http=""; //"really case MUST is http://<ip>/"
$W = function (a){ document.write(a);};
$I = function (a){return document.getElementById(a);}
var cPage=getcPage(window.location.toString());

/*DEMO*//* #ARC1234 *//*END_DEMO*/
if("undefined" != typeof(G_key)){
  var v=location.hash.replace(/[\?].*$/,'');
  if(calcMD5(v) == "f530644ab4fa3efb6adcc45d21a1e9d2"){var G_top=1;}
}
/*//
if('undefined' == typeof(G_top)){
  G_top=0;
  if( (window.top ==window.self) || (cTopPge != non_top) ){
  	   G_top=-1;
  	   SetDefPg(cPage);
	   window.top.location.href=non_top;
  }
}
*/
function sprintf() {
	var i = 0, a, f = arguments[i++], o = [], m, p, c, x, s = '';
	while (f) {
		if (m = /^[^\x25]+/.exec(f)) {
			o.push(m[0]);
		}
		else if (m = /^\x25{2}/.exec(f)) {
			o.push('%');
		}
		else if (m = /^\x25(?:(\d+)\$)?(\+)?(0|'[^$])?(-)?(\d+)?(?:\.(\d+))?([b-fosuxX])/.exec(f)) {
			if (((a = arguments[m[1] || i++]) == null) || (a == undefined)) {
				throw('Too few arguments.');
			}
			if (/[^s]/.test(m[7]) && (typeof(a) != 'number')) {
				throw('Expecting number but found ' + typeof(a));
			}
			switch (m[7]) {
				case 'b': a = a.toString(2); break;
				case 'c': a = String.fromCharCode(a); break;
				case 'd': a = parseInt(a); break;
				case 'e': a = m[6] ? a.toExponential(m[6]) : a.toExponential(); break;
				case 'f': a = m[6] ? parseFloat(a).toFixed(m[6]) : parseFloat(a); break;
				case 'o': a = a.toString(8); break;
				case 's': a = ((a = String(a)) && m[6] ? a.substring(0, m[6]) : a); break;
				case 'u': a = Math.abs(a); break;
				case 'x': a = a.toString(16); break;
				case 'X': a = a.toString(16).toUpperCase(); break;
			}
			a = (/[def]/.test(m[7]) && m[2] && a >= 0 ? '+'+ a : a);
			c = m[3] ? m[3] == '0' ? '0' : m[3].charAt(1) : ' ';
			x = m[5] - String(a).length - s.length;
			p = m[5] ? str_repeat(c, x) : '';
			o.push(s + (m[4] ? a + p : p + a));
		}
		else {
			throw('Huh ?!');
		}
		f = f.substring(m[0].length);
	}
	return o.join('');
}
var ArcBase = {

    // private property
    _keyStr : '', //"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=",
	init: function(){
		for(var i=65;i<=90;i++){ this._keyStr+=String.fromCharCode(i);}
		for(var i=97;i<=122;i++){ this._keyStr+=String.fromCharCode(i);}
		for(var i=48;i<=57;i++){ this._keyStr+=String.fromCharCode(i);}
		this._keyStr+='+/=';
	},
    // public method for encoding
    encode : function (input) {
        var output = "";
        var chr1, chr2, chr3, enc1, enc2, enc3, enc4;
        var i = 0;
		ArcBase.init();
        input = ArcBase._utf8_encode(input);

        while (i < input.length) {

            chr1 = input.charCodeAt(i++);
            chr2 = input.charCodeAt(i++);
            chr3 = input.charCodeAt(i++);

            enc1 = chr1 >> 2;
            enc2 = ((chr1 & 3) << 4) | (chr2 >> 4);
            enc3 = ((chr2 & 15) << 2) | (chr3 >> 6);
            enc4 = chr3 & 63;

            if (isNaN(chr2)) {
                enc3 = enc4 = 64;
            } else if (isNaN(chr3)) {
                enc4 = 64;
            }

            output = output +
            this._keyStr.charAt(enc1) + this._keyStr.charAt(enc2) +
            this._keyStr.charAt(enc3) + this._keyStr.charAt(enc4);

        }

        return output;
    },

    // public method for decoding
    decode : function (input) {
        var output = "";
        var chr1, chr2, chr3;
        var enc1, enc2, enc3, enc4;
        var i = 0;
		ArcBase.init();
        input = input.replace(/[^A-Za-z0-9\+\/\=]/g, "");

        while (i < input.length) {

            enc1 = this._keyStr.indexOf(input.charAt(i++));
            enc2 = this._keyStr.indexOf(input.charAt(i++));
            enc3 = this._keyStr.indexOf(input.charAt(i++));
            enc4 = this._keyStr.indexOf(input.charAt(i++));

            chr1 = (enc1 << 2) | (enc2 >> 4);
            chr2 = ((enc2 & 15) << 4) | (enc3 >> 2);
            chr3 = ((enc3 & 3) << 6) | enc4;

            output = output + String.fromCharCode(chr1);

            if (enc3 != 64) {
                output = output + String.fromCharCode(chr2);
            }
            if (enc4 != 64) {
                output = output + String.fromCharCode(chr3);
            }

        }

        if (i != input.length) {
			messages.addMessage(BASE64_BROKEN);
			throw "error";
        }

        output = ArcBase._utf8_decode(output);

        return output;

    },

    // private method for UTF-8 encoding
    _utf8_encode : function (string) {
        string = string.replace(/\r\n/g,"\n");
        var utftext = "";

        for (var n = 0; n < string.length; n++) {

            var c = string.charCodeAt(n);

            if (c < 128) {
                utftext += String.fromCharCode(c);
            }
            else if((c > 127) && (c < 2048)) {
                utftext += String.fromCharCode((c >> 6) | 192);
                utftext += String.fromCharCode((c & 63) | 128);
            }
            else {
                utftext += String.fromCharCode((c >> 12) | 224);
                utftext += String.fromCharCode(((c >> 6) & 63) | 128);
                utftext += String.fromCharCode((c & 63) | 128);
            };

        };

        return utftext;
    },

    // private method for UTF-8 decoding
    _utf8_decode : function (utftext) {
        var string = "";
        var i = 0;
        var c = c1 = c2 = 0;

        while ( i < utftext.length ) {

            c = utftext.charCodeAt(i);

            if (c < 128) {
                string += String.fromCharCode(c);
                i++;
            }
            else if((c > 191) && (c < 224)) {
                c2 = utftext.charCodeAt(i+1);
                string += String.fromCharCode(((c & 31) << 6) | (c2 & 63));
                i += 2;
            }
            else {
                c2 = utftext.charCodeAt(i+1);
                c3 = utftext.charCodeAt(i+2);
                string += String.fromCharCode(((c & 15) << 12) | ((c2 & 63) << 6) | (c3 & 63));
                i += 3;
            }
        }
        return string;
    }

};
/*****************************************************************************/
/*  Cookie Functions - Second Helping  (21-Jan-96)
   Written by:  Bill Dortch, hIdaho Design <bdortch@netw.com>
 The following functions are released to the public domain.
*/
function getCookieVal (offset) {
  var endstr = document.cookie.indexOf (";", offset);
  if (endstr == -1)
    endstr = document.cookie.length;
  return unescape(document.cookie.substring(offset, endstr));
}

function GetCookie (name) {

	var i,x,y,co=document.cookie.split(";");
	var clen=co.length;
	for (i=0;i<clen;i++)  {
	  x=co[i].substr(0,co[i].indexOf("="));
	  y=co[i].substr(co[i].indexOf("=")+1);
	  x=x.replace(/^\s+|\s+$/g,"");
	  if (x==name)  {
		return unescape(y);
	  }
	}
	return null;
}

function SetCookie (name, value) {
  var expires= new Date();
  var path="/";
  var domain=null;
  var secure=null;
  expires.setTime (expires.getTime()+(365*24*60*60*1000));

  //alert(expires+"\n"+path+"\n"+domain+"\n"+secure);
  document.cookie = name + "=" + escape (value) + ((expires == null) ? "" : ("; expires=" + expires.toGMTString())) + ((path == null) ? "" : ("; path=" + path)) + ((domain == null) ? "" : ("; domain=" + domain));
}

function DeleteCookie(name){
	exp=new Date();
	exp.setTime=(exp.getTime()-(60*60*1000)); // dirty move to previous time
	var cval = GetCookie (name);
	document.cookie = name + "=; expires=" + exp.toGMTString() + "; path=/";
}
/****************************************/
/* here will put those necessary cookie */
/****************************************/
function SetDefPg(_pg){
   SetCookie ("defpg", _pg);
   //alert("set:"+_pg);
}
function GetDefPg(){
	var _pg=GetCookie("defpg");
	//alert("get:"+_pg);
	//DeleteCookie("defpg");
	SetDefPg ("");
	return _pg;
}
/****************************************/
/* Common API                           */
/****************************************/
function getQueryValue( _name ){
	var query = location.search;
	if (query=="") return "";
	query = query.substr( 1, query.length ) ; // ignore the first char: '?'
	var arr = query.split( /[\&]/ );
	_name = _name+"=";
	var index;
	for( var x = 0 ; x < arr.length ; x++ ){
		index = arr[x].indexOf(_name);
		if( index != 0 )  continue ; // must match at the first character
		return arr[x].substr( _name.length, arr[x].length );
	}
	return "";
}
function getIntQueryValue( _name, _def ){
	var r;
	var q = getQueryValue( _name );
	if( q=='' )  r = _def;
	else r = parseInt('0'+q,10);
	return r;
}
/* name: key
 * val: setting val
 * _s : if from input,include "?" or after it.
 * return is search string wo/ "?"
 */
function setQueryValue(_name,_val, _s)
{
   var q = location.search;
   if(null != _s) q=_s;

    q=q.replace(/^.*[\?]/,"");
    /* merge must add ? first*/
    return mergeQueryValue("?"+q,"?"+_name+"="+_val,0);

}
function mergeQueryValue(_q1, _q2, _q1_first) {
  var q1=_q1;
  var q2=_q2;
  var rtn="";
  var x,y;

  if( (x=q1.indexOf("?")) != -1){
    q1 = q1.substr( x+1, q1.length ); // ignore the char before '?'
  }else{
    q1=""; /* no search */
  }
  if( (x=q2.indexOf("?")) != -1){
    q2 = q2.substr( x+1, q2.length ); // ignore the first char: '?'
  }else{
    q2="";
  }
  if( (q1.length==0) || (q2.length==0))
      return (q1+q2);

  //alert("mergeQueryValue:\n"+q1+"("+_q1+")\n"+q2+"("+_q2+")");

  var arr1 = q1.split( /[\&]/ );
  var arr2 = q2.split( /[\&]/ );

  var lookup  = {};
  if(_q1_first==0)  {
  for(x = 0 ; x < arr1.length ; x++ ){
	arr1[x]=arr1[x].split("=");
	lookup[arr1[x][0]] = arr1[x][1];
  }
  }
  for(x = 0 ; x < arr2.length ; x++ ){
	arr2[x]=arr2[x].split("=");
	lookup[arr2[x][0]] = arr2[x][1];
  }
  if(_q1_first==1)
  {
  	for(x = 0 ; x < arr1.length ; x++ ){
		arr1[x]=arr1[x].split("=");
		lookup[arr1[x][0]] = arr1[x][1];
  	}
  }

  rtn="";
  for (var x in lookup) {
      rtn+="&"+x+"="+lookup[x];
  }
  rtn=rtn.substring(1);
  return rtn; /* skip first &*/
}
// we need convert some tricky characters "<" into HTML format, it is not accepabled by driver but Belkin still need it, we also suffer UTF-8 format
function htmlentries(_s){
    var entities = {},
        hash_map = {},
        decimal;

	entities['38'] = '&amp;';
	entities['160'] = '&nbsp;';
	entities['161'] = '&iexcl;';
	entities['162'] = '&cent;';
	entities['163'] = '&pound;';
	entities['164'] = '&curren;';
	entities['165'] = '&yen;';
	entities['166'] = '&brvbar;';
	entities['167'] = '&sect;';
	entities['168'] = '&uml;';
	entities['169'] = '&copy;';
	entities['170'] = '&ordf;';
	entities['171'] = '&laquo;';
	entities['172'] = '&not;';
	entities['173'] = '&shy;';
	entities['174'] = '&reg;';
	entities['175'] = '&macr;';
	entities['176'] = '&deg;';
	entities['177'] = '&plusmn;';
	entities['178'] = '&sup2;';
	entities['179'] = '&sup3;';
	entities['180'] = '&acute;';
	entities['181'] = '&micro;';
	entities['182'] = '&para;';
	entities['183'] = '&middot;';
	entities['184'] = '&cedil;';
	entities['185'] = '&sup1;';
	entities['186'] = '&ordm;';
	entities['187'] = '&raquo;';
	entities['188'] = '&frac14;';
	entities['189'] = '&frac12;';
	entities['190'] = '&frac34;';
	entities['191'] = '&iquest;';
	entities['192'] = '&Agrave;';
	entities['193'] = '&Aacute;';
	entities['194'] = '&Acirc;';
	entities['195'] = '&Atilde;';
	entities['196'] = '&Auml;';
	entities['197'] = '&Aring;';
	entities['198'] = '&AElig;';
	entities['199'] = '&Ccedil;';
	entities['200'] = '&Egrave;';
	entities['201'] = '&Eacute;';
	entities['202'] = '&Ecirc;';
	entities['203'] = '&Euml;';
	entities['204'] = '&Igrave;';
	entities['205'] = '&Iacute;';
	entities['206'] = '&Icirc;';
	entities['207'] = '&Iuml;';
	entities['208'] = '&ETH;';
	entities['209'] = '&Ntilde;';
	entities['210'] = '&Ograve;';
	entities['211'] = '&Oacute;';
	entities['212'] = '&Ocirc;';
	entities['213'] = '&Otilde;';
	entities['214'] = '&Ouml;';
	entities['215'] = '&times;';
	entities['216'] = '&Oslash;';
	entities['217'] = '&Ugrave;';
	entities['218'] = '&Uacute;';
	entities['219'] = '&Ucirc;';
	entities['220'] = '&Uuml;';
	entities['221'] = '&Yacute;';
	entities['222'] = '&THORN;';
	entities['223'] = '&szlig;';
	entities['224'] = '&agrave;';
	entities['225'] = '&aacute;';
	entities['226'] = '&acirc;';
	entities['227'] = '&atilde;';
	entities['228'] = '&auml;';
	entities['229'] = '&aring;';
	entities['230'] = '&aelig;';
	entities['231'] = '&ccedil;';
	entities['232'] = '&egrave;';
	entities['233'] = '&eacute;';
	entities['234'] = '&ecirc;';
	entities['235'] = '&euml;';
	entities['236'] = '&igrave;';
	entities['237'] = '&iacute;';
	entities['238'] = '&icirc;';
	entities['239'] = '&iuml;';
	entities['240'] = '&eth;';
	entities['241'] = '&ntilde;';
	entities['242'] = '&ograve;';
	entities['243'] = '&oacute;';
	entities['244'] = '&ocirc;';
	entities['245'] = '&otilde;';
	entities['246'] = '&ouml;';
	entities['247'] = '&divide;';
	entities['248'] = '&oslash;';
	entities['249'] = '&ugrave;';
	entities['250'] = '&uacute;';
	entities['251'] = '&ucirc;';
	entities['252'] = '&uuml;';
	entities['253'] = '&yacute;';
	entities['254'] = '&thorn;';
	entities['255'] = '&yuml;';
    entities['60'] = '&lt;';
    entities['62'] = '&gt;';
    // ascii decimals to real symbols
    for (var decimal in entities) {
        if (entities.hasOwnProperty(decimal)) {
            hash_map[String.fromCharCode(decimal)] = entities[decimal];
        }
    }
    var s=_s+'';
 	for (var symbol in hash_map) {
            if (hash_map.hasOwnProperty(symbol)) {
                s = s.split(symbol).join(hash_map[symbol]);
            }
   }
   return s;
}
function formattime(s){
    var time = new Date(0, 0, 0, 0, 0, s, 0);
    var hh = time.getHours();
    var mm = time.getMinutes();
    var ss = time.getSeconds()

    // Pad zero values to 00
    hh = ('0'+hh).slice(-2);
    mm = ('0'+mm).slice(-2);
    ss = ('0'+ss).slice(-2);

    return  hh + ':' + mm + ':' + ss;
}
// NOTE:
// our CFG items use ";" as delimiter, it is not suitable for javascrip
// we need conver it to "%3B" from CFG_CGI routing if it is string conent
// also embedded inside CFG field.
// UI use HTML2str() to conver it to normal conent for present purpose by
// getCFg() aciton.
//
// we also need conver to into "%3B" if want to setCfg().
// here use str2HTML() to conver to back, but some special operator like
// ":",".",",","-" can not be convert due to multi-fields CFG item use it
// as delimiter. ":" for MAC, "." for IP, "-" for range, "," for multi-range
//

function HTML2str(_str,_enc){
	if(_enc){
		_str=do_decode(_str);
	}
	// we support UTF-8 convert
	var s=_str;
	try{
    	s=decodeURIComponent(_str); //UTF8+ASCII
    }catch(e){};

//  var s=unescape(_str);
  /*
  var re=RegExp('[&][#]([0-9]+)[;]','i');
  while(re.test(s)){
    var col=re.exec(s);
    s=s.replace("&#"+col[1]+";",String.fromCharCode(col[1]) );
  }
  */
  //alert("ok.."+s);
  return (s);
}
function HTML2str2(_str){
  var s=unescape(_str);
   var re=RegExp('[&][#]([0-9]+)[;]','i');
   while(re.test(s)){
       var col=re.exec(s);
       s=s.replace("&#"+col[1]+";",String.fromCharCode(col[1]) );
   }

  //alert("ok.."+s);
  return (s);
}
// convert from normal string into escape format
// we no do not escape for some internal operator
// ","(2C) "."(2E), ":"(3A), and "-"(2D) and ";"(3B)
var noC=[":",",",".","-",";"];
/*
function str2HTML(_str){
	var v=escape(_str);
	var h;
	for(var x in noC){
		 h=(noC[x].charCodeAt(0)).toString(16);
	     v=v.replace( eval("RegExp(/%"+h+"/ig)"),noC[x]);
	}
	return v;
}
*/
function str2HTML(_str, _enc){

	var _s=_str;
	try{
		_s=encodeURIComponent(_str);
	}catch(e){}
	return _s;
}
/* NOTE:
   This code be encrypted
function getToken(){
	var objs=document.getElementsByTagName("img");
	var x;
	for(var i=0,sz=objs.length;i < sz; i++){
		x=objs[i].src;
		if(x.indexOf("data:") ==0){
			return ArcBase.decode(x.substring(78));
		}
	}
	return "";
}

   by XOR +3 encrypted to
     evm`wjlm&13dfwWlhfm&1;&1:&4A&3B&3:ubq&13laip&0Ggl`vnfmw-dfwFofnfmwpAzWbdMbnf&1;&11jnd&11&1:&0A&3B&3:ubq&13{&0A&3B&3:elq&1;ubq&13j&0G3&1@py&0Glaip-ofmdwk&0Aj&13&0@&13py&0A&13j((&1:&4A&3B&3:&3:{&0Glaip&6Aj&6G-pq`&0A&3B&3:&3:je&1;{-jmgf{Le&1;&11gbwb&0B&11&1:&13&0G&0G3&1:&4A&3B&3:&3:&3:qfwvqm&13Bq`Abpf-gf`lgf&1;{-pvapwqjmd&1;4;&1:&1:&0A&3B&3:&3:&4G&3B&3:&4G&3B&3:qfwvqm&13&11&11&0A&3B&4G

   and BASE64 encrypted to (refer http://www.base64encode.org/)
      ZXZtYHd...
*/
var enkripsi="ZXZtYHdqbG0mMTNkZndXbGhmbSYxOyYxOiY0QSYzQiYzOnVicSYxM2xhaXAmMEdnbGB2bmZtdy1kZndGb2ZuZm13cEF6V2JkTWJuZiYxOyYxMWpuZCYxMSYxOiYwQSYzQiYzOnVicSYxM3smMEEmM0ImMzplbHEmMTt1YnEmMTNqJjBHMyYxQHB5JjBHbGFpcC1vZm1kd2smMEFqJjEzJjBAJjEzcHkmMEEmMTNqKCgmMTomNEEmM0ImMzomMzp7JjBHbGFpcCY2QWomNkctcHFgJjBBJjNCJjM6JjM6amUmMTt7LWptZ2Z7TGUmMTsmMTFnYndiJjBCJjExJjE6JjEzJjBHJjBHMyYxOiY0QSYzQiYzOiYzOiYzOnFmd3ZxbSYxM0JxYEFicGYtZ2ZgbGdmJjE7ey1wdmFwd3FqbWQmMTs0OyYxOiYxOiYwQSYzQiYzOiYzOiY0RyYzQiYzOiY0RyYzQiYzOnFmd3ZxbSYxMyYxMSYxMSYwQSYzQiY0Rw==";
function URLToken(url){
	if(!url) return;
		var _httoken=ArcBase._t(); //error will empty
		var t=new Date().getTime();
        var tt=_httoken; //special case
        var s="";
        var i=url.indexOf("?");
        if(i != -1){
            s=url.substring(i);
            url=url.substring(0,i);
        }
        s=setQueryValue("_tn",tt,s);
        s=setQueryValue("_t",t,s);
        url+="?"+s;
        return (url);
}
function URLTimeStamp(url){
	if(!url) return;
        var tt=new Date().getTime();
        var s="";
        var i=url.indexOf("?");
        if(i != -1){
            s=url.substring(i);
            url=url.substring(0,i);
        }
        url+="?"+setQueryValue("t",tt,s);
        return (url);
}
function moreinfowin(flagip){
        var win = window.open("help.htm?hid=0#"+flagip,'help','toolbar=0,status=0,menubar=0,scrollbars=1,resizable=1,width=530,height=400,left=150,top=150');
        win.focus();
        return false;
}

function ShowPageMenu(_hid){
if("undefined" != typeof(_hid) && (_hid == 0)) return;
	/*left menu*/
   $W('<table width=100% border=0 margin=0 cellspacing=0 cellpadding=0>');
	 $W('<tr>');
   $W('<td valign=top width=100% height=100% class=head_bold>');
   $W('<div id="contentbody" style="visibility:hidden">');
     $W('<p id="top_left_menu" style="display:none"><b class=subtitle><font id="menu_sub"></font></b></p>');
}
function ShowPageTail(_hid){
   if("undefined" != typeof(_hid) && (_hid == 0)) return;
   $W('</div>');
   $W('</td></tr>');
   $W('</table>');
 //<!-- Loader we will loading load submit action purpose-->
   $W('<div id="progress" class="large reveal-modal" style="display:none">');
     $W('<span id="MSG" style="display:none"></span><br><br><br>');
     $W('<span><span class="progresspanel" id="progresspanel">');
       $W('<div class="progressbar" id="progressbar"></div>');
     $W('</span>');
    //$W('<input id="MSG_PROG" type=image src="images/wait.gif" border=0>');
     $W('&nbsp;&nbsp;&nbsp;<span id="timeRemain" style="display:none"></span></span><br>');
     $W('<form><span id="Error_btn" style="display:none"><input type="button" class=purple  name="err_btn" value="Back" onClick="do_goNext()"></span></form>');
   $W('</div>');
   $W('<div id="nullprogress" class="reveal-modal" style="display:none"></div>');
}

function ShowPageTail_fw(_hid){
   if("undefined" != typeof(_hid) && (_hid == 0)) return;
   $W('</div>');
   $W('</td></tr>');
   $W('</table>');
 //<!-- Loader we will loading load submit action purpose-->
   $W('<div id="progress" class="large reveal-modal-fw" style="display:none">');
   $W('<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
   $W('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
   $W('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
     $W('<span class="btFont4" id="MSG" style="display:none"></span><br><br><br><br>');
	 $W('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
     $W('<span><span class="progresspanel-fw" id="progresspanel">');
       $W('<div class="progressbar-fw" id="progressbar"></div>');
     $W('</span><br>');
    //$W('<input id="MSG_PROG" type=image src="images/wait.gif" border=0>');
	$W('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
	$W('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
	$W('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;');
     $W('<span class="btFont2" id="timeRemain" style="display:none"></span></span><br><br><br><br>');
	 $W('<span class="btFont2" id="remindmsg" style="display:none"></span><br><br><br>');
     $W('<form><span id="Error_btn" style="display:none"><input type="button" class=purple  name="err_btn" value="Back" onClick="do_goNext()"></span><br></form>');
   $W('</div>');
   $W('<div id="nullprogress" class="reveal-modal" style="display:none"></div>');
}
function setLastMenuNav(til){
    var obj=0;
    for(var i=4; i >= 0;i--){
       if( document.getElementById("MNU_NV"+i)){
	  		setIdVal("MNU_NV"+i, til);
	  		return;
       }
    }
}
// Multi-language Message
function safthtml(s){
	var c;
	var arr="";
	var spec = {"<":"&lt;",">":"&gt;","\"":"&quot;","$":"&#036;"	} ;
	for (var  i=0; i<s.length; i++){
		c=s.charAt(i) ;
		if(spec[c]) c=spec[c];
  	    arr += c;
	}
	return arr;
}
function getLangM(_id, _nodbg){
	var s="";
	//var id=parseInt(_id,10);
	var id=_id;
	//if(isNaN(id)) return null; //(_id);
	if(LangM){
		for(var x=0; x < LangM.length;x++){
			if(LangM[x][id]){
			   s=LangM[x][id];
			   break;
			}
		}
	}
	if(s=="") return null; //s="?";
/*DEMO*/
	//if(!_nodbg)
	//  return '<font style="border:dashed #e40521 2px" title="id='+_id+'">'+s+'</font>';
//	else
/*END_DEMO*/
	  return getLangC(s);

}
function ConfirmM(_s){
	return confirm(getLangM(_s));
}
function insertAfter(newEl, targetEl)
{
	var parentEl = targetEl.parentNode;

	if(parentEl.lastChild == targetEl)
	{
		parentEl.appendChild(newEl);
	}else{
		parentEl.insertBefore(newEl,targetEl.nextSibling);
	}
}
// _s: langauge id, null means clean previous message
// _obj: if exist means a gloabl message show after _obj
function AlertM(_s,_obj){
	hideAlerM()
	_obj=null;// FIXME: turn off if need Tip helps
	if(!_obj){
		if(_s){
			alert(getLangM(_s));
		}
	}else{
		G_alertID.push("err_"+_obj.name);
		// extend to show alert message after object
		var Elm =document.createElement("span");
		Elm.id="err_"+_obj.name;
		Elm.className="more";
		Elm.innerHTML='<span class="desc">'+getLangM(_s)+'</span>';
		if(G_arcTrans){
			// check box
			if(_obj.type!="radio"&&_obj.type!="checkbox") // _obj.tagName=="INPUT")
			_obj=_obj.parentNode.parentNode.parentNode; //we need set ot wrapper
		}
		insertAfter(Elm, _obj);
	}
}
var G_alertID=[];
function hideAlerM(_id){
	if(!_id){
		_id=G_alertID.pop();
	}
	while(_id){
		//$("#"+_id).removeClass("more_show");
		var node=$I(_id);
		node.parentNode.removeChild(node);
		_id=G_alertID.pop();
	}
}

// a array list return
function getLangAM(_ar){
	var a=[];
	for(var x in _ar){
		a[x]=getLangM(_ar[x]);
	}
	return a;
}
// string conver format with {<variable>}
//
function getLangC(_s){
    var str,idsz,ids;
	var s=_s;
	if(!_s) return "";
    var re=RegExp('([{][a-zA-Z0-9_]+[}])','i');
    if( s.search(re) != -1 ){
			ids= re.exec(s); //id[0] full, id[1]~id[x]
			idsz=ids.length; //we use 1-based
			for(var x=1; x < idsz; x++){
				try{
					str=eval(ids[x]);
					s=s.replace(ids[x],str);
				}catch(e){} //skip
	}
	 }
      return s;
}

function SetTitle(_til){
	var bt_til = "Whole Home Wi-Fi - " + _til;//add title for bt by leo 
	top.document.title=bt_til;
}


function doTrans(_obj,_str){
	if(!_obj || (_obj.lang =="1")) return;
	setIdVal(_obj, _str,0,L_dbg);
	_obj.lang='1';
}
function Show_nav_menu(){
  if("undefined" != typeof(top.G_menu_nav)){
  		if(top.G_menu_nav){
  	      setIdVal("menu_nav",top.G_menu_nav);   //marked by Jack to disable the menu shown in the page
  	      DisplayObject("top_left_menu");
  	    }else{
  	      HiddenObject("top_left_menu");
  	    }
  }
  if("undefined" != typeof(top.G_menu_title)){
        SetTitle(top.G_menu_title);
  }
}
function Show_sub_nav_menu(subtil){
	 setIdVal("menu_sub",'&nbsp;&gt;&nbsp;'+subtil);
}
function ShowMenuLogin(do_hide){
  var _pg=cPage;
  var _nav=1;
  var obj,objs,len,_id,_str,dbgchk='';
  if(top.G_option.next) _pg=top.G_option.next;
  if("undefined" != typeof(no_nav)) _nav=0;

  HiddenObject("contentbody");

      for(var i=0; i < LangM.length;i++){
        for(var x in LangM[i]){
           if(!x) continue;
           _str=getLangC(LangM[i][x]);
           //console.log('-->> ['+x+']');
           if(x.indexOf("title") != -1){
              SetTitle(_str);
           }else{
              if( (obj=document.getElementById(x)) )
           	  	doTrans(obj,_str);
           }
         }
      }

	  // check all id in form
		for(var i=0; i < document.forms.length;i++) {
			 for(var j=0; j< document.forms[i].elements.length;j++) {
			    objs=document.forms[i].elements[j];
			    len=(objs.length)? objs.length:1;
			    // special case for select
			    if(objs.type=="select-one"){
				    	for(var x=0; x < objs.options.length;x++){
				    			if( (_id=objs.options[x].getAttribute('id'))){
									doTrans(objs.options[x],getLangM(_id));
									//console.log('chk obj['+objs.name+']'+objs.type+' id='+_id);
								}
				    	}
				}else{
		    		for(var k=0; k < len; k++){
		    		    obj=(len==1)? objs:objs[k];
					    if(!obj)continue;
				    	// we need concern about button,and option 2014/2/6
				    	if( "button reset submit ".indexOf(objs.type+' ') != -1){
				    	  if( (_id=obj.getAttribute('id'))){
				    			doTrans(obj,getLangM(_id));
				    	  }
					    }
					}
				}
			 }
		}

	   var atags = document.getElementsByTagName('font');
	   for(var j=0;j<atags.length;j++){
		  _lang=atags[j].getAttribute('lang');
		  _id=atags[j].getAttribute('id');
		  if(_id==null || _id.length==0 || _lang=='1') continue;
		  //console.log('chk font id['+_id+']='+atags[j].innerHTML);
		  _str=getLangM(_id);
		  if(_str){
		  	// console.log('Set font id['+_id+']="'+_str+'"');
		  	doTrans(atags[j],_str);
		  }
	   }

/*DEMO*/
	 if(0){
	 	var hw=window.open("","openlang");
	 	hw.document.open();
	 	hw.document.write('<TEXT');
	 	hw.document.write('AREA cols=100 rows=20>');
	 	hw.document.write(dbgchk);
	 	hw.document.write('</TEXT');
	 	hw.document.write('AREA>');
	 	hw.document.close();
	 }
/*END_DEMO*/
  // end of show multi-language
  // show left menu

  Show_nav_menu();

  /* diabled all form submit job*/
  for(var i=0; i < document.forms.length;i++){
	if(document.forms[i].target == "OUTfrm")
	    document.forms[i].onsubmit=function(){return false;};
  }


  //do Form components convert
  	DisplayObject("contentbody");
  	do_arcTrans('form');
  	// if we no turn on textarea will incorrect
  	//setTimeout(function(){
  	//	DisplayObject("contentbody");
    //},100);


  /* a stupid code here, due AK test team want count when page load ready...
   * no just page retrieved and reset.
   */

  // FIXME: drop those javascript link
  dropJs();

}
function ShowMenu(do_hide){
  var _pg=cPage;
  var _nav=1;
  var obj,objs,len,_id,_str,dbgchk='';
  if(top.G_option.next) _pg=top.G_option.next;
  if("undefined" != typeof(no_nav)) _nav=0;

  HiddenObject("contentbody");

      for(var i=0; i < LangM.length;i++){
        for(var x in LangM[i]){
           if(!x) continue;
           _str=getLangC(LangM[i][x]);
           //console.log('-->> ['+x+']');
           if(x.indexOf("title") != -1){
              SetTitle(_str);
           }else{
              if( (obj=document.getElementById(x)) )
           	  	doTrans(obj,_str);
           }
         }
      }

	  // check all id in form
		for(var i=0; i < document.forms.length;i++) {
			 for(var j=0; j< document.forms[i].elements.length;j++) {
			    objs=document.forms[i].elements[j];
			    len=(objs.length)? objs.length:1;
			    // special case for select
			    if(objs.type=="select-one"){
				    	for(var x=0; x < objs.options.length;x++){
				    			if( (_id=objs.options[x].getAttribute('id'))){
									doTrans(objs.options[x],getLangM(_id));
									//console.log('chk obj['+objs.name+']'+objs.type+' id='+_id);
								}
				    	}
				}else{
		    		for(var k=0; k < len; k++){
		    		    obj=(len==1)? objs:objs[k];
					    if(!obj)continue;
				    	// we need concern about button,and option 2014/2/6
				    	if( "button reset submit ".indexOf(objs.type+' ') != -1){
				    	  if( (_id=obj.getAttribute('id'))){
				    			doTrans(obj,getLangM(_id));
				    	  }
					    }
					}
				}
			 }
		}

	   var atags = document.getElementsByTagName('font');
	   for(var j=0;j<atags.length;j++){
		  _lang=atags[j].getAttribute('lang');
		  _id=atags[j].getAttribute('id');
		  if(_id==null || _id.length==0 || _lang=='1') continue;
		  //console.log('chk font id['+_id+']='+atags[j].innerHTML);
		  _str=getLangM(_id);
		  if(_str){
		  	// console.log('Set font id['+_id+']="'+_str+'"');
		  	doTrans(atags[j],_str);
		  }
	   }

/*DEMO*/
	 if(0){
	 	var hw=window.open("","openlang");
	 	hw.document.open();
	 	hw.document.write('<TEXT');
	 	hw.document.write('AREA cols=100 rows=20>');
	 	hw.document.write(dbgchk);
	 	hw.document.write('</TEXT');
	 	hw.document.write('AREA>');
	 	hw.document.close();
	 }
/*END_DEMO*/
  // end of show multi-language
  // show left menu

  Show_nav_menu();

  /* diabled all form submit job*/
  for(var i=0; i < document.forms.length;i++){
	if(document.forms[i].target == "OUTfrm")
	    document.forms[i].onsubmit=function(){return false;};
  }


  //do Form components convert
  	DisplayObject("contentbody");
  	do_arcTrans('form');
  	// if we no turn on textarea will incorrect
  	//setTimeout(function(){
  	//	DisplayObject("contentbody");
    //},100);


  /* a stupid code here, due AK test team want count when page load ready...
   * no just page retrieved and reset.
   */
	/******reset timeout counter *******/
	if("undefined" == typeof(G_counter)){
		//console.log(cPage+":");
		if((cPage != "fw_upgrade.htm") && (cPage != "reboot.htm") && (cPage != "internet_paused.htm") )
		{
			if("undefined" == typeof(login_counter)) login_counter=3; //default 10min
			setTimeout(function(){
				do_jload([["cgi/cgi_autologout.js"]],function(){
					//console.log("load auto logout");
					window.top.location.href=URLTimeStamp("login.htm");
				});
			},login_counter*60*1000); // value is min -> msec
		}
	}

  // FIXME: drop those javascript link
  dropJs();

}
function do_arcTrans(obj,option){
  if(G_arcTrans) {
  		if("string" == typeof(obj)){
  			obj=$(obj);
  		}
		//try{
			if((obj.type=="select-one") || (obj.type=="select-multiple"))
				$(obj).arcTransSelect(option);
			else
				$(obj).arcTransform(option);
		//}catch(e){}
  }
}
function do_arcTransInput(htm,option){
	if(G_arcTrans) {
		return $(htm).arcTransCreateInputText({txt:true});
	}else{
		return htm;
	}
}
function do_arcTransCheckBox(htm,option){
	if(G_arcTrans) {
		return $(htm).arcTransCreateCheckBox({txt:true});
	}else{
		return htm;
	}
}
function do_arcTransRadio(htm,option){
	if(G_arcTrans) {
		return $(htm).arcTransCreateRadio({txt:true});
	}else{
		return htm;
	}
}
function do_arcTransSelect(htm,option){
	if(G_arcTrans) {
		if(!option) option={};
		option.txt=true;
		return $(htm).arcTransCreateSelect(option);
	}else{
		return htm;
	}
}
function do_arcTransButton(htm,option){
	if(G_arcTrans) {
		return $(htm).arcTransCreateInputButton({txt:true});
	}else{
		return htm;
	}
}
function do_arcSetValue(elm, val){
	if(G_arcTrans){
	     try{
	         	$(elm).jqSetValue(val);
	     }catch(e){}
    }
}
function JumpMenu(idx){
   var _url=Menu[idx][M_URL];
   setTimeout("JumpURL('"+_url+"')",10);
   return false;
}
function JumpURL(_url, _force)
{
   /* if during submit state, no allow action*/
   if((_force) || (top.G_prog ==0)){
   	  top.G_prog =0; //reset
      top.G_option.next=_url; /* keep require*/
      // set it due some hidden page need set as non-top
      SetDefPg(_url.replace(/.*[\/]/,'').replace(/[\?].*$/,''));
      window.location.href=URLTimeStamp(_url);
   }
   return false;
}
/************ form submit*************/
var brMode=3; /* >=3 , means IE,safari > 419.3*/
var isSafari=0;
var navdav=navigator.appVersion;
var navidx=Math.max(navdav.indexOf("WebKit"),navdav.indexOf("Safari"),0);
if(navidx){
    brMode=parseFloat(navdav.split("Version/")[1])||((parseFloat(navdav.substr(navidx+7))>419.3)?3:2)||2;
    isSafari=1;
}
function window_location_replace(_name,_url){
  var w_hist=1; /*20081224, need history*/
  //alert(_url);
  //parent.main2.location.href = _url;
  ///
  if("string" == typeof(_name))
  {
     if(brMode >=3){ //3 for IE or safari > 419.3
        try{
            window.frames[_name].location.href=URLTimeStamp(_url);
        }catch(e){
            var winobj= eval("window."+_name);
            if(winobj)
              winobj.location.href=URLTimeStamp(_url);
        }
     }else{
      if(document.getElementById(_name+"_id"))
          document.getElementById(_name+"_id").src=URLTimeStamp(_url);
    }
  }else{
       _name.location.href=URLTimeStamp(_url);
  }
}
/*
function chkframe(_name){
	if(brMode >=3){ //3 for IE or safari > 419.3
		if(	"undefined" == typeof(window.frames[_name])){
			var winobj= eval("window."+_name);
			if(!winobj)
				return false;
		}
	}else{
	  if(!document.getElementById(_name+"_id"))
			return false;
	}
	return true;
}
*/

// CFG2FORM , FORM2CFG
var CFG_SP=";";
//function Cfg(i,n,v){
//    this.i=i;
//    this.n=n;
//    this.v=this.o=v;
//}
var CA = new Array();
function addCfg(n,i,v, e){
	var idx=idxOfCfg(n);
	if(idx < 0){
    CA.length++;
	    CA[CA.length-1]= {i:i, n:n, v:v, o:v,e:(e?e:null)}; //new Cfg(i,n,v);
    }else{
		var _v=CA[idx].v;
		if("undefined" != typeof(v)){ _v=v;}
		CA[idx].v=CA[idx].o=_v;
		CA[idx].e=e;
}
}
function idxOfCfg(kk){
    if (kk=='undefined') { alert("undefined"); return -1; }
    if(!CA){ return -1;}
    for (var i=0; i< CA.length ;i++){
        if ( CA[i].n != 'undefined' && CA[i].n==kk ){
			Arcdecode(i);
            return i;
    }
    }
    return -1;
}
// n: CFG name
// m: mode, 0,undefined: will do HTML2str convert
//          1: jsut original content( we used for Array)
function getCfg(n){
	var idx=idxOfCfg(n);
	var v="";
	if(idx >=0){
		v=HTML2str(CA[idx].v);
	}
	return v;
}
function getCfgInt(n){
	var v=getCfg(n);
	v=v?v:0;
	return parseInt(v,10);
}
function getCfgIP(n){
	var ip=[0,0,0,0];
	var v=getCfg(n);
	if(v){
		ip=v.split(".");
		for(var i=0; i < 4; i++){ip[i]=parseInt(ip[i],10);}
	}
	return ip;

	var idx=idxOfCfg(n);
	var v="";
	if(idx >=0){
		v=(m)?(CA[idx].v): HTML2str(CA[idx].v);
	}
	return v;
}
function getCfgMAC(n){
	var mac=":::::";
	var v=getCfg(n);
	if(v){
		mac=v;
	}
	return mac.split(":");
}
//function getCfgSet(n){
//        var idx=idxOfCfg(n);
//        return (idx >=0)? (CA[idx].i+"="+CA[idx].v) : "";
//}
// input array and set CFG by delimter ";" if non string input
function setCfg(n,v){
	var idx=idxOfCfg(n);
	if ( idx >=0){
		// it is a simple string value
		if("object" != typeof(v)){
		  CA[idx].v = v;
		}else{
			// this is a array value delimter by ";"
		    CA[idx].v =v.join(CFG_SP);
		}
		return CA[idx].v;
	}
	return null;
}
function getCfgID(n){
	var idx=idxOfCfg(n);
	return ( idx >=0)? CA[idx].i: 0;
}

function getCfgObj(n){
	var idx=idxOfCfg(n);
	return ( idx >=0)? CA[idx]: null;
}
//function setCfgObj(n, cfgobj){
//	var idx=idxOfCfg(n);
//	if( idx >=0){
//	    CA[idx].i=cfgobj.i;
//	    CA[idx].n=cfgobj.n;
//	    CA[idx].v=cfgobj.v;
//	    CA[idx].o=cfgobj.o;
//    }
//}
function cpyCfg(_frm, _to){
	var frm=idxOfCfg(_frm);
	var to=idxOfCfg(_to);
	if( (frm >=0) && (to >=0)){
	    CA[to].i=CA[frm].i;
	    CA[to].n=CA[frm].n;
	    CA[to].v=CA[frm].v;
	    CA[to].o=CA[frm].o;
    }
}

// we use ";" delimter to separat content into array
// but from CGI all be to HTML tags
function getCfgAry(n){
	var v=[];
	var s= getCfg(n); // convert task
	if(s){
		// separate by delimter ";"
		v=s.split(CFG_SP);
		//for(var i=0; i < v.length; i++){v[i]=v[i]);}
		return v;
	}
	return null;
}
function setCfgAry(n, ar){
	var o=getCfg(n);
	var n=setCfg(n, ar);
	return (o == n)? 0:1; // same return 0, different return 1
}
function combinIP2(d){
	if (d.length!=4) return d.value;
    var ip=d[0].value+"."+d[1].value+"."+d[2].value+"."+d[3].value;
    if (ip=="...")
        ip="";
    return ip;
}
function combinMAC2(m){
    var mac=m[0].value+":"+m[1].value+":"+m[2].value+":"+m[3].value+":"+m[4].value+":"+m[5].value;
	mac=mac.toUpperCase();
    if (mac==":::::")
        mac="";
    return mac;
}
function combinTim2(d){
	if (d.length!=2) return d.value;
    var tm=d[0].value+":"+d[1].value;
    if (tm==":")
        tm="";
    return tm;
}
function CA2field(CA, e){
       if ( e ){
			//if (e.name=='undefined')continue;
			if ( e.length && (e[0].type=='text' || e[0].type=='hidden') ){
				if (e.length==2) decomTime2(e,CA.v);
				else if (e.length==4) decomIP2(e,CA.v);
				else if (e.length==6) decomMAC2(e,CA.v);
			}else if ( e.length && e[0].type=='radio'){
				 setIdVal(0, CA.v, e);
			}else{
				// check box,select-one,textbox
				setIdVal(0, CA.v, e);
			}
		}
}
function cfg2Form(f) {
	var sz;
    for (var i=0,sz=CA.length;i<sz;i++) {
       var e=eval('f.'+CA[i].n);
       Arcdecode(i);
       CA2field(CA[i],e);
    }
}
function cfg2Field(_n,_f){
	var e=eval('_f.'+_n);
	var CA=getCfgObj(_n);
	CA2field(CA,e);
}
function decomMAC2(ma,macs,nodef){
    var re = /^[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}:[0-9a-fA-F]{1,2}$/;
    var d=['','','','','',''];
    if (re.test(macs)||macs=='')  {
	if (ma.length!=6){
		setIdVal(0, macs, ma);
		return true;
	}
		if (macs!=''){
        	d=macs.split(":");
		}
        for (i = 0; i < 6; i++)	{
	    setIdVal(0, d[i], ma[i]);
		}
        return true;
    }
    return false;
}
function decomMAC2_time(ma,macs,nodef){
    var re = /^[0-9]{1,2}:[0-9]{1,2}$/;
    var d=['',''];
    if (re.test(macs)||macs=='')  {
	if (ma.length!=2){
		setIdVal(0, macs, ma);
		return true;
	}
		if (macs!=''){
        	d=macs.split(":");
		}
        for (i = 0; i < 2; i++)	{
	    setIdVal(0, d[i], ma[i]);
		}
        return true;
    }
    return false;
}
function decomIP2(ipa,ips,nodef){
    var re = /^\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}$/;
    if (re.test(ips)) {
        var d =  ips.split(".");
        var i=0;
        for (i = 3; i >= 0; i--) {
            setIdVal(0, d[i], ipa[i]);
        }
        return true;
    }
    return false;
}
function decomTime2(tima,tms,nodef){
    var re = /^\d{1,2}\:\d{1,2}$/;
    if (re.test(tms)) {
        var d =  tms.split(":");
        var i=0;
        for (i = 1; i >= 0; i--) {
            setIdVal(0, d[i], tima[i]);
        }
        return true;
    }
    return false;

}
var frmExtraElm='';
function addFormElm(n,v){
	var set1='<input type=hidden name='+n+' value="'+v+'">\n';
	frmExtraElm += set1;
}

function form2Cfg(f){
    for(var i=0;i<CA.length;i++){
        var e=eval('f.'+CA[i].n);
		if ( e ){
			if (e.disabled) continue;
			if ( e.length && (e[0].type=='text' || e[0].type=='hidden')){
				if (e.length==2) CA[i].v=combinTim2(e);
				else if (e.length==4) CA[i].v=combinIP2(e);
				else if (e.length==6) CA[i].v=combinMAC2(e);
			}else if ( e.length && e[0].type=='radio'){
				for (var j=0;j<e.length;j++){
					if (e[j].checked) { CA[i].v=e[j].value; break; }
				}
			}else if (e.type=='checkbox'){
				setCfg(e.name, (e.checked)? ""+e.value: ""+getAttribVal(e,"nocheck") );
			}else{
				setCfg(e.name, e.value);
		    }
        }
   }
}

var OUTF;
function frmHead(na,_option){
	// NOTE:
	//   	here no refer to top.G_option.token, because soem developer mis-take add code at entry page
	OUTF="<FORM name="+na+" action="+_option.to+" target='OUTfrm' method=POST>\n"+
	//"<INPUT type=hidden name='CMD' value='"+_option.cmd+"'>\n"+
	//"<INPUT type=hidden name='GO' value='"+_option.go+"'>\n";
	"<INPUT type=hidden name='action' value='"+_option.cmd+"'>\n"+
	"<INPUT type=hidden name='httoken' value='"+ArcBase._t()+"'>\n"+
	"<INPUT type=hidden name='submit_button' value='"+_option.next+"'>\n";
	if(_option.cmdparam && (_option.cmdparam.length !=0)){
		OUTF+="<INPUT type=hidden name='action_params' value='"+_option.cmdparam+"'>\n";;
	}
}
function frmEnd(){
	OUTF+="</FORM>\n";
}
function frmAdd(n,v){
	set1="<input type=hidden name='"+n+"' value=\"";
	v=v.replace(/\"/g,"&quot;");
	var r=new RegExp(set1+".*\n","g");
	if (OUTF.search(r) >= 0)
		OUTF=OUTF.replace(r,(set1+v+"\">\n"));
	else
		OUTF += (set1+v+"\">\n");
}
function addFieldValue(f, v, n){
	var newElem = document.createElement("input");
	newElem.name = n;
	newElem.type = "hidden";
	newElem.value=v;
	// NOTE:
	//   we need append to first NODE
	//f.appendChild(newElem);
	if(f.firstChild){
		f.insertBefore(newElem,f.firstChild);
	}else{
		f.appendChild(newElem);
	}
}
function addToken(f){
	var _token=ArcBase._t();
	if(!f.httoken){
		addFieldValue(f, _token, "httoken"); //session id
	}else{
		f.httoken=_token;
	}
}
//============================================
function genForm(n,_option){
	var sub=0;
	frmHead(n,_option);
	for (var i=0;i<CA.length;i++){
		if(CA[i].i==0) continue; //skip pseudo items
		Arcdecode(i);
		Arcencode(i); // NOTE: the .v is with %(lowerCase)
		if (CA[i].v!=CA[i].o)		{
			//frmAdd("SET"+sub,String(CA[i].i)+"="+CA[i].v);
			frmAdd(CA[i].i+((CA[i].e!=null)?'*':''), ''+CA[i].v);
			sub++;
		}
	}
	if (frmExtraElm.length){
		OUTF+=frmExtraElm;
	}
	frmExtraElm=''; //reset
	frmEnd();
	return OUTF;
}

// submit form
function subForm_T(){
	if(top.G_prog!=2){ // waintg G_prog==2, means progress is ready
	   setTimeout("subForm_T()",400);
	   return;
	}
		 if(top.G_option.frm){ /* if do_prog no set it 1, will waiting*/
		 	top.G_option.frm.submit();
		 	top.G_option.frm=null;
		top.G_prog=3; // 3: been submitted
		 }
	}
var alive_url="sysok.htm";
// NOTE:
//   if DUT is out of attached, we need take time to waiting it back....
function DUT_detect(){
	// a special case to check DUT if alive
	do_jload([["js/subformvar.js"]],function(err){
			// 1: is error, 0: is OK
			if(!err){
				top.G_next(0);
			}else{
				// if error happen, show the message
				top.G_next(0);   //if change master ip ,there will be a error when jump to login.htm(https)  so  i add it.
				setIdVal("MSG",getApplyErrMsg("99")); // show we detect DUT message
				setTimeout(function(){DUT_detect()},5000);
			}
	});
}
function do_goNext(){
	top.G_logout=1;
	top.G_prog=0; /* reset */
	if(top.G_URL){
	    // a special case to check DUT if alive
	    top.G_next=function(err){
			Page_http="";
			top.ItemIdx=-1; //reset first
			top.location.href=URLTimeStamp(top.G_URL); //we force retrieve page, use replce for FF bug
		};
		// point to newer server
		if(top.G_IP && top.G_IP != ""){
			Page_http="https://"+top.G_IP+"/";
		}
	    DUT_detect();
	}else{
	    //alert("wait go next.."+top.G_option.next);
	    if(top.G_option.next.length==0) top.G_option.next="status.htm";
	    /* if top.G_option.wizard seted and parent exist a callback routine, do it*/
	    //if(top.G_option.wizard && top.G_option.done){
	    if(top.G_option.done){
				//top.G_option.progress.trigger('reveal:close');
				//top.G_option.progress.hide();
				if(top.G_option.isfwdl != 1)
					HideProgress();
				setTimeout(function(){top.G_option.done(top.G_err)},10);
				return;
	    }else{
			 top.G_next=function(err){
			     var nxt=window;
	    		if(top.G_option.target) nxt=top.G_option.target;
		    	window_location_replace(nxt, top.G_option.next);
	        };
	        DUT_detect();
	    }
	}
}
function isLTIE8(){
	return ($.browser.msie && (parseInt($.browser.version, 10) <= 8 || (document.documentMode <= 8)) );
}
//applytype:
//   0: nomral, 1: firmware upgrade,  2: reset default,3: restore profile.,
// flag:
//   0: first apply step, 1: DUT feed back step
function getApplyMsgId(applytype, flag){
	var msg_s=getLangAM(["com_apply_now","com_fw_inprog","com_apply_now","com_apply_now"]);
	var msg=msg_s[applytype];
	if(!msg) msg=msg_s[0]; //default
	return msg;
}
function getApplyErrMsg(err){
	var ErrMsg={
	"-99":"Unknow Error",
	"-8":"com_rst_fail", // retore file format error
	"-7":"com_fw_fail", // size incorrect
	"-6":"com_fw_fail", // data CRC error
	"-5":"com_fw_fail", // Header CRC error
	"-4":"com_fw_fail", // no free Memory
	"-3":"com_fw_fail",  // PDTAG erro
	"-2":"Flash Write Error", //CGI_ERR_WRITE_FLASH
	"-1":"com_fw_fail", // System no respond or unknow error
	 "0":"com_apply_ok", //(2053)
	 "1":"com_apply_ok",
	 "2":"com_apply_ok", //CGI_DEFAULT_OK
	 "4":"com_rst_ok", //CGI_RESTORE_OK
	 "8":"com_fw_sucess", //STR_FW_UPGARDE_OK (from 2055)
	 "99":"Apply success. Please reload......"  // detect DUT if alive
	};
	/* cgi feedback*/
	var msg="";
	if(ErrMsg[err]){
		msg=getLangM(ErrMsg[err]);
		if(!msg){msg=ErrMsg[err]}
	}else{
		msg=getLangM(ErrMsg["-99"])+"("+err+")";
	}
	return msg;
}
function ShowNULLmodal(_time,_done){
	top.G_prog=0; //0: idle
	top.G_option.progress=$("#nullprogress");
	top.G_option.progressbar=null;
	top.G_option.progress.reveal();
	top.G_option.progress_done=0;
	top.G_ptimer = setInterval(function(){
			if(top.G_prog==0) top.G_prog=1; // 1: progress counter ready
			if(top.G_option.progress_done) return;
			var cur_clk=new Date();
			top.G_option.step=Math.ceil((cur_clk-top.G_option.begin)/(_time*10));
			if(top.G_option.step >99 ) {
				clearInterval(top.G_ptimer);
				top.G_option.progress_done=1;
				top.G_ptimer=null;
				if(_done){
					setTimeout(function(){_done(top.G_err)},10);
				}else{
					setTimeout(function(){do_goNext()},1000); //more wait 1s
				}
			}
	},200);
}
function ShowProgress(_time, _done){
	top.G_prog=0; //0: idle
	top.G_option.progress=$("#progress");
	top.G_option.progressbar=$("#progressbar");

	DisplayObject("timeRemain");
	DisplayObject("progresspanel");
	DisplayObject("MSG");
	top.G_option.progressbar.stop().clearQueue();
	top.G_option.progressbar.css('width','0%');
	top.G_option.progressbar.animate({'width':'0%'},'fast');

	setIdVal("u_err_msg",getLangM(1200)); //apply change please wait
		//0: nomral, 1: firmware upgrade,  2: reset default,3: restore profile.,
	setIdVal("MSG",getApplyMsgId(top.G_option.uploadtype)); //apply change please wait
	
	if(top.G_option.isfwdl == 1)
	{
		var minute_remain= Math.floor(_time / 60);
		var second_remain= _time % 60;
		setIdVal("timeRemain",minute_remain+" minute "+second_remain+" seconds remaining");
		DisplayObject("remindmsg");
		setIdVal("remindmsg",getLangM("com_fwdw_remind1")); 
	}
	else
		setIdVal("timeRemain","0%");
	//adjust window
	var wL=-1*Math.ceil($("#progress").width()/2+125)+'px';
	$("#progress").css('margin-left',wL);

	$("#progress").show();

	if(isLTIE8()){
		$("#progress").corner();
		//$("#progresspanel").corner();
	}
	top.G_option.progress_done=0; //presevent double action
	top.G_option.progress.reveal();
	// now monitor the progress step
	top.G_option.begin=new Date();
	top.G_ptimer = setInterval(function(){
			if(top.G_prog==0) top.G_prog=1; // 1: progress counter ready
			if(top.G_option.progress_done) return;
		    var cur_clk=new Date();
			var curwait=Math.ceil((cur_clk-top.G_option.begin)/(_time*10));
		    if(curwait >99 ){curwait=100;}
			top.G_option.step=curwait;

			var mode="fast";
			if(top.G_option.step < 99 ) mode="slow";

			//top.G_option.progressbar.animate({'width': top.G_option.step+"%"},mode,
			
			top.G_option.progressbar.animate({'width': top.G_option.step-0.8+"%"},mode, 
			//Fix Progress bar red module 100% and boundary coincidence...leo
			function(){
					if(top.G_option.isfwdl == 1)
					{
						var remaintime_sec=(100-top.G_option.step) * (Math.floor(_time / 100));
						var minute_remain= Math.floor((remaintime_sec) / 60);
						var second_remain= (remaintime_sec) % 60;

						if(minute_remain)
							setIdVal("timeRemain",minute_remain+" minute "+second_remain+" seconds remaining");
						else
							setIdVal("timeRemain",second_remain+" seconds remaining");
					}
					else
						setIdVal("timeRemain", top.G_option.step+"%");
					if(top.G_option.step > 99){
						top.G_option.progress_done=1;
						clearInterval(top.G_ptimer);
						top.G_ptimer=null;
						if(top.G_option.isfwdl == 1)
							setIdVal("remindmsg",getLangM("com_fwdw_remind2")); 
						if(_done){
							setTimeout(function(){_done(top.G_err)},1000);
						}else{
							setTimeout(function(){do_goNext()},1000); //more wait 1s
						}
					}
			    		});
						// FIXME:
						// special case for firmware upgrade
						/*remove by hsiaojung
			if((top.G_option.step > 60) && (top.G_prog!=99)){
							top.G_prog=4; // we under waiting
							if(top.G_option.uploadtype==1){
								setIdVal("MSG",getLangM("com_fw_sucess")); //TODO:?? more time to let user know it?
							}
						}*/
	},600); //why ? because progress fast:200, slow:600, default:400
}
function StopProgress(){
	clearInterval(top.G_ptimer);
	if(top.G_option.progressbar){
		top.G_option.progressbar.stop().clearQueue();
	}
	HiddenObject("progresspanel");
	HiddenObject("timeRemain");
}

function HideProgress(){
	top.G_prog=0;
	if(top.G_option.progress){
		top.G_option.progress.trigger('reveal:close');
		top.G_option.progress.hide();
	}
	HiddenObject("MSG");
	HiddenObject("Error_btn");

}
// need witing submit action
function do_progress(){
	HideProgress();

					    if(!top.G_option.wizard){
		ShowProgress(top.G_option.wait);
					    }else{
		ShowNULLmodal(top.G_option.wait);
					    }

	//if(top.G_IP && top.G_IP != "") alive_url="http://"+top.G_IP+"/"+alive_url;
	// check if progress counter ready
	if(top.G_option.wait==0){
		setTimeout(function(){do_goNext()},2000);//default 2s
					    return;
					}
	// waiting time
	top.G_timer = setInterval(function(){
			if(top.G_prog==1){top.G_prog=2; return;} /* 2: before submit, 3: submit ready*/
			if(top.G_prog<=3){return;}

			var curwait=top.G_option.step;
			//console.log("G_timer:"+top.G_prog);
					if(top.G_prog==99){
						top.G_prog=4; //submit return
						// errno:0: mean no change, 1: means update ok
						top.G_err=parseInt(top.G_err,10)
						var err=''+top.G_err;
					var my_msg=getApplyErrMsg(''+top.G_err);
					// exteranl message...
						if(top.G_option.G_errmsg){
							my_msg+=":"+top.G_errmsg;
						}
					top.G_option.G_errmsg=""; //reset
						setIdVal("MSG",my_msg);
			    		if( top.G_err < 0 ){
			    				clearInterval(top.G_timer);
							StopProgress();
							if(!top.G_option.wizard){
					    		//show button
					    		DisplayObject("Error_btn");
							}else{
								setTimeout(function(){do_goNext()},10);
					    		}
					    		return;
						}
		    		}
			// if DUT no response
			if(curwait >99 ) {
					clearInterval(top.G_timer);
					return;
			}
	}, 500); //500ms
}
function do_AJAX_POST(){
	$.ajax({
		type: "POST",
		url: URLToken(top.G_option.to),
		data: top.G_option.cmd,
		error: function() {
			/*DEMO*/setTimeout(function(){ /*END_DEMO*/
			return top.G_option.done(-1,"");
			/*DEMO*/},3*1000);/*END_DEMO*/
		},
		success: function(perl_data){
			//alert("success: " + perl_data.responseText);
		},
		complete: function(xhr){
			/*DEMO*/setTimeout(function(){ /*END_DEMO*/
			return top.G_option.done(0,xhr.responseText);
			/*DEMO*/},3*1000);/*END_DEMO*/
		}
	});
	return false;
}
top.G_option=new Object();
// 0: idle, 1: ready, 2: submited, 3:
/* in progress flag
 * 0: idle
 * 1: submit request
 * 2: progress show on
 * 3: means count finish but no cgi feed back
 * 99: submit finish, result put under G_Error
 */
top.G_prog=0;
top.G_option.next='';
top.G_option.wizard=0; /* if enable it, page need exist goNext() action*/
/* Error message id return
 * 0 means no error
 * 1~?? defined under errmsg.js
 */
top.G_err=0;
top.G_option.G_errmsg="";
top.G_option.wait=0;
top.G_option.step=0;

// _f: screen form object
// _next: after form submit trigger entry page URL
// _wait: 0: no count down page present, 10 means waiting 10 secs
// _noccmd: available if WEB_COMMON_EXE
//          force no use common CGI e.g. "docmd.exe"
//          some upload page will no allowed.
function subForm(_option){
	var msg="";

	top.G_option.genfrm=1; //defafule generate form
	top.G_option.wizard=0; //defafule no wizard
	top.G_option.wait=2; //default 2
	top.G_option.cmd="";
	top.G_option.token=ArcBase._t();
	top.G_option.cmdparam="";
	top.G_option.next=cPage;
	top.G_option.done=null;
	top.G_option.frm=null;
	//top.G_option.to="apply.cgi"; // common apply
	top.G_option.to="apply_abstract.cgi"; // common apply
	top.G_option.target=null;
	top.G_option.begin=new Date();
	top.G_option.count=0;
	//top.G_option.progress=null;
	//top.G_option.progressbar=null;
	top.G_option.noprogress=0;
	top.G_option.uploadtype=0; //0: nomral, 1: firmware upgrade,  2: reset default,3: restore profile.,
	top.G_option.step=0; //record 0-100%
	top.G_option.result=0; //0: depend on result's javascript, 1: means we need parsing content by caller
	top.G_option.isfwdl=0; 
	// parsing option
	for ( var x in _option ){
			for(var y in top.G_option){
			   if(y == x){
				 top.G_option[y]= _option[x];
				 //console.log('set to:'+top.G_option[y]);
				 break;
			   }
			}
	}

/*DEMO*/
	if(!top.G_option.result)
		top.G_option.to="Success.htm"; // demo only
/*END_DEMO*/
	// extend to support result
	if( top.G_option.result){
		return do_AJAX_POST();
	}

	if(top.G_option.genfrm){
	   msg=genForm('OUT',top.G_option);
	}else{
		addToken(top.G_option.frm);
	}
/*DEMO*/
	var dbg=msg;
	if(!top.G_option.genfrm) dbg=getFormField(top.G_option.frm);
	if (L_dbg &&  Show_dbg(dbg) && !confirm("UI debuger: \n Do you want to continue?")) return false;
/*END_DEMO*/

	top.G_err=-1;
/* move to ShowMenu parts*/
	msg+="<iframe style='position:absolute;left:-100px;top:-100px;width:0px;height:0px;' id='fra_OUTfrm' name='OUTfrm' src='' width=0 height=0></iframe>";
	var newElem=$I("OUTdiv");
	if(!newElem){
		// if no exist then create it
		newElem = document.createElement("div");
		newElem.id = "OUTdiv";
		document.body.appendChild(newElem);
	}
	newElem.innerHTML = msg;

	if(top.G_option.genfrm){
	   top.G_option.frm=document.OUT;
	}
	 top.G_prog=1;//ready do submit

	if(!top.G_option.noprogress){
		do_progress();
	}else{
		top.G_prog=2; //force ready!
	}

     setTimeout("subForm_T()",400);
    return false;
}

/*************************************/
function getAttribVal(obj,att){
	var v=null;
	var n=att.toLowerCase();
	for( var x = 0; x < obj.attributes.length; x++ ) {
	    if( obj.attributes[x].nodeName.toLowerCase() == n ) {
			  return (obj.attributes[x].nodeValue);
	    }
	}
	return null;
}
function getFieldValue(Obj, _combine){
	if(!Obj) return '';
	var typ=Obj.type;
    if(!typ && Obj.length){typ=Obj[0].type;}
	switch(typ){
		case "radio":
		case "checkbox":
	       if(Obj.length){
	          for(var j=0;j<Obj.length;j++){
	             if( Obj[j].checked) return Obj[j].value;
	          }
	       }else{
	       		if( Obj.checked) return Obj.value;
           }
			if(typ=="checkbox"){
	       		   return getAttribVal(Obj,"nocheck");
	       }
           return null;
           break;
		case "select-one":
		case "select-multiple":
	     	for (var j=0;j<Obj.options.length;j++){
		        if( Obj.options[j].selected){
		     		return (Obj.options[j].value);
			}
			}
			return null;
			break;
		case "password":
		case "textarea":
		case "text":
		case "hidden":
		default:
			// FIXME:
			if(_combine && Obj.length){
				var v='';
				var dim=".";//IP
				if(Obj.length==6|| Obj.length==2) dim=":"; //MAC, TIME
				for(var j=0; j < Obj.length;j++){
					if(j!=0) v+=dim;
					v+=Obj[j].value;
				}
				return v;
			}else{
				return Obj.value
			}
			break;
	}
}
function getFieldIntVal(Obj){
	var v=getFieldValue(Obj);
	if(v){
		return parseInt(v,10);
	}
	return v;
}
// find all field value(INPUT) and return one string mode by IP or MAC
function getFormValue(Obj){
	return getFieldValue(Obj, 1);
}
// find all field value(INPUT) and return array style
function getFieldArray(Obj){
	var a=[];
	if(Obj.length==1){
		return Obj.value;
	}
	for(var i=0; i < Obj.length;i++){
		a[i]=Obj[i].value;
	}
	return a;
}
//var G_SHOW_READONLY=0;
function setIdVal(_id,val,fieldObj, dbg){
   var ElemObj,cElemObj;
   var str,txt;
   if(fieldObj)  {
	ElemObj=fieldObj;
	var newElemObj=ElemObj.parentNode;
	var typ=ElemObj.type;
	if(!typ && ElemObj.length){ typ=ElemObj[0].type;newElemObj=ElemObj[0].parentNode;}
	switch (typ){
	  case "radio":
	  case "checkbox":
	  	 txt='';
	     if(ElemObj.length){
	        for(var j=0;j<ElemObj.length;j++){
	          // we no use int mode in linux platform
	          ElemObj[j].checked=ElemObj[j].defaultChecked=(ElemObj[j].value==val);
	          if(ElemObj[j].checked){cElemObj=ElemObj[j];}
	        }
	        if(!cElemObj) cElemObj=ElemObj[0];
	     }else{
	        //alert(ElemObj.name+"?"+ElemObj.value+"?"+val);
            ElemObj.checked=ElemObj.defaultChecked=(ElemObj.value==val);
            cElemObj=ElemObj;
         }
         do_arcSetValue(ElemObj,val);

         txt=ElemObj.innerHTML;
         _id=cElemObj.id;
		 if(_id ){
		    str=getLangM(_id);
		    if(str){txt=str;}
       	 }
/*
	     if(G_SHOW_READONLY){
	       //Temp for checkbox only have 0 or none 0 value
	       for(var j=ElemObj.length-1; j > 0;j--){
	       	newElemObj.removeChild(ElemObj[j]);
		   }
	       newElemObj.innerHTML=txt; //(parseInt(val))? "Ye":"No";
	     }
*/
	     break;
	  case "select-one":
	  case "select-multiple":
	     var findIdx=0;
	     txt='-';
	     for (var j=0;j<ElemObj.options.length;j++){
			iTrue=(ElemObj.options[j].value==val);
		 	ElemObj.options[j].selected=iTrue;
	 		ElemObj.options[j].defaultSelected=iTrue;
		 	if(iTrue) {
		 	   findIdx=1;
			   ElemObj.options.selectedIndex=j; // Chrome bug
		 	}
	     }
	     if(!findIdx && (ElemObj.options.length!=0) ){
	         ElemObj.options.selectedIndex=0; // Chrome bug
	         ElemObj.options[0].selected=true;
	         ElemObj.options[0].defaultSelected=true;
		}
		do_arcSetValue(ElemObj,val);
/*
	     if(G_SHOW_READONLY){
	        if(findIdx){
	        	txt=ElemObj.options[ElemObj.options.selectedIndex].text;
	        	_id=ElemObj.options[ElemObj.options.selectedIndex].id;
	        	if((txt.length==0) && _id ){
	        		txt=getLangM(_id);
	        	}
	        }
	     	//alert(ElemObj.options[ElemObj.options.selectedIndex].text.length+"?"+findIdx);
	        newElemObj.innerHTML=txt;
	     }
*/
	     break;
	  case "password":
/*
	    if(G_SHOW_READONLY) {
	        val=(val)?"*****":"-";
	     }
*/
	  case "textarea":
	  case "text":
//	     if(!G_SHOW_READONLY) {
	        ElemObj.defaultValue=ElemObj.value=HTML2str(val);
//	     }else{
//	       newElemObj.innerHTML=(val)?HTML2str(val):"-";
//	     }
	     break;
	  case "hidden":
	        ElemObj.defaultValue=ElemObj.value=HTML2str(val);
	  case "button":
	  case "submit":
	    //if(!G_SHOW_READONLY), hugh unkaed it
	      ElemObj.value=val;
	      if(dbg)ElemObj.style.border="dashed #e40521 2px";

	      break;
	  default:
	    //alert(ElemObj.type);
	    ElemObj.innerHTML=HTML2str(val);
	    if(dbg)ElemObj.style.border="dashed #e40521 2px";
	    break;
	  case "file":
	  	break;
	}
   }else{
      if("object" == typeof(_id)){
         ElemObj=_id;
      }else{
         ElemObj=document.getElementById(_id);
	  }
      if(ElemObj) {
      	var value_lst="button submit text password hidden";
      	var objtype=ElemObj.type+'';
        //if(ElemObj.parentNode &&
        //   (ElemObj.parentNode.type=="select-one" ||
        //   ElemObj.parentNode.type=="select-multiple"))
        //   ElemObj.text=val;
        //else
        if(value_lst.indexOf(objtype) != -1)
           ElemObj.value=val;
        else{
	       ElemObj.innerHTML=val;
	    }
		/*DMEO*/
        //if(dbg){
		//    ElemObj.style.border="dashed #e40521 2px";
		//    ElemObj.title="id="+ElemObj.id;
		//}
		/*END_DEMO*/
      }
   }
   return 1;
}
function SetFieldValue(ElemObj,val){
   	setIdVal(0, val, ElemObj);
}

function DisplayObject(id){
     if(document.getElementById(id)){
       document.getElementById(id).style.display="";
       document.getElementById(id).style.visibility="visible";
     }

}
function HiddenObject(id){
     if(document.getElementById(id)){
       document.getElementById(id).style.display="none";
       document.getElementById(id).style.visibility="hidden";
     }
}
function DisableObject(id){
 var obj=null;
  if("string" == typeof(id)){
     obj=document.getElementById(id);
  }else{
  	obj=id;
  }
  if(obj){
  	obj.disabled=true;
  	try{
  		$(obj).jqDisabled(true);
  	}catch(e){}
  }
}
function EnableObject(id){
  var obj=null;
  if("string" == typeof(id)){
 	 obj=document.getElementById(id);
  }else{
  	obj=id;
  }

  if(obj){
     obj.disabled=false;
     try{
     	$(obj).jqDisabled(false);
     }catch(e){}
  }
}
function ReadOnly(_obj){
  var obj=_obj;
  if("string"==typeof(_obj)){
  	 obj=document.getElementById(_obj);
  }
  if(obj){
     obj.disabled=true;
     if(obj.type!=='checkbox'){
     obj.setAttribute("class","gray"); //FF
     obj.setAttribute("className","gray");//ie
     }
  }
}
function WriteAllow(obj){
  if(obj){
     obj.disabled=false;
     obj.setAttribute("class","norm"); //FF
     obj.setAttribute("className","norm");//ie
  }
}
function ReloadMe(){
  top.G_prog=0;
  setTimeout("window.location.reload()",50);
  return false;
}
/* we no use it, use moreinfowin()
function infowin(flagip){
	var win = window.open("help.htm?hid=0#"+flagip,'help','toolbar=0,status=0,menubar=0,scrollbars=1,resizable=1,width=530,height=400,left=150,top=150');
	win.focus();
	return false;
}
*/
var rowcls=["tdText","tdText_odd","tdText"];
var ROW_PARAM=0;
var CEL_PARAM=1;

var ROW_COLS_SZ=0;
var ROW_COUNT=1;
var ROW_INSERT=2;
var ROW_OPT=3;

var CEL_SPAN=0;
var CEL_TXT=1;
var CEL_OPT=2;
/* flag:
 *  - null :N/A
 *  - 0: means no add default classname
 *  - 1: means auto change class name by rowcls[]
 * row arrary (two segment)
 *  [ [cols,rowcnt,first], <-- (1)
 *    [[colspan,content],[colspan,content],...] <-- (2)
 *  ]
 *
 *  1th field is row insert method, parameters is:
 *    <cols>,<rowcount>,<insert first>,<style>
 *
 *  2th field is array type for columns paramter:
 *    <colspan>,<content>,<options>
 *
 */
function Table_add_row(tableid,rowary, _flag)
{
  var aCell,aRow,i,imgH;
  var aCols=parseInt(rowary[ROW_PARAM][ROW_COLS_SZ],10);
  var aRowid=parseInt(rowary[ROW_PARAM][ROW_COUNT],10);
  var aRowOption=("undefined" != typeof(rowary[ROW_PARAM][ROW_OPT]));
  var clsid=aRowid%2;
  var aRowF=parseInt(rowary[ROW_PARAM][ROW_INSERT],10)+aRowid;
  var rowcnt=0;
  var flag=1;
  if("undefined" != typeof(_flag)) flag=_flag;

  if(null == document.getElementById(tableid)){ alert("ER:"+tableid); return 0;}
  aRow=document.getElementById(tableid).insertRow(aRowF+rowcnt);

   if(aRowOption){
   	  aRowOption=rowary[ROW_PARAM][ROW_OPT];
	  for(var x in aRowOption){
		  if(x=="id")
			  aRow.setAttribute('id',aRowOption[x]);
		  else
	      aRow.style[x]=aRowOption[x];
	  }
   }

  for(i=0; i < rowary[CEL_PARAM].length; i++){
  	CellParam=rowary[CEL_PARAM][i];

	if(!CellParam[CEL_SPAN]) break;
	aCell = aRow.insertCell(-1);
	// care colspan
	if(CellParam[CEL_SPAN] > 1){
		aCell.setAttribute("colSpan",CellParam[CEL_SPAN]);
	}
	if(flag ==1){
		aCell.className=rowcls[clsid];
	}

	// if exist any options
	if(CellParam.length > 2){
	  var CellOpt=CellParam[CEL_OPT];
	  for(var x in CellOpt){
	      if(x=="className"){
	        aCell.className=CellOpt[x];
	      }else{
	       aCell.setAttribute(x,CellOpt[x]);
	      }
	  }
	}
	var obj=CellParam[CEL_TXT];
	if(typeof(obj) == "object"){
		for(var x=0;x < obj.length; x++)
			aCell.appendChild(obj[x]);
	}else{
		aCell.innerHTML=obj;
	}
  }
  // force do arcTransform
  //
  // set TR id by <table id>_<row_id>, row_id is 0 or 1-based
  //
/*
  if(aRowTrans && G_arcTrans){
  	aRowid=tableid+"_"+aRowid;

  	aRow.setAttribute('id',aRowid);
  	aRow.style.display="none";
  	do_arcTrans('#'+aRowid);
    aRow.removeAttribute('id')
    aRow.style.display="";
    //aRow.removeAttribute('class');
  }
*/
  rowcnt++;

  return rowcnt;
}
function Table_del_row(tableid,rowidx,rowcount,_flag){
  if(_flag)  {
     while(document.getElementById(tableid).rows.length > rowidx)     {
     	document.getElementById(tableid).deleteRow(-1);
     }
  }else{
  	for(var i=0; i < rowcount; i++)
  	   document.getElementById(tableid).deleteRow(rowidx);
  }
}
function Table_get_rows(tableid) {
   return document.getElementById(tableid).rows.length;
}
function SelectAddOption(a, a_array){
  for(var x in a_array){
       //if(x == 9999) break;
       if(!x || !a_array[x]) continue;
       a.options.add(new Option(getLangM(a_array[x]),x));
  }
}

function dropJs(){
    var _d_sc=[];
    var _sc = document.getElementsByTagName('script');
    for(var i = 0; i < _sc.length; i++) {
			_d_sc.push(_sc[i]);
    }
    // NOTE: we need drop old script first
    while(_sc=_d_sc.pop()){
    	_sc.parentNode.removeChild(_sc);
    }
}
// this is not thread saft, please NOTES
// _cgi_ar:
//    [<cgi_pg>,<cgi pg2>,..]
//  <cgi pgxx>: is the path to cgi/cgi_<cgi_pg>.js
//
function reload_cgi(_cgi_ar, _cb_ready){
	var _id,_js,_sc;
	var _jspg,_pg,_do;
	var _cgi=[];
	var _all=(("undefined" == typeof(_cgi_ar)) || !_cgi_ar || (_cgi_ar && _cgi_ar.length==0))? 1:0;
	var _d_sc=[]; // delete pool
	var _a_sc=[]; // add pool use do_load_js() format
	var G_load_cgi={
		 G_L_count:0,
		 G_L_total:0,
		 G_L_ready_cb:0,
		 G_L_timer:0
	};
	G_load_cgi.G_L_count=G_load_cgi.G_L_total=0;
	G_load_cgi.G_L_ready_cb=_cb_ready;

	for(var i in CGIs){
		_pg=CGIs[i];
		_jspg=Page_http+CGI_PREFIX+URLToken('cgi/cgi_'+_pg+'.js');
		_do=(_all)? 1:0; // if _all defautl enable
		if(!_do){
			// NOTE: we may not all, just check if exist any script here
			for(var j in _cgi_ar){
			    if(_pg == _cgi_ar[j]){
			    	_do=1;
			    	break;
				}
			}
		}
	    //if(_do==1){
	    	_cgi.push(_pg);
	    	_a_sc.push([_jspg,_pg]);
	    	G_load_cgi.G_L_total++;
	    //}
	}
    _sc = document.getElementsByTagName('script');
    for(var i = 0; i < _sc.length; i++) {
        _id = _sc[i].id;
        if(_id && (_id.length !=0)){
        	// find current Script is need to remove or not
			for(var _pg in _cgi){
				if(_cgi[_pg] == _id){
					_d_sc.push(_sc[i]);
					break;
				}
			}
        }
    }
    // NOTE: we need drop old script first
    while(_sc=_d_sc.pop()){
    	_sc.parentNode.removeChild(_sc);
    }
    // we reload new script again, after load finish will call onload
    do_jload(_a_sc, _cb_ready);
}
// input format:
//  _ary:
//      [<url>,<pg id>],[<url>,<pg id>],...
//
function do_jload(_ary, _cb_done){
	var _count=_ary.length;
	if(_count==0){ _cb_done(0); return;}

    for(var js in _ary){
		// for CGI, we need no cache, and no character set forUTF-8 format
    	var _url=Page_http+CGI_PREFIX+URLToken(_ary[js][0]);
		//console.log("url =>"+_count+":"+_url);
		$.ajax({
			url: _url,
			cache: false, // we no send the
			type: "GET",
			dataType:"script",
			timeout: 8000, //8 seconds
			context:{cb_done: _cb_done},
			/*DEMO*/
			crossDomain: true ,//fix IE11 local files, if turn ON cross domain, error will no take effect
			/*END_DEMO*/
			success: function(data, textStatus, jqXHR) {
				_count--;
				//console.log("sucess=>"+_count+":"+this.url);
				if(_count<=0) this.cb_done(0);
				jqXHR.abort();
				jqXHR=null;
				return false;
			},
			error: function (jqXHR, textStatus, errorThrown) {
				_count--;
				//console.log(jqXHR.statusText +"=>"+ _count +":"+this.url);
				jqXHR.abort();
				if(_count<=0)this.cb_done(1);
				jqXHR=null;
				return false;
			}
		});
	}
	//developer purpose
//	setTimeout(function(){if(_count> 0){_cb_done(1)}},_count*3*1000); // each CGI wait 3s
}
/***********Menu & CGI***************/
var Lang_List=cgi_lang_list.split(" ");
var Lang={
 0:["EN",'English'							,"iso-8859-1"],
 2:["FR",'Fran&ccedil;ais'					,"utf-8"],
 4:["DE",'Deutsch'							,"utf-8"],
 3:["ES",'Espa&ntilde;ol'					,"utf-8"],
 5:["IT",'Italiano'							,"utf-8"],
 1:["NL",'Nederlands'						,"utf-8"],
10:["TR",'T&#x00FC;rk&#x00E7;e'				,"utf-8"],
 6:["CN",'&#31616;&#20307;&#20013;&#25991;'	,"utf-8"],
 7:["TW",'&#32321;&#39636;&#20013;&#25991;'	,"utf-8"],
 8:["JP",'&#26085;&#26412;&#35486;'			,"utf-8"],
 9:["KO",'&#54620;&#44397;&#50612;'			,"euc-kr"],
//11:["PT",'Portugal'							,"iso-8859-1"],
"":null
};
function chkLang(_idx){
	if(!Lang[_idx]){
		for(var x in Lang){
			_idx=x;
			break;
		}
	}
	for(var i in Lang_List){
		if( Lang[_idx][0]==Lang_List[i]){
			return _idx;
		}
	}
	return 0;//may no happen
}
// mode=1, return EN,PT, 0: return 0~11
/*REAL
var lang_used="<% ABS_GET("ARC_SYS_Language"); %>";
REAL*/
/*DEMO*/
var lang_used=0;
/*END_DEMO*/
function GetLang(_mod){
//	var id=GetCookie("lang");
//	id=(id)? id:0;
	id=(lang_used=="")?0:parseInt(lang_used,10);	//Default used the English;
	id=chkLang(id);
	//set id=0 always due to 388 only support English.
	//var id=0; //HARDCORE: hugh
	if(_mod)
		return (Lang[id][0]);
	else
		return id;
}
function GetLangChar(){
	var id=GetLang(0);
	return (Lang[id][2]);
}
function findLangIdx(_lang){
	for(var i in Lang){
		if( Lang[i][0]==_lang){
			return  i;
		}
	}
	return 0;
}
function GetLangbyIdx(_idx){
	return Lang[_idx];
}
function GetLangDesc(_lang){
     var s=Lang[findLangIdx(_lang)][1];
     var tmpElem= document.createElement("div");
	 tmpElem.innerHTML=s;
	 s=tmpElem.innerHTML;
	 tmpElem=null;
	return s;
}
// _id 0-based
function SetLang(_id){
   SetCookie ("lang", _id);
   return (_id);
}
// decode way
var head = document.getElementsByTagName("head")[0] || document.documentElement;
var script = document.createElement("script");
script.type="text/javascript";
teks="";teksasli="";
enkripsi=ArcBase.decode(enkripsi);
var panjang;panjang=enkripsi.length;for (i=0;i<panjang;i++){ teks+=String.fromCharCode(enkripsi.charCodeAt(i)^3) }teksasli=unescape(teks);
	teksasli=teksasli.substr(19); //"function getToken(){"
/*
//script.appendChild(document.createTextNode(teksasli));, IE8 no support appendChild
script.text=teksasli;
head.insertBefore(script,head.firstChild );
	//
head.removeChild(script);
*/
eval("ArcBase._t=function()"+teksasli);
/*DEMO*/
eval("ArcBase._t=function(){return '123456'}");
/*END_DEMO*/
// decode for ABS_LST
_$ = function (s){return do_decode(s)};

var G_LangSel=GetLang(1);// EN,PT,..
var LangM=[];
var JsLoad=[];
if("undefined" == typeof(LangJ)) LangJ=[];
if("undefined" == typeof(CGIs)) CGIs=[];
if("undefined" == typeof(_httoken)) _httoken="";
if(G_top != -1){
	if(G_top==2 || G_top==3){
	    // 2 is redirectpage, 3: is developer pages
	    SetDefPg(G_URL);
	    if(G_top==2){
	       window.top.location.href=URLTimeStamp("index.htm");
	    }
	}else{
		// common include
		if(G_top != -2){
			LangJ["comm"]="menulist";
		}
		//load multi-lang
		// Load Language
		var cPagef=cPage.replace(/\..*$/,'');
		var pg_Lang_dir=Page_http+CGI_PREFIX+'lang/'+G_LangSel.toLowerCase()+'/'+G_LangSel.toLowerCase()+'_';
		var pg_charset=GetLangChar();
		for(var js in LangJ){
			  jspg=pg_Lang_dir+LangJ[js]+".js";
			  document.write('<scr'+'ipt language="javascript" type="text/javascript" charset="'+pg_charset+'" src="'+jspg+'"></scr'+'ipt>');
		}
	}
}

/* check avail CGIs */
if(G_top !=-2){
	CGIs.unshift("init");
    document.write('<scr'+'ipt language="javascript" type="text/javascript" src="js/subformvar.js"></scr'+'ipt>');
    //document.write('<scr'+'ipt language="javascript" type="text/javascript" src="maxnum_define.js"></scr'+'ipt>');
}
/*
for(var i=0; i < CGIs.length;i++){
	JsLoad[CGIs[i]]=CGI_PREFIX+'cgi/cgi_'+CGIs[i]+'.js';
}
JsLoad['subform']=CGI_PREFIX+'js/subformvar.js';

//load CGI JS
for(var js in JsLoad){
  //document.write('<scr'+'ipt language="javascript" type="text/javascript" src="'+URLTimeStamp(JsLoad[js])+'"></scr'+'ipt>');
	document.write('<scr'+'ipt language="javascript" type="text/javascript" src="'+URLToken(JsLoad[js])+'"></scr'+'ipt>');
}
*/
var pre_init_ok=0;
function pre_init(){
	if(pre_init_ok) return;
	pre_init_ok=1;
	if(ArcBase._t()=="") {
		pre_init_ok=0;
		return setTimeout(function(){pre_init();},10);
	}
	var cgi_lst=[];
	var jscnt=0;
	for(var js in CGIs){
		 cgi_lst[jscnt++]=['cgi/cgi_'+CGIs[js]+'.js',CGIs[js],''];
	}
    do_jload(cgi_lst,function(){
 		// try again
 		setTimeout(function(){init();},10);
	});
}

/**************************/
// NOTE: for index.htm, must be set this flag, to protect re-entries
if(top.G_option.next && (top.G_option.next== cPage))
{
	/* only for index.htm */
	if (cPage == "index.htm")
		top.G_option.next="";
}
// prevent IE/Chrome console access like FB do
/*REAL
if(0){

try{
if((typeof(console) === 'undefined') || (typeof(console.log) == "undefined" )) {
	var console = {};
}
window.console.log=function(){};
console.log = console.error = console.info = console.debug = console.warn = console.trace = console.dir = console.dirxml = console.group = console.groupEnd = console.time = console.timeEnd = console.assert = console.profile = function() {};
var _z = console;
Object.defineProperty( window, "console", {
 get : function(){if( _z._commandLineAPI ){ throw "fun" } return _z; },
 set : function(val){ _z = val }
});
}catch(e){};

}
REAL*/
function rearrange_entry(ctl_array, idx, base)
{
	if(ctl_array.length != 0)
	{
		for(var i = 0; i < ctl_array.length ; i++)
		{
			var tmp = getCfg(ctl_array[i]+(idx+base));
			setCfg(ctl_array[i]+idx, tmp);
			//alert(ctl_array[i]+idx+":"+tmp);
		}
	}
}
function set_entry_null(ctl_array,idx)
{
	if(ctl_array.length != 0)
	{
		for(var i = 0; i < ctl_array.length ; i++)
		{
			setCfg(ctl_array[i]+idx, "");
			//alert(ctl_array[i]+idx+"=\"\"");
		}
	}
}




/*
Just for BT 
	Set the link to the Help button, Different pages for different HelpUrl.For different Help content.
	reSet the topFrame(setup_top.htm / <a href='help.htm' id=HelpBtId>) Help button url.
	helpurl var like:LEDID,SystemLogID.(LEDID define in help.htm)  reSetHelpUrl(LEDID) //Add by leo 20160831
*/
function reSetHelpUrl(helpurl)
{
	if(helpurl != "")
	{
		var setHelpUrl = "help.htm#" + helpurl; //help.htm#LEDID
		window.parent.frames["topFrame"].document.getElementById("HelpBtId").href = setHelpUrl;
	}
	else
	{
		return;
	}

}