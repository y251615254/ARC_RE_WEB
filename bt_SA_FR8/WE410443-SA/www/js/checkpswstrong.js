//Password strength detection function
//20170523..leo

/*
A. Number
B. letter  
C. Uppercase 
D. Lowercase 
E. Special character 


Category	Criteria
Invalid		<5 characters or >20 characters
			5-20 characters, Only A, B , C or D
			5-20 characters, Contains illegal words [Appendix C] or characters [Appendix B]

Weak		5-7 characters, 2 of A,B,C,D
			5-20 characters, 2 of A,B,C,D, Contains triple repeat characters (e.g. aaa or BBB)

OK			5-7 characters, 3 of A,B,C,D
			8-20 characters, 2 of A,B,C,D

Good		5-7 characters, A+B+C+D
			8-20 characters, 3 of A,B,C,D

Strong		8-20 characters, A+B+C+D
*/

/*******************************check psw strong start****************************************/
/*
//The function has bug
function checkstrong_multiple_all_choice(psw) //4 of (BC, BD, A, E) 
{

	//BCBDAE
	var bcbdae = /^[a-zA-Z0-9]+$/; //upper. lower. number .Special character 
	if(bcbdae.test(psw))
	{
		//alert(psw+ "@ OK BCBDAE ");
		if(checkstrong_is_special_characterE(psw))
		{		
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		//alert(psw+ "@ No BCBDAE");
		return 0;
	}
}

*/


function checkstrong_is_numberA(psw) //A. Number
{
		

	var reg = /^[0-9]+$/;
	//psw="1236a87";
	if(reg.test(psw))
	{
		//alert(psw+ "@ number");
		return 1;
	}
	else
	{
		//alert(psw+ "@ No number");
		return 0;
	}
}

function checkstrong_length(psw, min, max)
{
	if(psw.length < min || psw.length > max)
	{
		//alert(psw + "@" + min + "@"  + max+ "@ 1");
		return 1;
	}
	else
	{
		//alert(psw + "@" + min + "@"  + max+ "@ 0");
		return 0;
	}
}

function checkstrong_length_range(psw, min, max) 
{
	

	if(psw.length <= max && psw.length >= min)
	{
		//alert(psw + min + max+ "@ length");
		return 1;
	}
	else
	{
		//alert(psw + min + max +  "@ No length ");
		return 0;
	}
}


function checkstrong_BCA(psw) //BC. Uppercase letter  A number
{
	
	var reg = /^[A-Z0-9]+$/;

	if(reg.test(psw))
	{
		return true;
	}
	else
	{
		return false;
	}
}

function checkstrong_BDA(psw) //BD. Lowercase lette  A number
{
	
	var reg = /^[a-z0-9]+$/;
	//alert(psw);
	if(reg.test(psw))
	{
		return true;
	}
	else
	{
		return false;
	}
}


function checkstrong_is_uppercase_letterBC(psw) //BC. Uppercase letter 
{
	//alert("checkstrong_is_uppercase_letterBC")
	var reg = /^[A-Z]+$/;

	if(reg.test(psw))
	{
		//alert(psw+ "@ Uppercase letter");
		return 1;
	}
	else
	{
		//alert(psw+ "@ No uppercase lettert");
		return 0;
	}
}

function checkstrong_is_lowercase_letterBD(psw) //BD. Lowercase letter
{
		//alert("checkstrong_is_lowercase_letterBD")

	var reg = /^[a-z]+$/;
	if(reg.test(psw))
	{
		//alert(psw+ "@ Lowercase letter");
		return 1;
	}
	else
	{
		//alert(psw+ "@ No lowercase lettert");
		return 0;
	}
}

function checkstrong_is_special_characterE(psw) // E. Special character 
{
	//var psw="abcde";
	for(var i=0; i < psw.length; i++)
	{
		var tmp_char=psw.charCodeAt(i);
		if(tmp_char < 32 || tmp_char > 126)
		{
			//alert("No .is allow special char");
			return 0;
		}
		else// 32<= tmp <=126
		{
			//alert("Ok .is special char");
			return 1;
		}
	}
}


