Object.extend = function(target, base) {
	for (var method in base)
		if (base.hasOwnProperty(method))
			if (!target.hasOwnProperty(method))
				target[method] = base[method];

	for (var method in base.prototype)
		if (base.prototype.hasOwnProperty(method))
			if (!target.prototype.hasOwnProperty(method))
				target.prototype[method] = base.prototype[method];
};

Object.isArray = function(object) {
	return object.hasOwnProperty("length");
};
