Jappsy.GLFunc = function(callback) {
	this.callback = callback;
}

Jappsy.GLFunc.prototype.render = function(object) {
	this.callback(object);
}