function checkstrong_is_triple_repeat_characters(psw) //triple repeat characters
{
		

	//var reg = /([a-zA-Z0-9~`!@#$%^&*;:|\\<>,.?/-_])\1{2}/;
	var reg = /([\s\S])\1{2}/;
	//psw="1236a87";
	if(reg.test(psw))
	{
		//alert(psw+ "@ OK triple_repeat_characters");
		return 1;
	}
	else
	{
		//alert(psw+ "@ No triple_repeat_characters");
		return 0;
	}
}


//The function has bug
function checkstrong_multiple_triple_choice(psw) //3 of (BC, BD, A, E) //BCBDA,BCBDE,BCAE,BDAE
{

	//BCBDA
	var bcbda = /^[a-zA-Z0-9]+$/; //upper lower number
	if(bcbda.test(psw))
	{
		//alert(psw+ "@ OK bcbda ");
		return 1;
	}
	else
	{
		//alert(psw+ "@ No bcbda");
		return 0;
	}

	//BCBDE
	if(checkstrong_is_special_characterE(psw))
	{
		var bcbde = /^[a-zA-Z0-9]+$/; //upper lower special
		if(bcbde.test(psw))
		{
			//alert(psw+ "@ OK BCBDE ");
			return 1;

		}
		else
		{
			//alert(psw+ "@ No BCBDE");
			return 0;
		}

	}
	
	var bcbde = /^[a-zA-Z0-9]+$/; //upper lower special
	if(bcbde.test(psw))
	{
		//alert(psw+ "@ OK BCBDE ");
		if(checkstrong_is_special_characterE(psw))
		{		
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		//alert(psw+ "@ No BCBDE");
		return 0;
	}

	//BCAE
	var bcae = /^[A-Z0-9]+$/; //upper special number
	if(bcbde.test(psw))
	{
		//alert(psw+ "@ OK BCAE ");
		if(checkstrong_is_special_characterE(psw))
		{		
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		//alert(psw+ "@ NO BCAE");
		return 0;
	}

	//BDAE
	var bcae = /^[a-z0-9]+$/; //Lowercase letter . Number . Special character 
	if(bcbde.test(psw))
	{
		//alert(psw+ "@ OK BDAE ");
		if(checkstrong_is_special_characterE(psw))
		{		
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		//alert(psw+ "@ NO BDAE");
		return 0;
	}


}




function checkstrong_character_1(pwd)
{
	if(checkstrong_character(pwd) == 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}



function checkstrong_character_2(pwd)
{
	if(checkstrong_character(pwd) == 2)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}


function checkstrong_character_3(pwd)
{
	if(checkstrong_character(pwd) == 3)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}

function checkstrong_character_4(pwd)
{
	if(checkstrong_character(pwd) == 4)
	{
		return 1;
	}
	else
	{
		return 0;
	}

}



function checkstrong_illegal_character(pwd)
{
	var illegal_flag=0;// default is not  illegal_character
	for(var i=0; i < pwd.length; i++)
	{
		var tmp_char=pwd.charCodeAt(i);

		if( tmp_char < 32 || tmp_char > 126)  
		{
			illegal_flag = 1;// has illegal_character
		}

	}

	//alert(illegal_flag);
	return illegal_flag;
}


function checkstrong_character(pwd)
{
	var lower_flag=0;
	var upper_flag=0;
	var number_flag=0;
	var special_flag=0;

	
	for(var i=0; i < pwd.length; i++)
	{
		var tmp_char=pwd.charCodeAt(i);

		if(48 <= tmp_char && tmp_char <= 57)  //48 <= t <= 57 (0 <= t <=9)
		{
			number_flag = 1;
		}

		if(65 <= tmp_char && tmp_char <= 90)  //65 <= t <= 90 (A <= t <=Z)
		{
			upper_flag = 1;
		}

		if( 97 <= tmp_char && tmp_char <= 122 )  //97 <= t <= 122 (a <= t <=z)
		{
			lower_flag = 1;
		}



		 //32 <= t <= 47   ( space!"#$%&'()*+,-./ )   
		 //58 <= t <= 64   (  :;<=>?@  )
		 //91 <= t <= 96   ( [\]^_` )
		 //123 <= t <= 126 ( {|}~ )
		
		if( (32 <= tmp_char && tmp_char <= 47) || (58 <= tmp_char && tmp_char <= 64) || (91 <= tmp_char && tmp_char <= 96) || (123 <= tmp_char && tmp_char <= 126) ) 
		{
			special_flag = 1;
		}
	}

	var tmp = lower_flag + upper_flag + number_flag + special_flag;

	//alert(tmp);
	if(tmp == 3)
	{
		//alert("allow special char");
		return tmp;
	}
	else if(tmp == 4)
	{
		//alert("allow special char");
		return tmp;
	}
	else
	{
		return tmp;
	}
}

function checkstrong_contains_illegal_words_appendix_c(psw)
{
	var flag=0;
	//psw = "abcd1234";
	var appendix_c_array = new Array();
	appendix_c_array.push("btwholehome", "btwholehome1", "wholehome", "wholehome1","wholehomewifi", "wholehomewifi1");
	appendix_c_array.push("whw123", "btwifi", "btwifi1", "password","password1", "Password");
	appendix_c_array.push("passwd", "p@ssw0rd", "pass word", "admin","admin123", "adminpassword");
	appendix_c_array.push("abc123", "abcd1234", "letmein", "logmein","welcome1", "qwerty");
	appendix_c_array.push("qwertyui", "1234567890", "0987654321");
	appendix_c_array.push("987654321", "qwerty");


	for(var i=0; i < appendix_c_array.length; i++)
	{
		var tmp_words=appendix_c_array[i];
		if(psw == tmp_words)
		{
			//alert("contains illegal_words");
			flag = 1;
			break;
			//return 1;
		}
		else
		{
			//alert("no contains illegal_words");
			//return 0;
		}
	}

	return flag;
}



function checkstrong_contains_illegal_words_appendix_a(psw)
{

	var appendix_a_flag=0;

	for(var i=0; i < psw.length; i++)
	{
		var tmp_charxx=psw.charAt(i);
		var tmp_char=psw.charCodeAt(i);
		if(tmp_char < 32 || tmp_char > 126)
		{
			appendix_a_flag = 1;
			//alert("contains illegal char");
			break;
		}
		else// 32<= tmp <=126
		{
			//alert("no contains special char");	
		}
	}

	return appendix_a_flag;
}

function checkstrong_contains_illegal_words_appendix_b(psw)
{

	var appendix_b_flag=0;

	for(var i=0; i < psw.length; i++)
	{
		var tmp_charxx=psw.charAt(i);
		var tmp_char=psw.charCodeAt(i);
		if(tmp_char < 33 || tmp_char > 126)
		{
			appendix_b_flag = 1;
			//alert("contains illegal char");
			break;
		}
		else// 33<= tmp <=126
		{
			//alert("no contains special char");	
		}
	}

	return appendix_b_flag;
}


function checkstrong_invalid(sPW)
{
	/*
		<8 characters or >63 characters
		8-63 characters, Only A, B , C or D
		8-63 characters, Contains illegal words [Appendix C] or characters [Appendix A]
	*/
	var invalid_value = 0;
	if(checkstrong_length(sPW, 8, 63))
		invalid_value = 1;
	if(invalid_value != 1)//spw must be 8<pwd<63
	{
		if(checkstrong_character_1(sPW))
			invalid_value = 2;
		if(checkstrong_contains_illegal_words_appendix_a(sPW))
			invalid_value = 3;
		if(checkstrong_contains_illegal_words_appendix_c(sPW))
			invalid_value = 4;
	}

	return invalid_value;
	/*
	if (checkstrong_length(sPW, 8, 63) 
		|| (checkstrong_length_range(sPW, 8, 63) && checkstrong_character_1(sPW) ) 
		|| (checkstrong_length_range(sPW, 8, 63) &&  (checkstrong_contains_illegal_words_appendix_a(sPW) || checkstrong_contains_illegal_words_appendix_c(sPW)) ))
	{
		return 1; 
	}
	else
	{
		return 0; 
	}
	*/
}

function checkstrong_invalid_password(sPW)
{
	/*
		<5 characters or >20 characters  
		5-20 characters, Only A, B , C or D
		5-20 characters, Contains illegal words [Appendix C] or characters [Appendix B]
	*/
	var invalid_value = 0;
	if(checkstrong_length(sPW, 5, 20))
		invalid_value = 1;
	if(invalid_value != 1)//spw must be 5<pwd<20
	{
		if(checkstrong_character_1(sPW))//5-20 characters, Only A, B , C or D
			invalid_value = 2;
		if(checkstrong_contains_illegal_words_appendix_b(sPW))
			invalid_value = 3;
		if(checkstrong_contains_illegal_words_appendix_c(sPW))
			invalid_value = 4;
	}

	return invalid_value;
}


function checkstrong_weak(sPW)
{
	/*
	Week:
	8-11 characters, 2 of A,B,C,D
	12-63 characters, 2 of A,B,C,D, Contains triple repeat characters (e.g. aaa or BBB)
	*/	

    if ( 	
		 ( checkstrong_length_range(sPW, 8, 11)&&checkstrong_character_2(sPW)) ||
		 ( checkstrong_length_range(sPW, 8, 63) && checkstrong_is_triple_repeat_characters(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}

}

function checkstrong_weak_password(sPW)
{
	/*
	Week:
	5-7 characters, 2 of A,B,C,D
	5-20 characters, 2 of A,B,C,D, Contains triple repeat characters (e.g. aaa or 
	*/	

    if ( 	
		 ( checkstrong_length_range(sPW, 5, 7)&&checkstrong_character_2(sPW)) ||
		 ( checkstrong_length_range(sPW, 5, 20)&& checkstrong_is_triple_repeat_characters(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}

}


function checkstrong_ok(sPW)
{
	/*
	OK:
	8-11 characters, 3 of A,B,C,D
	12-63 characters, 2 of A,B,C,D
	*/
    if ( 	
		 ( checkstrong_length_range(sPW, 8, 11)&&checkstrong_character_3(sPW)) ||
		 ( checkstrong_length_range(sPW, 12, 63)&&checkstrong_character_2(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}
}


function checkstrong_ok_password(sPW)
{
	/*
	OK:
		5-7 characters, 3 of A,B,C,D
		8-20 characters, 2 of A,B,C,D

	*/
    if ( 	
		 ( checkstrong_length_range(sPW, 5, 7)&&checkstrong_character_3(sPW)) ||
		 ( checkstrong_length_range(sPW, 8, 20)&&checkstrong_character_2(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}
}



function checkstrong_good(sPW)
{
	/*
	good
	8-11 characters, A+B+C+D
	12-63 characters, 3 of A,B,C,D
	*/
    if ( 	
		 ( checkstrong_length_range(sPW, 8, 11)&&checkstrong_character_4(sPW)) ||
		 ( checkstrong_length_range(sPW, 12, 63)&&checkstrong_character_3(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}
}

function checkstrong_good_password(sPW)
{
	/*
	good
	5-7 characters, A+B+C+D
	8-20 characters, 3 of A,B,C,D

	*/
    if ( 	
		 ( checkstrong_length_range(sPW, 5, 7)&&checkstrong_character_4(sPW)) ||
		 ( checkstrong_length_range(sPW, 8, 20)&&checkstrong_character_3(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}
}




function checkstrong_strong(sPW)
{
	/*
		Strong:
		12-63 characters, A+B+C+D
	*/
    if ( 	
		 ( checkstrong_length_range(sPW, 12, 63)&&checkstrong_character_4(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}
}

function checkstrong_strong_password(sPW)
{
	/*
		Strong:
			8-20 characters, A+B+C+D	
	*/
    if ( 	
		 ( checkstrong_length_range(sPW, 8, 20)&&checkstrong_character_4(sPW) )
	   )
	{
        return 1; 
	}
	else
	{
        return 0; 
	}
}


//返回强度级别   
var invalid_glb_flag = 0;
//Wi-Fi Settings->Security Key
function check_wifi_psw_strong(sPW)
{    
	var invalid_value = 0;
	if(sPW == "")
	{
		invalid_glb_flag = 1;
		return 0;//Dfault_color
	}
	else if ((invalid_value = checkstrong_invalid(sPW)) > 0)
	{
		invalid_glb_flag = invalid_value;
		return 1; 
	}
	else if( checkstrong_weak(sPW) )
	{
		invalid_glb_flag = 0;
		return 2; 
	}
	else if(checkstrong_ok(sPW))
	{
		invalid_glb_flag = 0;
		return 3; 
	}
	else if(checkstrong_good(sPW))
	{
		invalid_glb_flag = 0;
		return 4; 
	}
	else if(checkstrong_strong(sPW))
	{
		invalid_glb_flag = 0;
		return 5; 
	}
	else
	{
		invalid_glb_flag = 1;
		return 0;//Dfault_color
	}
}

//Admin Password->Password
function checkStrongPassword(sPW)
{    
	var invalid_value = 0;
	if(sPW == "")
	{
		invalid_glb_flag = 1;
		return 0;//Dfault_color
	}
	else if((invalid_value = checkstrong_invalid_password(sPW)) > 0)
   	{
		invalid_glb_flag = invalid_value;
		return 1; 
	}
	else if(checkstrong_weak_password(sPW) )
	{
		invalid_glb_flag = 0;
		return 2; 
	}
	else if(checkstrong_ok_password(sPW))
	{
		invalid_glb_flag = 0;
		return 3; 
	}
	else if(checkstrong_good_password(sPW))
	{
		invalid_glb_flag = 0;
		return 4; 
	}
	else if(checkstrong_strong_password(sPW))
	{
		invalid_glb_flag = 0;
		return 5; 
	}
	else
	{
		invalid_glb_flag = 1;
		return 0;//Dfault_color
	}
}



//返回强度级别    //has bug
function checkStrong_bck(sPW)
{    

	if(sPW == "")
	{
		return 0;//Dfault_color
	}

	/*
	Invalid:
	< 8 or > 63
	Only BC
	Only BD
	Only A
	Contains illegal key word
	*/
	//alert(sPW);
    if (checkstrong_length(sPW, 8, 63) || checkstrong_is_uppercase_letterBC(sPW) || checkstrong_is_lowercase_letterBD(sPW) || checkstrong_is_numberA(sPW) || checkstrong_illegal_character(sPW) )
	{
		//alert(sPW + "@  Invalid 1");
        return 1; 
	}


	/*
	Week:
	8 - 11 and BC + A
	8 - 11 and BD + A
	8  - 63 and contain triple repeat characters
	*/

    if ( 	
		 ( checkstrong_length_range(sPW, 8, 11)&&checkstrong_BCA(sPW)) ||
		 ( checkstrong_length_range(sPW, 8, 11)&&checkstrong_BDA(sPW)) ||
		 ( checkstrong_length_range(sPW, 8, 63)&& checkstrong_is_triple_repeat_characters(sPW) ) 
	   )
	{
		//alert(sPW + "@  Week 2");
        return 2; 
	}


	/*
	OK:
	8 - 11 and 3 of (BC, BD, A, or E) and no triple repeat characters
	*/
    if ( 	
		  ( checkstrong_length_range(sPW, 8, 11) && 
		    checkstrong_character_3(sPW) && 
		    !checkstrong_is_triple_repeat_characters(sPW))
	    )
	{
		//alert(sPW + "@  OK 3");
        return 3; 
	}


	/*
	Good:
	8 - 11 and BC + BD + A + E and no triple repeat characters
	12 - 63 and 3 of (BC, BD, A, or E) and no triple repeat characters
	*/
	if( 	
		( checkstrong_length_range(sPW, 8, 11) && 
		  checkstrong_character_4(sPW) && 
		  (!checkstrong_is_triple_repeat_characters(sPW)) 
		) ||
		(  checkstrong_length_range(sPW, 12, 63) && 
		   checkstrong_character_3(sPW) && 
		   (!checkstrong_is_triple_repeat_characters(sPW))
		))
	{
		//alert(sPW + "@  Good 4");
		return 4; 
	}

	/*
		Strong:
		12 - 63 and BC + BD + A + E and no triple repeat characters
	*/
	if( 	
		(  checkstrong_length_range(sPW, 12, 63) && 
		   checkstrong_character_4(sPW) && 
		   (!checkstrong_is_triple_repeat_characters(sPW))
		)
      )
	{
		//alert(sPW + "@  Strong 5");
		return 5; 
	}

	return 0;
}

/*******************************check psw strong end****************************************/
	
 
