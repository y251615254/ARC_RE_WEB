/*
 *
 * arcTransform
 * come from jqtransform:
 * by mathieu vilaplana mvilaplana@dfc-e.com
 * Designer ghyslain armand garmand@dfc-e.com
 *
 *
 * Version 1.0 25.09.08
 * Version 1.1 06.08.09
 * Add event click on Checkbox and Radio
 * Auto calculate the size of a select element
 * Can now, disabled the elements
 * Correct bug in ff if click on select (overflow=hidden)
 * No need any more preloading !!
 *
 ******************************************** */

(function($){
	var defaultOptions = {preloadImg:true};
	var arcTransformImgPreloaded = false;

	var arcTransformImgZindex = 100;

	var arcTransformPreloadHoverFocusImg = function(strImgUrl) {
		//guillemets to remove for ie
		strImgUrl = strImgUrl.replace(/^url\((.*)\)/,'$1').replace(/^\"(.*)\"$/,'$1');
		var imgHover = new Image();
		imgHover.src = strImgUrl.replace(/\.([a-zA-Z]*)$/,'-hover.$1');
		var imgFocus = new Image();
		imgFocus.src = strImgUrl.replace(/\.([a-zA-Z]*)$/,'-focus.$1');
	};

	isSafari = function() {
	  return /Safari/.test(navigator.userAgent) && /Apple Computer/.test(navigator.vendor);
	};
	isChrome = function() {
	  return /Chrome/.test(navigator.userAgent) && /Google Inc/.test(navigator.vendor);
	};
	isIE = function() {
	  return /MSIE/.test(navigator.userAgent) || /Trident/.test(navigator.userAgent);
	};
	isFF =function(){
	  return /Mozilla/.test(navigator.userAgent) && /Firefox/.test(navigator.userAgent);
	};
	var arcTransformMakeTextNodeFloat = function(field) {
		var textnodes = field.parent().contents().filter(function() { return this.nodeType == 3;});
		for(var i=0;i<textnodes.length;i++)
		{
			/*if(document.all)
				alert(textnodes[i].innerText);
			else
				alert(textnodes[i].textContent);
			*/
			var t = $.trim(textnodes[i].data);
			if(t != "") {$(textnodes[i]).wrap("<div class='float_left_text'></div>");}
		}
	}

	var isIEVer = function(){
		//console.log("==>"+$.browser.version);
		//IE11: Mozilla/5.0 (Windows NT 6.3; Trident/7.0; rv 11.0) like Gecko
		//var isIE=$.browser.msie; //jQuery bug when IE11 comming 2013/12/12,hugh
		//if(!isIE){
		//	if(navigator.userAgent.indexOf("Trident")!=-1){ //this term only > IE9, old is MSIE
		//		isIE=true;
		//	}
		//}
		uaMatch = function( ua ) {
			ua = ua.toLowerCase();

			var match = /(chrome)[ \/]([\w.]+)/.exec( ua ) ||
				/(webkit)[ \/]([\w.]+)/.exec( ua ) ||
				/(opera)(?:.*version|)[ \/]([\w.]+)/.exec( ua ) ||
				/(msie) ([\w.]+)/.exec( ua ) ||
				ua.indexOf("compatible") < 0 && /(mozilla)(?:.*? rv:([\w.]+)|)/.exec( ua ) ||
				[];

			return {
				browser: match[ 1 ] || "",
				version: match[ 2 ] || "0"
			};
		};
		if (isIE()){
			//return (parseInt($.browser.version, 10));
			var a=uaMatch(navigator.userAgent);
			return parseInt(a.version,10);
		}
		return 0;
	}

	$.fn.getElementAttr = function(){
		var $input=$(this);
		var org={'type':'','name':'','class':'','value':'','disabled':'','id':'','size':'','maxlength':'','onchange':'','onclick':'','onfocus':'','onblur':''};
		//console.log("==>"+$.browser.version);
		for(var x in org){
			if($input.attr(x)){
				org[x]=$input.attr(x);
				//console.log(x+"="+$input.attr(x));
			}

		}

		// IE7 bug, when do loop check attributes[] no found it, only under HTML attribute, not at DOM
		if( isIEVer()== 7){
			org['class']=$input[0].className;
		}

		if($input.attr('disabled') == "disabled"){
			//console.log(org['name']+"=disabled");
			org['disabled']='disabled';
		}
		// jquery'si issue, value can not be retrieved by type, it always "submit"
		if(org['value']==""){
			if( $input[0].tagName == "BUTTON"){
				org['value']=$input.html();
			}else{
				org['value']=$input[0].value;
			}

		}

		return org;
	}
	$.fn.arcTransWidth =function(method){
			/*Copyright 2012, Ben Lin (http://dreamerslab.com/)*/
			if("undefined" == typeof(method)) method='width';

			var $target = this.eq( 0 );
			var fix, restore;
			var tmp   = [];
			var style = '';
			var $hidden;
			fix = function (){
			  var style = 'position: absolute !important; top: -1000 !important; ';

			  // this is useful with css3pie
			  $target = $target.
				clone().
				attr( 'style', style ).
				appendTo( 'body' );
			};

			restore = function (){
			  // remove DOM element after getting the width
			  $target.remove();
			};
			fix();
			// get the actual value with user specific methed
			// it can be 'width', 'height', 'outerWidth', 'innerWidth'... etc
			// configs.includeMargin only works for 'outerWidth' and 'outerHeight'
			//var actual = /(outer)/.test( method ) ? $target[method]() : $target[method]();
			var actual = $target[method]();

		  restore();
		  // IMPORTANT, this plugin only return the value of the first element
		  return actual;
	}
	/***************************
	  Labels
	***************************/
	var arcTransformGetLabel = function(objfield){
		var selfForm = $(objfield.get(0).form);
		var oLabel = objfield.next();
		if(!oLabel.is('label')) {
			oLabel = objfield.prev();
			if(oLabel.is('label')){
				var inputname = objfield.attr('id');
				if(inputname){
					oLabel = selfForm.find('label[for="'+inputname+'"]');
				}
			}
		}
		if(oLabel.is('label')){return oLabel.css('cursor','pointer');}
		return false;
	};

	/* Hide all open selects */
	var arcTransformHideSelect = function(oTarget){
		var ulVisible = $('.arcTransformSelectWrapper ul:visible');
		ulVisible.each(function(){
			var oSelect = $(this).parents(".arcTransformSelectWrapper:first").find("select").get(0);
			//do not hide if click on the label object associated to the select
			if( !(oTarget && oSelect.oLabel && oSelect.oLabel.get(0) == oTarget.get(0)) ){
				$(this).hide();
			}
		});
	};
	/* Check for an external click */
	var arcTransformCheckExternalClick = function(event) {
		if ($(event.target).parents('.arcTransformSelectWrapper').length === 0) { arcTransformHideSelect($(event.target)); }
	};

	/* Apply document listener */
	var init_arcTransformAddDocumentListener=1;
	var arcTransformAddDocumentListener = function (){
		if(init_arcTransformAddDocumentListener){
			init_arcTransformAddDocumentListener=0;
			$(document).mousedown(arcTransformCheckExternalClick);
		}
	};
	/* Add a new handler for the reset action */
	var arcTransformReset = function(f){
		var sel;
		$('.arcTransformSelectWrapper select', f).each(function(){sel = (this.selectedIndex<0) ? 0 : this.selectedIndex; $('ul', $(this).parent()).each(function(){$('a:eq('+ sel +')', this).click();});});
		$('a.arcTransformCheckbox, a.arcTransformRadio', f).removeClass('arcTransformChecked');
		$('input:checkbox, input:radio', f).each(function(){if(this.checked){$('a', $(this).parent()).addClass('arcTransformChecked');}});
	};
	/***************************
	  Buttons
	 ***************************/
	$.fn.arcTransCreateInputButton=function(option){
		var opt = $.extend({},defaultOptions,option);
		var $input=$(this);
		var inputSize=$input.arcTransWidth(); //$input.width(); //more space, some language will incorrect
		var midcss=wrapcss=wdcss=divcss="";
		var org=$input.getElementAttr();

		//IE7:+10, IE8: +15px(1.5), IE9/Chrmoe: 10(1.2)
		midcss='style="height:33px;width:'+Math.ceil(inputSize*1.5)+'px;"'; //IE7:10, IE8: 15px, IE9/Chrmoe: 10
		var htm='';
		var mouseevent=' onmousedown="$(this).arcTransButtonHover(0)" '+
		 	 		   ' onmouseup="$(this).arcTransButtonHover(1)" '+
			 		   ' onmouseover="$(this).arcTransButtonHover(2)" '+
			 		   ' onmouseout="$(this).arcTransButtonHover(3)" ';

		wrapcss=org['class'];

		//if($input.attr('disabled') == "disabled"){
		if(org['disabled']){
			wrapcss+=' ButtonDisable ';
		}
		//console.log("is diabled? "+$input.attr('disabled')+"?"+org['disabled']);
		wdcss+=' id="'+ org['name']+'" onClick="return $(this).arcTransInputButtonClick()" func="'+org['onclick']+'" '+
			   (org['disabled']? 'disabled="disabled" ': ' ');//FIXME: we need the class ??
/*

<div class="arctransformdone arcTransformButton">
	<a onclick="return check_user_num();" class="button" href="javascript:return void(0)" onmouseover="Btn_hover(this,0)" onmouseout="Btn_out(this,1)" />>
		<span class=buttonCapLeft></span>
		<span class=buttonText>Add User</span>
		<span class=buttonCapRight></span>
	</a>
</div>
*/
		htm+='<div id="btn_'+ org['name']+'" class="arctransformdone arcTransformButton" style="min-width:'+Math.ceil(inputSize*2)+'px;">'+
			  '<a '+ wdcss+ ' href="javascript:void(0)" >'+
			  	'<span class="ButtonLeft '+wrapcss+'" '+mouseevent+'></span>'+
			  	'<span class="ButtonMiddle '+wrapcss+'" '+midcss+' '+mouseevent+'>'+org['value']+'</span>'+
				'<span class="ButtonRight '+wrapcss+'" '+mouseevent+'></span>'+
			  '</a></div>';
	    return htm;
	}
	$.fn.arcTransButtonHover = function(flag){
		//var $input = $(this);
		var $input = $(this).parent();
		var isdisable=$input[0].getAttribute('disabled');
		if(!isdisable){ isdisable=$input.attr('disabled');}
		// IE7 return true/false
		if((isdisable=='disabled')|| isdisable) return ;
		//console.log("this:"+flag);
		$('span', $input).each(function(idx){
			//console.log("find:"+$(this)[0].className);
			if(flag==3)
				$(this).removeClass('ButtonHover');
			if(flag==2)
				$(this).addClass('ButtonHover');
		});
	}

	$.fn.arcTransInputButton = function(){
			return this.each(function(){
				if($(this).hasClass('arcTransformHidden')) {return;}
				var $input = $(this);
				var newBtn=$input.arcTransCreateInputButton({txt:true});
				$(this).replaceWith($(newBtn));
			});
	}
	$.fn.arcTransInputButtonClick= function(){
		var isdisable=$(this)[0].getAttribute('disabled');
		if(!isdisable){ isdisable=$(this).attr('disabled');}

		// IE7 return true/false
		if((isdisable=='disabled') || isdisable) return false;

		var func=$(this).attr('func');
		//eval(func);
		$.globalEval(func);
		return false;
	}
	/***************************************
	 * Button (share with arcTransCreateInputButton)
	 ***************************************/
	$.fn.arcTransButton = function(){
			return this.each(function(){
				if($(this).hasClass('arcTransformHidden')) {return;}
				var $input = $(this);
				var newBtn=$input.arcTransCreateInputButton({txt:true,type:'button'}); //arcTransCreateButton({txt:true});
				$(this).replaceWith($(newBtn));
			});
	}
	/***************************
	  Text Fields
	 ***************************/
	$.fn.textNodes = function() {
		var ret = [];
		this.contents().each( function() {
			var fn = arguments.callee;
			if ( this.nodeType == 3)
				ret.push( this );
			else $(this).contents().each(fn);
		});
		return $(ret);
	};

	function getTextNodesIn(node, includeWhitespaceNodes) {
		var textNodes = [], whitespace = /^\s*$/;

		function getTextNodes(node) {
			if (node.nodeType == 3) {
				if (includeWhitespaceNodes || !whitespace.test(node.nodeValue)) {
					textNodes.push(node);
				}
			} else {
				for (var i = 0, len = node.childNodes.length; i < len; ++i) {
					getTextNodes(node.childNodes[i]);
				}
			}
		}

		getTextNodes(node);
		return textNodes;
	}

	/***************************
	  InputText
	 ***************************/
	$.fn.arcTransCreateInputText = function(option){
		var opt = $.extend({},defaultOptions,option);
	/*
		<div class="arcTransformInputWrapper arcTransformSafari" style="width: 40px; ">
			<div class="arcTransformInputInner">
				<div>
					<input type="text" name="MAC1_1" size="3" maxlength="2" value="0A" class="txt1 arctransformdone arcTransformInput" style="width: 56px; ">
				</div>
			</div>
		</div>
	*/
		var $input=$(this);
		// 2013/12/12: use outerWidth may correct for all if use "width" IE has problem size
		var wrapSize=$input.arcTransWidth("outerWidth"); //$input.width();
		var inputSize=Math.ceil(wrapSize*1.2); //FIXME: we dirty extend the inner input text width +10
		//var inputHeight=$input.arcTransWidth("outerWidth")-2; //$input.width();
		var att,txt;
		var wrapcss=css=(isSafari()) ? ' arcTransformSafari':'';
		var inncss='arcTransformInputInner';
		var org=$input.getElementAttr();

		if(org['disabled']){
			//console.log("==>"+$input.attr('disabled'));
			css+=' arcTransformDisabled ';
			inncss='arcTransformInputInnerDisabled';
			wrapcss+=' arcTransformInputWrapperDisabled';
		}
		//2013/12/12 IE seems has aligment issue
		// 2014/4/28: FF also has width too small
		if((isIEVer() !=0 ) || (isFF() !=0)){
			wrapSize=Math.ceil(wrapSize*1.2)+6; //IE7 us 6, IE8:12, FF use 6
		}
		//console.log("wrapSize="+wrapSize+"?"+$input.arcTransWidth("outerWidth")+'?'+$input.width());
		var htm='<span class="arcTransformInputWrapper '+ wrapcss + '" style="width:'+wrapSize+'px;">'+
		          '<div class="arcTransformInputInner '+inncss+'">'+
		             '<div><input ';
		htm+='class="arctransformdone arcTransformInput '+css+'" '+
			 'value="'+ org['value']+'" '+
			 'name="'+org['name']+'" '+
			 'size="'+org['size']+'" '+
			 'maxlength="'+org['maxlength']+'" '+
			 ((org['id'])? 'id="'+org['id']+'" ':"")+
			 'style="width:'+inputSize+'px" '+
			 'type="'+ org['type']+ '" '+
			 ((org['disabled']) ? "disabled ":" "); //FIXME: unknow why??/

	    htm+='onFocus="$(this).arcTransInputHover(0)" '+
	    	 'onBlur="$(this).arcTransInputHover(1)" '+
			 'onmouseover="$(this).arcTransInputHover(2)" '+
			 'onmouseout="$(this).arcTransInputHover(3)" '+
			 ((org['onchange'])? ' onchange="'+org['onchange']+'" ':' ') +
			 ' func_blur="'+org['onblur']+'" '+
			 ' func_focus="'+org['onfocus']+'" ';

	   htm+= '/>'+
			         '</div>'+
		          '</div>'+
		       '</span>';
		return htm;
	}
	$.fn.arcTransInputHover = function(flag){
		var func="";
		var $input = $(this);
		if($input.attr('disabled')){return}
		//console.log("find:"+$(this)[0].className);
		switch(flag){
		case 0: //onFocus
		   func=$input.attr("func_focus");
		   $(this).parent().parent().parent().addClass('arcTransformInputWrapper_focus');
		   if(func){ eval(func)}
		   break;
		case 1: //onBlur
		   func=$input.attr("func_blur");
		   $(this).parent().parent().parent().removeClass('arcTransformInputWrapper_focus');
		   if(func){ eval(func)}
		   break;
		case 2: //onmouseover
		   $(this).parent().parent().parent().addClass('arcTransformInputWrapper_hover');
		   break;
		case 3: //onmouseout
		   $(this).parent().parent().parent().removeClass('arcTransformInputWrapper_hover');
		   break;
		}
	}

	$.fn.arcTransInputText = function(option){
			return this.each(function(){
				var $input = $(this);
				if($input.hasClass('arctransformdone') || !$input.is('input')) {return;}
				var newInput=$input.arcTransCreateInputText({txt:true});
				$(this).replaceWith($(newInput));
			});
	}

	/***************************
	  Check Boxes
	 ***************************/
	$.fn.arcTransformCheckBoxLnkClick = function(){
		var aLink=$(this);
		var $input=aLink.parent().find('input');
		//do nothing if the original input is disabled

		if($input.attr('disabled')){return false;}

		//trigger the envents on the input object
		//$input.click(); // modified by lichen, to fix the trigger bug...
		var ischecked=!aLink.hasClass('arcTransformChecked'); //reverse it!
		//$input.attr('checked',ischecked?'checked':'');
		//console.log("click to "+ischecked+" ! " + $input.attr('checked')+'?'+$input[0].checked);
		//console.log("click to "+ischecked+" ! " + $input.attr('checked')+'?'+$input[0].checked);
		$($input)[0].click(); // modified by lichen, to fix the trigger bug...
		if(isIEVer() <=8){
			$input.trigger('change');
		}
		return false;
	}

	$.fn.arcTransformCheckBoxChange = function(){
		var aLink=$(this).parent().find('a');
		var ischecked=$(this)[0].checked;
		//console.log("change to "+$(this)[0].checked+" !");

		if(ischecked)
			aLink.addClass('arcTransformChecked')
		else
			aLink.removeClass('arcTransformChecked');
		return false;
	}

	$.fn.arcTransCreateCheckBox = function(option){
		var opt = $.extend({},defaultOptions,option);
/*
		<label>
		<span class="arcTransformCheckboxWrapper">
			<a href="#" class="arcTransformCheckbox arcTransformChecked"></a>
			<input type="checkbox" name="enable1" value="on" checked="" class="arcTransformHidden">
		</span>
		</label>
*/
		var $input=$(this);
		// FIXME: $input.attr('checked') will undefined, but $input[0].checked will true/false
		var ischecked=$input[0].checked;
		var css='';
		//console.log($(this).attr('name')+ "==> "+css+' <<<'+ $input.attr('checked')+ ' ..'+$input[0].checked);

		// FIXME: fix checked for jquery need atatched default "checked" attr()
		if(ischecked){
			// why it, we need jquery to know attrib "checked" properity
			$input.attr("checked","checked");
			css='arcTransformChecked';
			if($input.attr('disabled')){css+=' arcTransformCheckedDisabled';}
		}else{
			$input.removeAttr("checked"); // jquery no support attr('checked','')
			if($input.attr('disabled')){css+=' arcTransformDisabled';}
		}
		$input.addClass('arcTransformHidden');
		//add onchange event
		$input.attr('onChange','return $(this).arcTransformCheckBoxChange();');
		// NOTE: IE7: we need set void(0), otherwise UI scrool top
		var htm='<span class="arcTransformCheckboxWrapper">'+
				  '<a href="javascript:return void(0)" class="arcTransformCheckbox '+css+'" onClick="return $(this).arcTransformCheckBoxLnkClick()"></a>'+
				  $input[0].outerHTML+
				'</span>';
		return htm;
	}
	$.fn.arcTransCheckBox = function(){
			return this.each(function(){
				if($(this).hasClass('arcTransformHidden')) {return;}
				var $box = $(this);
				var newBox=$box.arcTransCreateCheckBox({txt:true});
				$(this).replaceWith($(newBox));
			});
	}

	/***************************
	  Radio Buttons
	 ***************************/
	$.fn.arcTransformRadioLnkClick = function(){
		var aLink=$(this);
		var $input=aLink.parent().find('input');
		//do nothing if the original input is disabled
		if($input.attr('disabled')){return false;}
		//trigger the envents on the input object
		$input.trigger('click').trigger('change');

		// uncheck all others of same name input radio elements
		$('input[name="'+$input.attr('name')+'"]',$input[0].form).not($input).each(function(){
					$(this).attr('type')=='radio' && $(this).trigger('change');
		});
		$input.click(); // modified by lichen, to fix the trigger bug...
		return false;
	}
	$.fn.arcTransformRadioChange = function(){
		var aLink=$(this).parent().find('a');
		$(this)[0].checked && aLink.addClass('arcTransformChecked') || aLink.removeClass('arcTransformChecked');
		return false;
	}
	$.fn.arcTransCreateRadio = function(option){
		var opt = $.extend({},defaultOptions,option);
/*
		<span class="arcTransformRadioWrapper">
			<a href="#" class="arcTransformRadio arcTransformChecked" rel="t_type1"></a>
			<input type="radio" name="t_type1" value="1" checked="" class="arcTransformHidden">
		</span>
*/
		var $input=$(this);
		var wrapcss=css="";
		if($input.attr('checked')){
			css='arcTransformChecked';
			if($input.attr('disabled'))
				css+=" arcTransformCheckedDisabled";
		}else{
			if($input.attr('disabled'))
				css+=" arcTransformDisabled";
		}
		$input.addClass('arcTransformHidden');
		//add onchange event
		var doChange=$input.attr('onChange');
		if(!doChange){doChange="";}else{  doChange+=";";}
		$input.attr('onChange',doChange+'return $(this).arcTransformRadioChange()');
		// NOTE: IE7: we need set void(0), otherwise UI scrool top
		var htm='<span class="arcTransformRadioWrapper">'+
				  '<a href="javascript:return void(0)" class="arcTransformRadio '+css+'" rel="t_type1" onClick="return $(this).arcTransformRadioLnkClick()"></a>'+
				  $input[0].outerHTML+
				'</span>';
		return htm;
	}
	$.fn.arcTransRadio = function(){
			return this.each(function(){
				if($(this).hasClass('arcTransformHidden')) {return;}
				var $box = $(this);
				var newBox=$box.arcTransCreateRadio({txt:true});
				$(this).replaceWith($(newBox));
			});
	}
/*
	$.fn.xarcTransRadio = function(){
		return this.each(function(){
			if($(this).hasClass('arcTransformHidden')) {return;}

			var $input = $(this);
			var inputSelf = this;

			oLabel = arcTransformGetLabel($input);
			oLabel && oLabel.click(function(){aLink.trigger('click');});
			// NOTE: IE7: we need set void(0), otherwise UI scrool top
			var aLink = $('<a href="javascript:return void(0)" class="arcTransformRadio" rel="'+ this.name +'"></a>');
			$input.addClass('arcTransformHidden').wrap('<span class="arcTransformRadioWrapper"></span>').parent().prepend(aLink);

			$input.change(function(){
				inputSelf.checked && aLink.addClass('arcTransformChecked') || aLink.removeClass('arcTransformChecked');
				return true;
			});

			if($input.attr('disabled'))
			{
				this.checked && aLink.addClass('arcTransformCheckedDisabled') || aLink.addClass('arcTransformDisabled');
				//oLabel && oLabel.addClass('disabled');
			}
			// Click Handler
			aLink.click(function(){
				if($input.attr('disabled')){return false;}
				$input.trigger('click').trigger('change');

				// uncheck all others of same name input radio elements
				$('input[name="'+$input.attr('name')+'"]',inputSelf.form).not($input).each(function(){
					$(this).attr('type')=='radio' && $(this).trigger('change');
				});

				return false;
			});
			// set the default state
			inputSelf.checked && aLink.addClass('arcTransformChecked');
		});
	};
*/
	/***************************
	  TextArea
	 ***************************/
	$.fn.arcTransTextarea = function(){
		return this.each(function(){
			var textarea = $(this);
			var textareaW = textarea.width();
			var textareaH = textarea.arcTransWidth('height');

			if(textarea.hasClass('arctransformdone')) {return;}
			textarea.addClass('arctransformdone');

			oLabel = arcTransformGetLabel(textarea);
			oLabel && oLabel.click(function(){textarea.focus();});

			var strTable = '<table cellspacing="0" cellpadding="0" border="0" class="arcTransformTextarea">';
			strTable +='<tr><td id="arcTransformTextarea-tl"></td><td id="arcTransformTextarea-tm"></td><td id="arcTransformTextarea-tr"></td></tr>';
			strTable +='<tr><td id="arcTransformTextarea-ml">&nbsp;</td><td id="arcTransformTextarea-mm"><div></div></td><td id="arcTransformTextarea-mr">&nbsp;</td></tr>';
			strTable +='<tr><td id="arcTransformTextarea-bl"></td><td id="arcTransformTextarea-bm"></td><td id="arcTransformTextarea-br"></td></tr>';
			strTable +='</table>';
			var oTable = $(strTable)
					.insertAfter(textarea)
					.hover(function(){
						!oTable.hasClass('arcTransformTextarea-focus') && oTable.addClass('arcTransformTextarea-hover');
					},function(){
						oTable.removeClass('arcTransformTextarea-hover');
					})
				;

			textarea
				.focus(function(){oTable.removeClass('arcTransformTextarea-hover').addClass('arcTransformTextarea-focus');})
				.blur(function(){oTable.removeClass('arcTransformTextarea-focus');})
				.appendTo($('#arcTransformTextarea-mm div',oTable))
			;

			textarea.width(textareaW-15);

			if(textarea.attr('disabled'))
			{
				textarea.addClass("disabled");
			}

			this.oTable = oTable;
			if(isSafari()){
				$('#arcTransformTextarea-mm',oTable)
					.addClass('arcTransformSafariTextarea')
					.find('div')
						.css('height',textareaH) //textarea.arcTransWidth('height')) //.height())
						.css('width' ,textareaW-15) //textarea.arcTransWidth('width')) //.width())
				;
			}
		});
	};

	/***************************
	  Select
	 ***************************/
	$.fn.arcTransformSelectWrapperClick = function(flag){
		this.each(function(index){
			// find the child first and make visible
			var $ul = $('ul', $(this));
			if( $ul.css('display') == 'none' ) {
				arcTransformHideSelect();
			}
			//console.log("click wrapper:"+flag);
			$ul.toggle();
			return false;
		});
		return false;
	}
	$.fn.arcTransformSelectLnkClick = function(){
		this.each(function(index){
			var $wrapper=$(this).parent().parent().parent();
			$('a.selected', $wrapper).removeClass('selected');
			$(this).addClass('selected');
			$select=$wrapper.find('select');
			/* Fire the onchange event */
			if ($select[0].selectedIndex != $(this).attr('index') &&
			    $select[0].onchange) {
			    	$select[0].selectedIndex = $(this).attr('index');
			    	$select[0].onchange();
			}
			$select[0].selectedIndex = $(this).attr('index');
			$('span:eq(0)', $wrapper).html($(this).html());

			//$('ul', $wrapper).hide(); // we no off it, but by wrapper it self
			return false;
		});
		return false;
	}
	$.fn.arcTransCreateSelect = function(option){
		var opt = $.extend({},defaultOptions,option);
	/*
			<div class="arcTransformSelectWrapper" style="z-index: 99; width: 111px; ">
				<div>
					<span style="width: 79px; ">item 0</span>
					<a href="#" class="arcTransformSelectOpen" style=""></a>
				</div>
				<ul style="width: 109px; display: none; visibility: visible; ">
					<li style=""><a href="#" index="0" class="selected">item 0</a></li>
					<li><a href="#" index="1">item 1</a></li>
					<li><a href="#" index="2">item 2</a></li>
					<li><a href="#" index="3">item 3</a></li>
					<li><a href="#" index="4">item 4</a></li>
				</ul>
				<select name="calling_0_1" id="calling_0_1" class="txt1 arcTransformHidden" style="width:150px;">
					<option value="99" selected="">item 0</option>
					<option value="0">item 1</option>
					<option value="1">item 2</option>
					<option value="2">item 3</option>
					<option value="3">item 4</option>
				</select>
			</div>
	*/
		var $select=$(this);

		var oIndex="";
		if(opt.zidx){
			oIndex="z-index:"+(arcTransformImgZindex--)+";";
		}
		//var inputSize=$select.arcTransWidth(); //$input.width();

		var iSelectWidth = $select.arcTransWidth(); //$select.outerWidth();
		// FIXME:
		//   the width is select orginal width and plus ICON of picking list click.
		//   by a tags and class is "arcTransformSelectOpen" (32px)
		var newWidth = iSelectWidth+32; //(iSelectWidth > oSelectWidth)? iSelectWidth+oLinkOpenWidth:oSelectWidth+owrapperWidth; //$wrapper.arcTransWidth();
		var htm;
		/// find the options list and set first entry as default value
		var lst='';
		var obj=$select[0];
		var txt,def_v=null;
		//
/*		it very slow!!
		$('option', $select).each(function(i){
				htm+='<li><a href="#" index='+i+' onClick="return $(this).arcTransformSelectLnkClick()">'+$(this).html()+'</a></li>';
		});
*/

		for(x=0; x< obj.options.length; x++){
			txt=obj.options[x].text;
			lst+='<li><a href="#" index='+x+' onClick="return $(this).arcTransformSelectLnkClick()">'+txt+'</a></li>';
			if(def_v==null){
					def_v=txt;
			}
		}

		htm='<div class="arcTransformSelectWrapper" style="'+oIndex+'width:'+newWidth+'px" onclick="return $(this).arcTransformSelectWrapperClick()">'+
				'<div>'+
					'<span onclick="return $(this).parent().parent().arcTransformSelectWrapperClick()">'+def_v+'</span>'+
					'<a href="#" class="arcTransformSelectOpen" style="" ></a>'+
				'</div>'+
				'<ul style="width:'+newWidth+'px;display:none;visibility:visible;">'+
				lst+
				'</ul>';
		//htm+='</ul>';
		// FIXME: we need to drop those style??
		$select.attr('class','');
		$select.attr('style','');
		//console.log("==>"+$select[0].outerHTML);
		$select.addClass('arcTransformHidden');

		//$select.css('arcTransformHidden');
		// FIXME: special case, we need drop the select orginal CSS here for display, because we need hide it
		htm+=$select[0].outerHTML;
		htm+='</div>';

		//$select.parent().remove($select[0]);
		// FIXME: dirty add lister?
		arcTransformAddDocumentListener();
		return htm;
	}
/*
	$.fn.XarcTransSelect = function(option){
			var opt = $.extend({},defaultOptions,option);
			return this.each(function(index){
				var do_refresh=0;
				var $select = $(this);
				if(opt.force){do_refresh=1}

				if(!do_refresh && $(this).hasClass('arcTransformHidden')) {return;}
				if(do_refresh){
					//caller want to refresh content
					// drop all element at UL ONLY and redraw it
					var oSpan;
					var newWidth=$select.arcTransWidth()+32;
					$wrapper=$(this).parent();
					oSpan = $('span:first',$wrapper);

					$wrapper.find('ul').remove();
					$wrapper.find('select').before('<ul style="display:none;visibility:visible;"></ul>');
					var $ul = $('ul', $wrapper).css('width',newWidth).hide();

					//	    FIXME: this code very slow....

					$('option', this).each(function(i){
							$ul.append($('<li><a href="#" index="'+ i +'" onClick="return $(this).arcTransformSelectLnkClick()">'+ $(this).html() +'</a></li>'));
					});

					//var obj=$select[0];
					//for(x=0; x< obj.options.length; x++){
					//	txt=obj.options[x].text;
					//	ul.append($('<li><a href="#" index='+x+' onClick="return $(this).arcTransformSelectLnkClick()">'+txt+'</a></li>'));
					//	if(def_v==null){
					//			def_v=txt;
					//	}
					//}

					$wrapper.css('width',newWidth);
					$ul.css('width',newWidth-2);
					oSpan.css({width:newWidth+"px"});

					// Calculate the height if necessary, less elements that the default height
					//show the ul to calculate the block, if ul is not displayed li height value is 0
					$ul.css({display:'block',visibility:'hidden'});
					newWidth = ($('li',$ul).length)*(($('li:first',$ul).height()!=0)?$('li:first',$ul).height():29);
					(newWidth < $ul.height()) && $ul.css({height:newWidth,'overflow':'hidden'});//hidden else bug with ff
					$ul.css({display:'none',visibility:'visible'});

					//Set the default
					var oIndex=this.selectedIndex;
					if(oIndex < 0) oIndex=0;
					$('a:eq('+ oIndex +')', $ul).trigger('click');

					//$('span:first', $wrapper).click(function(){$("a.arcTransformSelectOpen",$wrapper).trigger('click');});
					//oLabel && oLabel.click(function(){$("a.arcTransformSelectOpen",$wrapper).trigger('click');});
					$ul.css({display:'none',visibility:'visible'});
				}else{
					var newSel=$select.arcTransCreateSelect({txt:true});
					$(this).replaceWith($(newSel));
				}
			});
	}
*/
	$.fn.arcTransSelect = function(option){
		var opt = $.extend({},defaultOptions,option);
		return this.each(function(index){
			var $select = $(this);
			var selfForm = $(this)[0].form;
			var oIndex=index;
			var do_refresh=0;
			var selectWidth=$select.arcTransWidth(); //$select.width();

			if($select.hasClass('arcTransformHidden')) {do_refresh=1;}

			//console.log($(this).attr('name')+"=>"+oIndex+"<<"+index+' parsent:'+$select.parent().css('zIndex'));
			//if($select.attr('multiple')) {return;}
			if($select.attr('multiple')) {arcTransSelectMulti(this); return;}

			if(do_refresh) {
				if(opt.force != true){return;}
				// drop all element at UL ONLY and redraw it
				$wrapper=$(this).parent();
				$wrapper.find('ul').remove();
				$wrapper.find('select').before('<ul></ul>');
			}else{
				if( !$(selfForm).hasClass('arctransformdone')){return false}
				oIndex=arcTransformImgZindex--;

				//if($(this).attr('name') == "rule_app")
				//console.log("=?"+$(this).attr('name')+"=>"+selectWidth);
				arcTransformMakeTextNodeFloat($select);

				var oLabel  =  arcTransformGetLabel($select);

				/* First thing we do is Wrap it */
				// we need add <label> to make any HTML after select can not be wrap!
				var $wrapper = $select
					.addClass('arcTransformHidden')
					.wrap('<div class="arcTransformSelectWrapper"></div>') //hugh add label to adjust alignment
					.parent()
					.css({zIndex: oIndex})
				;
				/* Now add the html for the select */
				$wrapper.prepend('<div><span></span><a href="#" class="arcTransformSelectOpen"></a></div><ul></ul>');
			}


			//alert($select.width());
			var $ul = $('ul', $wrapper).css('width',selectWidth).hide(); // FIXME: if append prefix html will cause the width is zero,$select.width()).hide();
			/* Now we add the options */
			/* FIXME: this code very slow....*/
			var opt_cnt=0,ul_h=0;
			$('option', this).each(function(i){
				$ul.append($('<li><a href="#" index="'+ i +'">'+ $(this).html() +'</a></li>'));
				if(ul_h==0){ul_h=$wrapper.arcTransWidth('height');} // we need fix the picking list height
				opt_cnt++;
			});
			// fix if list count over 10 items, we need make it a scroll-able
			if(opt_cnt > 10){ opt_cnt= 10}
			$ul.css('height',(ul_h*opt_cnt));

			if($select.attr('disabled')){
				$wrapper.addClass('arcTransformSelectWrapperDisabled');
				$wrapper.find(".arcTransformSelectOpen").addClass("disabled");
			}
			/* Add click handler to the a */
			$('span:first', $wrapper).html(''); // clean present first
			$ul.find('a').click(function(){
					$('a.selected', $wrapper).removeClass('selected');
					$(this).addClass('selected');
					/* Fire the onchange event */
					if ($select[0].selectedIndex != $(this).attr('index') && $select[0].onchange) {
						$select[0].selectedIndex = $(this).attr('index');
						$select[0].onchange();
					}
					$select[0].selectedIndex = $(this).attr('index');
					//$('span:eq(0)', $wrapper).html($(this).html());// FIXME: hugh it is slow
					$('span:first', $wrapper).html($(this).html());
					$ul.hide();
					return false;
			});
			/* Set the default */
			//console.log(this.selectedIndex+":"+this.value);
			$('a:eq('+ this.selectedIndex +')', $ul).click();
			if(!do_refresh) {
				$('span:first', $wrapper).click(function(){$("a.arcTransformSelectOpen",$wrapper).trigger('click');});
				oLabel && oLabel.click(function(){$("a.arcTransformSelectOpen",$wrapper).trigger('click');});
			}
			this.oLabel = oLabel;

			/* Apply the click handler to the Open */
			var oLinkOpen = $('a.arcTransformSelectOpen', $wrapper);
			if(!do_refresh) {
				oLinkOpen.click(function(){
						//FIXME: 2013/12/16: Transform bug
						$ul=$(this).parent().parent().find('ul');
						//Check if box is already open to still allow toggle, but close all other selects
						if( $ul.css('display') == 'none' ) {arcTransformHideSelect();}
						if($select.attr('disabled')){return false;}

						/*$ul.slideToggle('fast', function(){
							var offSet = ($('a.selected', $ul).offset().top - $ul.offset().top);
							$ul.animate({scrollTop: offSet});
						});*/

						$ul.toggle();
						if($('a.selected', $ul).length != 0)
						{
						var offSet = ($('a.selected', $ul).offset().top - $ul.offset().top);
						if(offSet < 0 || offSet > $ul.height())
							$ul.animate({scrollTop: offSet+$ul.scrollTop()});
						}
						return false;
					})
				;
			}
			// Set the new width
			var iSelectWidth = $select.arcTransWidth(); //$select.outerWidth();
			var oSpan = $('span:first',$wrapper);
			var oSelectWidth = oSpan.arcTransWidth('innerWidth');
			var oLinkOpenWidth = oLinkOpen.outerWidth();
			var owrapperWidth = $wrapper.arcTransWidth();
			var oSpanWidth = oSpan.arcTransWidth();
			//var newWidth = (iSelectWidth > oSpan.innerWidth())?iSelectWidth+oLinkOpen.outerWidth():$wrapper.width();
			var newWidth = (iSelectWidth > oSelectWidth)? iSelectWidth+oLinkOpenWidth:oSelectWidth+owrapperWidth; //$wrapper.arcTransWidth();

			$wrapper.css('width',newWidth);
			$ul.css('width',newWidth-2);
			oSpan.css({width:newWidth});//oSpan.css({width:iSelectWidth}); hsiaojung change for media server selector width is too short
			

			// Calculate the height if necessary, less elements that the default height
			//show the ul to calculate the block, if ul is not displayed li height value is 0
			$ul.css({display:'block',visibility:'hidden'});
			var iSelectHeight = ($('li',$ul).length)*(($('li:first',$ul).height()!=0)?$('li:first',$ul).height():29);//+1 else bug ff ///($('li:first',$ul).height())
			//console.log("iSelectHeight="+iSelectHeight);
			(iSelectHeight < $ul.height()) && $ul.css({height:iSelectHeight,'overflow':'hidden'});//hidden else bug with ff
			$ul.css({display:'none',visibility:'visible'});

		});
	};

	/***************************
	  Multi-line Select
	 ***************************/
	var arcTransSelectMulti = function(selectField){
		var select = $(selectField);
		var selectW=select.height();


		if(select.hasClass('arctransformdone')) {return;}
		select.addClass('arctransformdone');

		oLabel = arcTransformGetLabel(select);
		oLabel && oLabel.click(function(){select.focus();});

		var strTable = '<table cellspacing="0" cellpadding="0" border="0" class="arcTransformTextarea">';
		strTable +='<tr><td id="arcTransformTextarea-tl"></td><td id="arcTransformTextarea-tm"></td><td id="arcTransformTextarea-tr"></td></tr>';
		strTable +='<tr><td id="arcTransformTextarea-ml">&nbsp;</td><td id="arcTransformTextarea-mm"><div></div></td><td id="arcTransformTextarea-mr">&nbsp;</td></tr>';
		strTable +='<tr><td id="arcTransformTextarea-bl"></td><td id="arcTransformTextarea-bm"></td><td id="arcTransformTextarea-br"></td></tr>';
		strTable +='</table>';
		var oTable = $(strTable)
				.insertAfter(select)
				.hover(function(){
					!oTable.hasClass('arcTransformTextarea-focus') && oTable.addClass('arcTransformTextarea-hover');
				},function(){
					oTable.removeClass('arcTransformTextarea-hover');
				})
			;

		select
			.focus(function(){oTable.removeClass('arcTransformTextarea-hover').addClass('arcTransformTextarea-focus');})
			.blur(function(){oTable.removeClass('arcTransformTextarea-focus');})
			.appendTo($('#arcTransformTextarea-mm div',oTable))
		;
		//textarea.width(textareaW);

		this.oTable = oTable;
		if(isSafari()){
			$('#arcTransformTextarea-mm',oTable)
				.addClass('arcTransformSafariTextarea')
				.find('div')
					.css('height',select.height())
					.css('width',select.width())
			;
		}

	};
	/***************************
	  InputText (disable)
	 ***************************/

	$.fn.jqDisabledInput = function(disabled) {
		return this.each(function(){
			if(disabled)
			{
				$(this).addClass("arcTransformDisabled");
				$(this).parent().parent().addClass("arcTransformInputInnerDisabled");
				$(this).parent().parent().parent().addClass("arcTransformInputWrapperDisabled");
			}
			else
			{
				$(this).removeClass("arcTransformDisabled");
				$(this).parent().parent().removeClass("arcTransformInputInnerDisabled");
				$(this).parent().parent().parent().removeClass("arcTransformInputWrapperDisabled");
			}
		});
	}
	/***************************
	  Check Boxes (disable)
	 ***************************/

	$.fn.jqDisabledCheckBox = function(disabled) {
		return this.each(function(){
			var aLink = $(this).prev();
			aLink.removeClass("arcTransformChecked");
			aLink.removeClass("arcTransformDisabled");
			aLink.removeClass("arcTransformCheckedDisabled");
			//console.log("check: set "+disabled);
			if(disabled)
				this.checked && aLink.addClass('arcTransformCheckedDisabled') || aLink.addClass('arcTransformDisabled');
			else
				this.checked && aLink.addClass('arcTransformChecked');
		});
	}
	/***************************
	  Radio Buttons (disable)
	 ***************************/

	$.fn.jqDisabledRadio = function(disabled) {
		return this.each(function(){
			var aLink = $(this).prev();
			aLink.removeClass("arcTransformChecked");
			aLink.removeClass("arcTransformDisabled");
			aLink.removeClass("arcTransformCheckedDisabled");
			if(disabled)
				this.checked && aLink.addClass('arcTransformCheckedDisabled') || aLink.addClass('arcTransformDisabled');
			else
				this.checked && aLink.addClass('arcTransformChecked');
		});
	}
	/***************************
	  Multi-line/single Select (Disabled)
	 ***************************/

	$.fn.jqDisabledSelect = function(disabled) {
		return this.each(function(){
			$wrapper = $(this).parent();
			if($wrapper && !$wrapper.hasClass("arcTransformSelectWrapper")) return;

			if(disabled)
			{
				$wrapper && $wrapper.addClass('arcTransformSelectWrapperDisabled');
				$wrapper && $wrapper.find(".arcTransformSelectOpen").addClass("disabled");
			}
			else
			{
				$wrapper && $wrapper.removeClass('arcTransformSelectWrapperDisabled');
				$wrapper && $wrapper.find(".arcTransformSelectOpen").removeClass("disabled");
			}
		});
	}
	/***************************
	  Buttons (disable)
	 ***************************/
	$.fn.jqDisabledButton = function(disabled) {
		return this.each(function(){
			if(disabled)
			{
				$(this).addClass("disabled");
				$("#btn_"+this.id+".arcTransformButton").addClass("arcTransformButtonDisabled")
			}
			else
			{
				$(this).removeClass("disabled");
				$("#btn_"+this.id+".arcTransformButton").removeClass("arcTransformButtonDisabled")
			}
		});
	}
	// hugh extan it
    $.fn.jqDisabled = function(disabled){
    	//var obj_type=$(this).attr('type');
    	var obj_type=$(this)[0].type;
    	if(obj_type==""){
    		obj_type=$(this)[0].tagName; //special for button
    	}
		switch(obj_type){
		case "radio":
			$(this).jqDisabledRadio(disabled);
			break;
		case "checkbox":
			$(this).jqDisabledCheckBox(disabled);
			break;
		case "select-one":
		case "select-multiple":
			$(this).jqDisabledSelect(disabled);
			break;
		case "password":
		case "textarea":
		case "text":
			$(this).jqDisabledInput(disabled);
			break;
		case "button":
		case "A":
			$(this).jqDisabledButton(disabled);
			break;
		case "file":
		case "hidden":
		default:
			break;
		}
    }
	/***************************
	  Set Value
	 ***************************/
	// hugh extend it
    $.fn.jqSetValue = function(value){
    	var obj_type=$(this)[0].type;
		switch(obj_type){
		case "radio":
			$(this).each(function(){
				$(this).arcTransformRadioChange();
			//	if($(this).val() == value){
			//		var aLink=$(this).parent().find('a');
			//		aLink.trigger('click');
			//	}
			});
			break;
		case "checkbox":
			var aLink=$(this).parent().find('a');
			var isCheck=($(this).val() == value);
			//console.log($(this).val() +"<==="+value);
			if(isCheck)
			   aLink.addClass('arcTransformChecked')
			else
			   aLink.removeClass('arcTransformChecked');
			break;
		case "select-one":
		case "select-multiple":
			$aWrap=$(this).parent();
			//$aWrap.trigger('click'); // we no need to emulate user click wrapper first then click hyper lin next
			$(this).find('option').each(function(){
				//console.log($(this).index()+"=>"+$(this).val());
				if($(this).val() == value){
						var selindex=$(this).index();
						// we got the select index
						var aLink=$aWrap.find('a[index='+selindex+']');
						if(aLink.length!=0){
							aLink.trigger('click');
						}
						//console.log($('ul', $aWrap)[0].innerHTML);
						$('ul', $aWrap).hide(); //force hide it
				}
			});

			break;
/*		case "password":
		case "textarea":
		case "text":
		case "button":
		case "file":
		case "hidden":*/
		default:
			break;
		}
    }
	/***************************
	  Major Transform
	 ***************************/
	$.fn.arcTransform = function(options){
		var opt = $.extend({},defaultOptions,options);

		/* each form */
		 return this.each(function(){
			var selfForm = $(this);
			//if(options["force"] != true)
			if(opt.force != true)
				if(selfForm.hasClass('arctransformdone')) {return;}
			selfForm.addClass('arctransformdone');
			// dirty to hide the transform working area
			selfForm.hide();

			$('input:submit, input:reset, input[type="button"]', this).arcTransInputButton();
			$('input:text, input:password', this).arcTransInputText();
			$('input:checkbox', this).arcTransCheckBox();
			$('input:radio', this).arcTransRadio();
			selfForm.show(); //why open here due to textarea width will incorrect
			$('textarea', this).arcTransTextarea();

			$('button', this).arcTransButton();

			if( $('select', this).arcTransSelect().length > 0 ){arcTransformAddDocumentListener();}
			if(this.tagName == "FORM"){
				selfForm.bind('reset',function(){var action = function(){arcTransformReset(this);}; window.setTimeout(action, 10);});
			}
			//preloading dont needed anymore since normal, focus and hover image are the same one
			/*if(opt.preloadImg && !arcTransformImgPreloaded){
				arcTransformImgPreloaded = true;
				var oInputText = $('input:text:first', selfForm);
				if(oInputText.length > 0){
					//pour ie on eleve les ""
					var strWrapperImgUrl = oInputText.get(0).wrapper.css('background-image');
					arcTransformPreloadHoverFocusImg(strWrapperImgUrl);
					var strInnerImgUrl = $('div.arcTransformInputInner',$(oInputText.get(0).wrapper)).css('background-image');
					arcTransformPreloadHoverFocusImg(strInnerImgUrl);
				}

				var oTextarea = $('textarea',selfForm);
				if(oTextarea.length > 0){
					var oTable = oTextarea.get(0).oTable;
					$('td',oTable).each(function(){
						var strImgBack = $(this).css('background-image');
						arcTransformPreloadHoverFocusImg(strImgBack);
					});
				}
			}*/
			// dirty to show the transform working area
			selfForm.show();

		}); /* End Form each */

	};/* End the Plugin */

	/* ******************** */
	/* modal                */
	/* ******************** */
	var modalQueued = false;
    $.fn.reveal = function (options) {
			var defaults = {
						animation: 'fadeAndPop',                // fade, fadeAndPop, none
						Speed: 300,                    // how fast animtions are
						closeOnBackgroundClick: false, // if you click background will modal close?
						closeOnESC: false, // if you press ESC will modal close?
						dismissModalClass: 'close-reveal-modal', // the class of a button or element that will close an open modal
						open: $.noop,
						opened: $.noop,
						close: $.noop,
						closed: $.noop
				};
				options = $.extend({}, defaults, options);

		return this.each(function () {
				var modal    = $(this),
				topMeasure = Math.ceil(($(window).height()-modal.height())/3), //NOTE: we fix it,parseInt(modal.css('top'), 10),
				topOffset  = modal.height() + topMeasure,
				leftOffset = Math.ceil( ($(window).width()-modal.width())/2),
				locked     = false,
				modalBg = $('.reveal-modal-bg'),
				closeButton;

				if (modalBg.length === 0) {
					modalBg = $('<div class="reveal-modal-bg" />').insertAfter(modal);
					modalBg.fadeTo('fast', 0.8);
				}

				function Modal_unlock() {
							locked = false;
				}

				function Modal_lock() {
							locked = true;
				}

				function closeOpenModals(modal) {

						var openModals = $(".reveal-modal.open");
						if (openModals.length === 1) {
						  modalQueued = true;
						  $(".reveal-modal.open").trigger("reveal:close");
						}
				}
				function doc_top(){
					return ($.browser.msie)? $('html,body').scrollTop(): $(document).scrollTop();
				}
				function Animation_open() {
					if (!locked) {
						Modal_lock();
						closeOpenModals(modal);
						modal.addClass("open");
						if (options.animation === "fadeAndPop") {
							modal.css({'top': (doc_top() - topOffset),
							'left':'50%', //leftOffset, auto adjust
							'opacity': 0,
							'visibility': 'visible'});
							modalBg.fadeIn(options.Speed / 2);
							modal.delay(options.Speed / 2).animate({
									"top": (doc_top() + topMeasure) + 'px',
									"opacity": 1
							}, options.Speed, function () {
								  modal.trigger('reveal:opened');
							});

						}
						if (options.animation === "fade") {
							modal.css({'opacity': 0, 'visibility': 'visible', 'top': doc_top() + topMeasure});
							modalBg.fadeIn(options.Speed / 2);
							modal.delay(options.Speed / 2).animate({
								"opacity": 1
							}, options.Speed, function () {
								modal.trigger('reveal:opened');
							});
						}
						if (options.animation === "none") {
							modal.css({'visibility': 'visible', 'top': doc_top() + topMeasure});
							modalBg.css({"display": "block"});
							modal.trigger('reveal:opened');
						}
					}
				}

				modal.bind('reveal:open.reveal', Animation_open);

				function Animation_close() {
					if (!locked) {
						Modal_lock();
						modal.removeClass("open");
						if (options.animation === "fadeAndPop") {
							modal.animate({
								"top": doc_top() - topOffset + 'px',
								"opacity": 0
						  	}, options.Speed / 2, function () {
								modal.css({'top': topMeasure, 'opacity': 1, 'visibility': 'hidden'});
						  	});
							if (!modalQueued) {
								  modalBg.delay(options.Speed).fadeOut(options.Speed, function () {
										modal.trigger('reveal:closed');
								  });
							} else {
								modal.trigger('reveal:closed');
							}
							modalQueued = false;
						}
						if (options.animation === "fade") {
							modal.animate({
								"opacity" : 0
							}, options.Speed, function () {
								modal.css({'opacity': 1, 'visibility': 'hidden', 'top': topMeasure});
							});
								if (!modalQueued) {
													  modalBg.delay(options.Speed).fadeOut(options.Speed, function () {
																	modal.trigger('reveal:closed');
													  });
													} else {
													  modal.trigger('reveal:closed');
													}
						}
						if (options.animation === "none") {
							modal.css({'visibility': 'hidden', 'top': topMeasure});
							if (!modalQueued) {
								modalBg.css({'display': 'none'});
							}
							modal.trigger('reveal:closed');
						}
					}
				}

				function destroy() {
					modal.unbind('.reveal');
					modalBg.unbind('.reveal');
					$('.' + options.dismissModalClass).unbind('.reveal');
					$('body').unbind('.reveal');
				}

				modal.bind('reveal:close.reveal', Animation_close);
				modal.bind('reveal:opened.reveal reveal:closed.reveal', Modal_unlock);
				modal.bind('reveal:closed.reveal', destroy);

				modal.bind('reveal:open.reveal', options.open);
				modal.bind('reveal:opened.reveal', options.opened);
				modal.bind('reveal:close.reveal', options.close);
				modal.bind('reveal:closed.reveal', options.closed);
				modal.trigger('reveal:open');

				closeButton = $('.' + options.dismissModalClass).bind('click.reveal', function () {
							modal.trigger('reveal:close');
				});

				if (options.closeOnBackgroundClick) {
					modalBg.css({"cursor": "pointer"});
					modalBg.bind('click.reveal', function () {
						modal.trigger('reveal:close');
					});
				}

				if (options.closeOnESC) {
					$('body').bind('keyup.reveal', function (event) {
						if (event.which === 27) { // 27 is the keycode for the Escape key
							  modal.trigger('reveal:close');
						}
					});
				}
			}); // end of return

		};
	/* End of modal*/

	/**********************
	 **   Round          **
	 **********************/
	var msie = /[MSIE][Trident]/.test(navigator.userAgent);

	var style = document.createElement('div').style,
		moz = style['MozBorderRadius'] !== undefined,
		webkit = style['WebkitBorderRadius'] !== undefined,
		radius = style['borderRadius'] !== undefined || style['BorderRadius'] !== undefined,
		mode = document.documentMode || 0,
		noBottomFold = msie && (!mode || mode < 8),

		expr = msie && (function() {
			var div = document.createElement('div');
			try { div.style.setExpression('width','0+0'); div.style.removeExpression('width'); }
			catch(e) { return false; }
			return true;
		})();

	$.support = $.support || {};
	$.support.borderRadius = moz || webkit || radius; // so you can do:  if (!$.support.borderRadius) $('#myDiv').corner();

	function sz(el, p) {
		return parseInt($.css(el,p),10)||0;
	}
	function hex2(s) {
		s = parseInt(s,10).toString(16);
		return ( s.length < 2 ) ? '0'+s : s;
	}
	function gpc(node) {
		while(node) {
			var v = $.css(node,'backgroundColor'), rgb;
			if (v && v != 'transparent' && v != 'rgba(0, 0, 0, 0)') {
				if (v.indexOf('rgb') >= 0) {
					rgb = v.match(/\d+/g);
					return '#'+ hex2(rgb[0]) + hex2(rgb[1]) + hex2(rgb[2]);
				}
				return v;
			}
			if (node.nodeName.toLowerCase() == 'html')
				break;
			node = node.parentNode; // keep walking if transparent
		}
		return '#ffffff';
	}

	function getWidth(fx, i, width) {
		switch(fx) {
		case 'round':  return Math.round(width*(1-Math.cos(Math.asin(i/width))));
		case 'cool':   return Math.round(width*(1+Math.cos(Math.asin(i/width))));
		case 'sharp':  return width-i;
		case 'bite':   return Math.round(width*(Math.cos(Math.asin((width-i-1)/width))));
		case 'slide':  return Math.round(width*(Math.atan2(i,width/i)));
		case 'jut':    return Math.round(width*(Math.atan2(width,(width-i-1))));
		case 'curl':   return Math.round(width*(Math.atan(i)));
		case 'tear':   return Math.round(width*(Math.cos(i)));
		case 'wicked': return Math.round(width*(Math.tan(i)));
		case 'long':   return Math.round(width*(Math.sqrt(i)));
		case 'sculpt': return Math.round(width*(Math.log((width-i-1),width)));
		case 'dogfold':
		case 'dog':    return (i&1) ? (i+1) : width;
		case 'dog2':   return (i&2) ? (i+1) : width;
		case 'dog3':   return (i&3) ? (i+1) : width;
		case 'fray':   return (i%2)*width;
		case 'notch':  return width;
		case 'bevelfold':
		case 'bevel':  return i+1;
		case 'steep':  return i/2 + 1;
		case 'invsteep':return (width-i)/2+1;
		}
	}

	$.fn.corner = function(options) {
		// in 1.3+ we can fix mistakes with the ready state
		if (this.length === 0) {
			if (!$.isReady && this.selector) {
				var s = this.selector, c = this.context;
				$(function() {
					$(s,c).corner(options);
				});
			}
			return this;
		}

		return this.each(function(index){
			var $this = $(this),
				// meta values override options
				o = [$this.attr($.fn.corner.defaults.metaAttr) || '', options || ''].join(' ').toLowerCase(),
				keep = /keep/.test(o),                       // keep borders?
				cc = ((o.match(/cc:(#[0-9a-f]+)/)||[])[1]),  // corner color
				sc = ((o.match(/sc:(#[0-9a-f]+)/)||[])[1]),  // strip color
				width = parseInt((o.match(/(\d+)px/)||[])[1],10) || 10, // corner width
				re = /round|bevelfold|bevel|notch|bite|cool|sharp|slide|jut|curl|tear|fray|wicked|sculpt|long|dog3|dog2|dogfold|dog|invsteep|steep/,
				fx = ((o.match(re)||['round'])[0]),
				fold = /dogfold|bevelfold/.test(o),
				edges = { T:0, B:1 },
				opts = {
					TL:  /top|tl|left/.test(o),       TR:  /top|tr|right/.test(o),
					BL:  /bottom|bl|left/.test(o),    BR:  /bottom|br|right/.test(o)
				},
				// vars used in func later
				strip, pad, cssHeight, j, bot, d, ds, bw, i, w, e, c, common, $horz;

			if ( !opts.TL && !opts.TR && !opts.BL && !opts.BR )
				opts = { TL:1, TR:1, BL:1, BR:1 };

			// support native rounding
			if ($.fn.corner.defaults.useNative && fx == 'round' && (radius || moz || webkit) && !cc && !sc) {
				if (opts.TL)
					$this.css(radius ? 'border-top-left-radius' : moz ? '-moz-border-radius-topleft' : '-webkit-border-top-left-radius', width + 'px');
				if (opts.TR)
					$this.css(radius ? 'border-top-right-radius' : moz ? '-moz-border-radius-topright' : '-webkit-border-top-right-radius', width + 'px');
				if (opts.BL)
					$this.css(radius ? 'border-bottom-left-radius' : moz ? '-moz-border-radius-bottomleft' : '-webkit-border-bottom-left-radius', width + 'px');
				if (opts.BR)
					$this.css(radius ? 'border-bottom-right-radius' : moz ? '-moz-border-radius-bottomright' : '-webkit-border-bottom-right-radius', width + 'px');
				return;
			}

			strip = document.createElement('div');
			$(strip).css({
				overflow: 'hidden',
				height: '1px',
				minHeight: '1px',
				fontSize: '1px',
				backgroundColor: sc || 'transparent',
				borderStyle: 'solid'
			});

			pad = {
				T: parseInt($.css(this,'paddingTop'),10)||0,     R: parseInt($.css(this,'paddingRight'),10)||0,
				B: parseInt($.css(this,'paddingBottom'),10)||0,  L: parseInt($.css(this,'paddingLeft'),10)||0
			};

			if (typeof this.style.zoom !== undefined) this.style.zoom = 1; // force 'hasLayout' in IE
			if (!keep) this.style.border = 'none';
			strip.style.borderColor = cc || gpc(this.parentNode);
			cssHeight = $(this).outerHeight();

			for (j in edges) {
				bot = edges[j];
				// only add stips if needed
				if ((bot && (opts.BL || opts.BR)) || (!bot && (opts.TL || opts.TR))) {
					strip.style.borderStyle = 'none '+(opts[j+'R']?'solid':'none')+' none '+(opts[j+'L']?'solid':'none');
					d = document.createElement('div');
					$(d).addClass('jquery-corner');
					ds = d.style;

					bot ? this.appendChild(d) : this.insertBefore(d, this.firstChild);

					if (bot && cssHeight != 'auto') {
						if ($.css(this,'position') == 'static')
							this.style.position = 'relative';
						ds.position = 'absolute';
						ds.bottom = ds.left = ds.padding = ds.margin = '0';
						if (expr)
							ds.setExpression('width', 'this.parentNode.offsetWidth');
						else
							ds.width = '100%';
					}
					else if (!bot && msie) {
						if ($.css(this,'position') == 'static')
							this.style.position = 'relative';
						ds.position = 'absolute';
						ds.top = ds.left = ds.right = ds.padding = ds.margin = '0';

						// fix ie6 problem when blocked element has a border width
						if (expr) {
							bw = sz(this,'borderLeftWidth') + sz(this,'borderRightWidth');
							ds.setExpression('width', 'this.parentNode.offsetWidth - '+bw+'+ "px"');
						}
						else
							ds.width = '100%';
					}
					else {
						ds.position = 'relative';
						ds.margin = !bot ? '-'+pad.T+'px -'+pad.R+'px '+(pad.T-width)+'px -'+pad.L+'px' :
											(pad.B-width)+'px -'+pad.R+'px -'+pad.B+'px -'+pad.L+'px';
					}

					for (i=0; i < width; i++) {
						w = Math.max(0,getWidth(fx,i, width));
						e = strip.cloneNode(false);
						e.style.borderWidth = '0 '+(opts[j+'R']?w:0)+'px 0 '+(opts[j+'L']?w:0)+'px';
						bot ? d.appendChild(e) : d.insertBefore(e, d.firstChild);
					}

					if (fold && $.support.boxModel) {
						if (bot && noBottomFold) continue;
						for (c in opts) {
							if (!opts[c]) continue;
							if (bot && (c == 'TL' || c == 'TR')) continue;
							if (!bot && (c == 'BL' || c == 'BR')) continue;

							common = { position: 'absolute', border: 'none', margin: 0, padding: 0, overflow: 'hidden', backgroundColor: strip.style.borderColor };
							$horz = $('<div/>').css(common).css({ width: width + 'px', height: '1px' });
							switch(c) {
							case 'TL': $horz.css({ bottom: 0, left: 0 }); break;
							case 'TR': $horz.css({ bottom: 0, right: 0 }); break;
							case 'BL': $horz.css({ top: 0, left: 0 }); break;
							case 'BR': $horz.css({ top: 0, right: 0 }); break;
							}
							d.appendChild($horz[0]);

							var $vert = $('<div/>').css(common).css({ top: 0, bottom: 0, width: '1px', height: width + 'px' });
							switch(c) {
							case 'TL': $vert.css({ left: width }); break;
							case 'TR': $vert.css({ right: width }); break;
							case 'BL': $vert.css({ left: width }); break;
							case 'BR': $vert.css({ right: width }); break;
							}
							d.appendChild($vert[0]);
						}
					}
				}
			}
		});
	};

	$.fn.uncorner = function() {
		if (radius || moz || webkit)
			this.css(radius ? 'border-radius' : moz ? '-moz-border-radius' : '-webkit-border-radius', 0);
		$('div.jquery-corner', this).remove();
		return this;
	};

	// expose options
	$.fn.corner.defaults = {
		useNative: true, // true if plugin should attempt to use native browser support for border radius rounding
		metaAttr:  'data-corner' // name of meta attribute to use for options
	};

})(jQuery);

