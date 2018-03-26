// from input format generate the PORT_RANG structure
// NOTE: we no more use supper task 's way, overwrite this function again, 2010/12/20, hugh
function parsePortValueRange(ports){
	var sub_value, range, sub_range, val1, val2;
	var i;

	if(ports == null || ports.length == 0) return null;

	sub_value = ports.split(/\,/);
	range = new Array(sub_value.length);
	for(i=0; i < sub_value.length; i++)	{
		range[i] = new PORT_RANGE();
		sub_range = sub_value[i].split(/\-/);
		// is single ports
		if(sub_range.length == 1){
			if(isNaN(sub_range[0]) == true) return null;
			range[i].b_port = range[i].e_port =parseInt(sub_range[0],10);
			range[i].mult=0;
		} else if(sub_range.length == 2) {
			// multi-ports
				if(isNaN(sub_range[0]) == true || isNaN(sub_range[1]) == true) return null;
			val1=parseInt(sub_range[0],10);
			val2=parseInt(sub_range[1],10);
			//if(val1 > val2){ val2=val1; val1=parseInt(sub_range[1],10);}
			range[i].b_port = val1;
			range[i].e_port = val2;
			range[i].mult=1;
		}else{
			return null;
		}
	}
	return range;
}
