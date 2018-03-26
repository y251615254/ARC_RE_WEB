var url_address=document.URL;

function ShowLangMenu(lang_idx)
{
	if(top.G_prog !=0) return false;

	if(lang_idx==-1){
		lang_idx=GetLang(0); //1-curr_lang_idx;
	}
	if(top.main2)
		top.main2.ShowMultiLangPanel(lang_idx,0);
	else
		parent.ShowMultiLangPanel(lang_idx, 1); //is login
	return false;
}
function LangHover(obj,flag){
	var cls=(obj.className+" lang_hover").split(" ");
	obj.className=cls[0];
	if(flag)
		obj.className+=" "+cls[1];
}
function ShowMultiLangPanel(lang_idx, islogin)
{

	if($("#multilang_menu").length == 0)
	{
		var str_div = "";
		for(var i in Lang_List){
			var idx=findLangIdx(Lang_List[i]);
			var LANG=GetLangbyIdx(idx);
			//console.log("LANG["+i+"]"+idx+"?"+LANG);
			var cls='';
			if(lang_idx ==idx){
				cls='class="menu_select"';
			}
			cls+=' onmouseover="LangHover(this,1)" onmouseout="LangHover(this,0)"';
			str_div += '<a href="javascript:top.topFrame.Select_LANG('+idx+','+islogin+');" ><div '+cls+'>'+LANG[1]+'</div></a>';
		}
		if(str_div){
			str_div='<div id="multilang_menu">'+str_div + '</div>';
		var oDiv = $(str_div)
			.hide()
			.appendTo($("body"))
			.hover(function(){
				;
			},function(){
				oDiv.hide();
			})
			;
		}
	}
	// 2014/4/28: FF no support $(body)[0].scrollTop but $(document).scrollTop
	//            IE/Chrome also support it, replace it.
	$("#multilang_menu")
		.css("left", $("html").width()-$("#multilang_menu").width()-42)
		.css("top",  $(document).scrollTop() + 2 + ((islogin)? 95:0))
		.toggle();
}
function Select_LANG(_idx, _login){
	SetLang(_idx); // keep cookie
	var next_url=(_login)? "login.htm": "index.htm";
	var httoken_val = ArcBase._t();
	
	$.post("chglang.cgi", {lang:_idx,nexturl:next_url,httoken:httoken_val})
		.complete(function() { top.window.location.reload(true); });
	
//	//TODO:
	//  submit to DUT for langauge changed
//	var f=document.tFF;
//	var next_url=(_login)? "login.htm": "index.htm";
//    f.language_flag.value=1;
//    f.menupage.value=next_url;
	//reload page again
//	f.action=next_url; //"index.htm";
// FIXME: TODO: multi-language not work
//	top.location.href=next_url;
//FIXME: dirty do demo only!!!
//    f.submit();
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
