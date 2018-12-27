
var url_address=document.URL;

function ShowLangMenu(idx)
{
	if (idx == 0)
		top.main1.main2.ShowMultiLangPanel("en");
	else
		top.main1.main2.ShowMultiLangPanel("de");
}

function ShowMultiLangPanel(lang)
{

	if($("#multilang_menu").length == 0)
	{
		var str_div = "<div id='multilang_menu'>";
		if(lang == "en")
			str_div += "<a href='javascript:top.topFrame.lang_EN();'><div class='menu_select'>English</div></a>";
		else
			str_div += "<a href='javascript:top.topFrame.lang_EN();'><div>English</div></a>";

		str_div += "<div class='seperator'></div>";

		if(lang == "de")
			str_div += "<a href='javascript:top.topFrame.lang_DE();'><div class='menu_select'>Deutsch</div></a>";
		else
			str_div += "<a href='javascript:top.topFrame.lang_DE();'><div>Deutsch</div></a>";

		str_div += "</div>";
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
	$("#multilang_menu")
		.css("left", $("html").width()-$("#multilang_menu").width()-42)
		.css("top", $("body")[0].scrollTop + 2)
		.toggle();
}


function lang_DE()
{
    document.tFF.language_flag.value=0;
//    document.tFF.menupage.value=menupage;
	if (url_address.indexOf("auto_upg2.htm",0)!= -1)
		document.tFF.menupage.value="/auto_upg2.htm";
	else if (url_address.indexOf("pin_code.htm",0)!= -1)
		document.tFF.menupage.value="/pin_code.htm";
	else if (url_address.indexOf("redir.htm",0)!= -1)
		document.tFF.menupage.value="/redir.htm"+location.search ;
	else if (url_address.indexOf("redir_.htm",0)!= -1)
		document.tFF.menupage.value="/redir_.htm"+location.search ;
	else
		document.tFF.menupage.value="/index.htm";
    document.tFF.submit();
}

function lang_EN()
{
    document.tFF.language_flag.value=1;
	if (url_address.indexOf("auto_upg2.htm",0)!= -1)
	{
		document.tFF.menupage.value="/auto_upg2.htm";
	}
	else if (url_address.indexOf("pin_code.htm",0)!= -1)
		document.tFF.menupage.value="/pin_code.htm";
	else if (url_address.indexOf("redir.htm",0)!= -1)
		document.tFF.menupage.value="/redir.htm"+location.search ;
	else if (url_address.indexOf("redir_.htm",0)!= -1)
		document.tFF.menupage.value="/redir_.htm"+location.search ;
	else
	{
		document.tFF.menupage.value="/index.htm";
    }
    document.tFF.submit();
}
function dw(message){ 
document.write(message); 
}

function dj(message)
{ 
return message; 
}


function AddButton(caption, link, onclick, class_append, target)
{
	if (caption.length > 1)
	{
		document.write('<div class="button_pane">');
		document.write('	<a class="button'+(class_append?class_append:'')+'" href="'+link+'" onclick="'+onclick+'" target="'+(target?target:'_self')+'">');
		document.write('		<span class="buttonCapLeft">&nbsp;</span>');
		document.write('		<span class="buttonText">'+caption+'</span>');
		document.write('		<span class="buttonCapRight">&nbsp;</span>');
		document.write('	</a>');
		document.write('</div>');
	}
}

function AddButtonStr(caption, link, onclick, class_append, target)
{
	var str = "";
	if (caption.length > 1)
	{
		str += '<div class="button_pane">';
		str += '	<a class="button'+(class_append?class_append:'')+'" href="'+link+'" onclick="'+onclick+'" target="'+(target?target:'_self')+'">';
		str += '		<span class="buttonCapLeft">&nbsp;</span>';
		str += '		<span class="buttonText">'+caption+'</span>';
		str += '		<span class="buttonCapRight">&nbsp;</span>';
		str += '	</a>';
		str += '</div>';
	}

	return str;
}

function AddInput(form, name, value)
{
	if(form && name && value)
	{
		if ((navigator.appName == 'Netscape') || (navigator.appName == 'Opera'))
		{
			var inputElement = document.createElement("input");
			inputElement.setAttribute("type", "hidden");
			inputElement.setAttribute("name", name);
			inputElement.setAttribute("value", value);
			form.appendChild(inputElement);
		}
		else
		{
			var inputElement = document.createElement("<input type='hidden' name='"+name+"' value='"+value+"'>");
			form.appendChild(inputElement);
		}
	}
}

function AddInputNameInButton(e, name)
{
	var form = $(e).parents("form");
	if(form)
	{
		AddInput(form[0], name, "1");
		form.submit();
	}
}

function str_add_bracket( text )
{
	/*
    //var prefix = "<font color=#848284>&nbsp;(" ;
    //var prefix = "<font color=#a0a0a0 size=1>&nbsp;(" ;
    //var posfix = ")</font>" ;
    var prefix = "<div class='default_value'>&nbsp;(" ;
    var posfix = ")</div>" ;
	if ( text==null || text=='' )
		text = ' ' ;
	return prefix + text + posfix ;
	*/
	return "";
}

function cancel_button()
{
	window.location.href=url_address;
}

