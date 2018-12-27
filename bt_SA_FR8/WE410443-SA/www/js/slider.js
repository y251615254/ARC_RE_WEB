/*
1.First write the following code in the HTML file:
<div class="scroll-bar" id="scroll-bar">
	<div class="entire-bar" id="entire-bar"></div>
	<div class="action-bar" id="action-bar"></div>
	<div class="action-block" id="action-block"></div>
</div>
<input type="text" id="showArea"/>

The class name and ID according to your love named
.css file is also according to your love to rename.

But the style required is:"scroll-bar" position must be relative,And the inside of the div(scroll-bar) three div Like:entire-bar,action-bar,action-block
position must be absolute,In addition to the outer layer of "scroll-bar" best to set a certain value margin-top,Inside the package div Set a certain value top,specific CSS in the annex can refer to.

2. HTML file in the head part of the introduction of the JS file.
<script src="slide.js"></script>

3.Add an script tag to the </body> tag and write the following code
new SlideBar({
	actionBlock : 'action-block',
	actionBar : 'action-bar',
	slideBar : 'scroll-bar',
	barLength : 500,
	interval : 50,
	maxNumber : 200,
	showArea : 'showArea'
});

Sliding the ID of the actionBlock corresponding to the slider, actionBar sliding correspondingly much display of ID, slideBar corresponding to the outer div ID, barLength corresponding strip width (pixel value), the interval corresponding you want the slider on the unit interval, maxnumber corresponding maximum value you want to slide, corresponding to the showArea display numerical input box ID (optional, if not removed).
*/

/*	window.onload = function() {
	var oActionBlock = document.getElementById('action-block');
	var oActionBar = document.getElementById('action-bar');
	var oScrollBar = document.getElementById('scroll-bar');
	var oShowAmount = document.getElementById('showAmount').getElementsByTagName('input')[0];
	var length = 550;

	clickSlide(oActionBlock, oActionBar, oScrollBar, 300, length, oShowAmount);
	drag(oActionBlock, oActionBar, 300, length, oShowAmount);
	addScale(60, 300, length, oScrollBar);
	inputBlur(oActionBlock, oActionBar, length, oShowAmount);
}		
*/

function SlideBar(data){
	var _this = this;
	var oActionBlock = document.getElementById(data.actionBlock);
	var oActionBar = document.getElementById(data.actionBar);
	var oSlideBar = document.getElementById(data.slideBar);
	var barLength = data.barLength;
	var interval = data.interval;
	var maxNumber = data.maxNumber;
	var oShowArea = null;
	var setLedValue= data.initLedValue;// Init Led slider default value 
	
	if(data.showArea){
		oShowArea = document.getElementById(data.showArea);	
	}

	if(oShowArea){
		_this.addScale(oSlideBar, interval, maxNumber, barLength);
		_this.inputBlur(oActionBlock, oActionBar, maxNumber, barLength, oShowArea);
		_this.clickSlide(oActionBlock, oActionBar, oSlideBar, maxNumber, barLength, oShowArea);
		//_this.drag(oActionBlock, oActionBar, maxNumber, barLength, oShowArea);
	}
	else{
		_this.addScale(oSlideBar, interval, maxNumber, barLength);
		_this.clickSlide(oActionBlock, oActionBar, oSlideBar, maxNumber, barLength);
		//_this.drag(oActionBlock, oActionBar, maxNumber, barLength);
	}


	//Init set Led value for web
	 _this.initLedSliderValue(oActionBlock, oActionBar, maxNumber, barLength, setLedValue);
	
}


