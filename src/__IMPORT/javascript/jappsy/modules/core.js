Jappsy.prototype.config = {
	DEBUG: "openglDebug",		// false or dom element id
	DEBUG_FPS: true,
	FPSLIMIT: 0
};

Jappsy.__IOS__ = /(iPad|iPhone|iPod)/g.test( navigator.userAgent );

Jappsy.prototype.nop = function() {
};

Jappsy.prototype.logTime = new Date().getTime();

Jappsy.prototype.log = function(text, nobr, parent) {
	if (!this.config.DEBUG) return null;
	
	if (this.oDebug === undefined) {
		this.oDebug = document.getElementById(this.config.DEBUG);
		this.oDebug.innerText = "";
	}

	var div = null;
	if (parent !== undefined)
		div = parent;
	else {
		div = document.createElement('div');
		this.oDebug.appendChild(div);
		var now = new Date().getTime() - this.logTime;
		var ms = Jappsy.mod(now, 1000); now = Math.floor(now / 1000);
		var s = Jappsy.mod(now, 60); now = Math.floor(now / 60);
		var m = Jappsy.mod(now, 60); now = Math.floor(now / 60);
		$(div).append(
			//('00' + m).substr(-2) + ':' +
			('00' + s).substr(-2) + '.' +
			('000' + ms).substr(-3) + '&nbsp;&nbsp;');
	}
	
	if (nobr !== undefined && nobr == true)
		$(div).append(text.toString() + '&nbsp;');
	else
		$(div).append(text.toString() + '<br />');
	
	return div;
}

Jappsy.prototype.logRemove = function(element) {
	if (element != null)
		this.oDebug.removeChild(element);
}

Jappsy.prototype.now = Jappsy.now;

Jappsy.SmoothValue = function(count) {
	this.items = [];
	for (var i = 0; i < count; i++)
		this.items.push(null);
	this.put = function(value) {
		this.items.unshift(value);
		this.items.pop();
	};
	this.value = function() {
		var total = 0;
		var count = 0;
		for (var i = 0; i < this.items.length; i++)
			if (this.items[i] !== null) {
				total += this.items[i];
				count++;
			}
		if (count > 0)
			return total/count;
		return null;
	};
}

Jappsy.SmoothInterval = function(count) {
	this.smooth = new Jappsy.SmoothValue(count);
	this.startTime = null;
	this.start = function() {
		this.startTime = Jappsy.now();
	}
	this.stop = function() {
		if (this.startTime !== null) {
			var interval = Jappsy.now() - this.startTime;
			this.startTime = null;
			this.smooth.put(interval);
			return interval;
		}
		return 0;
	}
	this.value = function() {
		var val = this.smooth.value();
		if (val !== null) {
			return Math.round(val * 1000) / 1000;
		}
		return 0;
	}
}