SlideBar.prototype = {
		//initialize (add scale line) //初始化(添加刻度线)
	addScale : function(slideBar, interval, total, barLength){
		//Interval represents the number of intervals between the scales, total represents the largest scale
		// slideBar In which container to add the scale
		// interval代表刻度之间间隔多少, total代表最大刻度
		// slideBar表示在哪个容器添加刻度
		
		var num = total / interval; //num	how many scales should be //num为应该有多少个刻度

		for (var i = 0; i < num + 1; i++) {
			var oScale = document.createElement('div');
			oScale.style.width = '2px';
			oScale.style.height = '16px';
			oScale.style.position = 'absolute';
			oScale.style.background = '#57399C';  //Indicator color
			oScale.style.zIndex = '-10';
			oScale.style.left = (i * interval * barLength) / total + 'px';
			slideBar.appendChild(oScale);

			var oText = document.createElement('div');
			oText.style.position = 'absolute';
			oText.style.top = '16px';
			oText.style.height = '16px';
			oText.innerHTML = i * interval;
			
			if(i == 0)
			{
				oText.innerHTML = "<br/><span style='color:#666666; font-size: 10pt; font-family:Calibri;'>Off</sapan>";
			}
			if(i == 1)
			{
				oText.innerHTML = "<br/><span style='color:#666666; font-size: 10pt; font-family:Calibri;'>Dim</sapan>";
			}
			if(i == 2)
			{
				oText.innerHTML = "<br/><span style='color:#666666; font-size: 10pt; font-family:Calibri;'>Normal</sapan>";
			}
			
			slideBar.appendChild(oText);
			oText.style.left = ((i * interval * barLength) / total) - (oText.offsetWidth / 2) + 'px';
		}
		
		
	},

	// Listening input box// 监听输入框
	inputBlur : function(actionBlock, actionBar, maxNumber, barLength, input){
		//actionBlock->Finger slider,	actionBar->Display bar,	input->input box
		//actionBlock指滑块,actionBar指显示条,input指显示的输入框
		var _this = this;
		input.onblur = function(){
			var inputVal = this.value;
			_this.autoSlide(actionBlock, actionBar, maxNumber, barLength, inputVal);
		}
	},


	initLedSliderValue : function(actionBlock, actionBar, maxNumber, barLength, input){
		//actionBlock->Finger slider,	actionBar->Display bar,	input->input box
		var _this = this;
		var inputVal = input;
		var target = (inputVal / maxNumber * barLength);
		_this.checkAndMoveInit(actionBlock, actionBar, target);
		
	},
	

	/* Auto slide after input value of the input box	*/
		/* 在输入框输入值后自动滑动	*/
	autoSlide : function(actionBlock, actionBar, total, barLength, inputVal){
		//inputVal->Represents the value entered in the input box
		//inputVal表示输入框中输入的值
		var _this = this;
		var target = (inputVal / total * barLength);
		_this.checkAndMove(actionBlock, actionBar, target);
	},


	/* Auto slide after input value of the input box	*/
	
	autoSlidex : function(actionBlock, actionBar, total, barLength, inputVal){
		//inputVal->Represents the value entered in the input box
		//actionBar的移动度和方向
		var _this = this;
		var target = (inputVal / total * barLength);
		_this.checkAndMove(actionBlock, actionBar, target);
	},
	

	
	checkAndMoveInit : function(actionBlock, actionBar, target){
		var actionBarPace = target;
		actionBar.style.width = actionBarPace + 'px';
		actionBlock.style.left = actionBar.offsetWidth - (actionBlock.offsetWidth / 2) + 'px';
	
	},
	
	
	/*	Check target (confirm movement direction) and slide	*//*	检查target(确认移动方向)并滑动	*/
	checkAndMove : function(actionBlock, actionBar, target){		
		//set action-block value near Off,Dim,Normal		
		if(target >= -3 && target <=170)
		{
			target = 2;
		}else if(target > 170 && target <375)
		{
			target = 248;
		}else if(target >= 375 && target <550)
		{
			target = 498;
		}else if(target > 500)
		{
			target = 500;
		}
	  //set end	
		
		if(target > actionBar.offsetWidth){
			actionBarSpeed = 13;		//actionBar Moving speed and direction
		}
		else if(target == actionBar.offsetWidth){
			return;
		}
		else if(target < actionBar.offsetWidth){
			actionBarSpeed = -13;
		}
		
		var timer = setInterval(function(){
			var actionBarPace = actionBar.offsetWidth + actionBarSpeed;

			if(Math.abs(actionBarPace - target) < 10){
				actionBarPace = target;
				clearInterval(timer);
			}
			/*Fix When the mouse fast move, slip cross-border issues start
				When the mouse fast move, slip cross-border issues to fixed value is 500, because 
			    WebUI is set to 500.SlideBar new ({barLength: 500,}) actionBarPace and barLength 
				should be equal.
			*/
			if(actionBarPace > 500)
			{
				actionBarPace = 500;
			}
			//Fix When the mouse fast move, slip cross-border issues end
			
			
			actionBar.style.width = actionBarPace + 'px';
			actionBlock.style.left = actionBar.offsetWidth - (actionBlock.offsetWidth / 2) + 'px';
		},30);
	},

	/*	Mouse click on the scale slider automatically	*/
	/*	鼠标点击刻度滑动块自动滑动	*/
	clickSlide : function(actionBlock, actionBar, slideBar, total, barLength, showArea){
		var _this = this;
		slideBar.onclick = function(ev){
			var ev = ev || event;
			var target = ev.clientX - slideBar.offsetLeft;
			if(target < 0){
				//Indicates that the mouse has exceeded that range.
				target = 0;
			}
			if(target > barLength){
				target = barLength;
			}
			_this.checkAndMove(actionBlock, actionBar, target);
			
			LedValueSlider = Math.round(target / barLength * total);
			
			if(showArea){
				showArea.value = Math.round(target / barLength * total);
				LedValueSlider = showArea.value;		
			}

			if(LedValueSlider == 0)
			{
				DisplayObject("slider_info_id");
			}
			else
			{
				HiddenObject("slider_info_id");
			}

		}
	},

	/*	Drag the mouse by the slide bar	*/
	//drag : function(actionBlock, actionBar, total, barLength, showArea){
	//	The parameters are click on the slide of the block, the sliding distance, the maximum value of the slide bar, display the value of the place (input box)
	/*	actionBlock.onmousedown = function(ev) {
			var ev = ev || event;
			var thisBlock = this;
			var disX = ev.clientX;
			var currentLeft = thisBlock.offsetLeft;

			document.onmousemove = function(ev) {
				var ev = ev || event;
				var left = ev.clientX - disX;

				if (currentLeft + left <= (barLength - thisBlock.offsetWidth / 2 ) && currentLeft + left >= 0 - thisBlock.offsetWidth / 2) {
					thisBlock.style.left = currentLeft + left + 'px';
					actionBar.style.width = currentLeft + left + (actionBlock.offsetWidth / 2) + 'px';
					if(showArea){
						showArea.value = Math.round(actionBar.offsetWidth / barLength * total);
					}
				}
				return false;
			}

			document.onmouseup = function() {
				document.onmousemove = document.onmouseup = null;
			}

			return false;
		}
	},
	*/
	

	getStyle : function(obj, attr){
		return obj.currentStyle?obj.currentStyle[attr]:getComputedStyle(obj)[attr];
	}
}