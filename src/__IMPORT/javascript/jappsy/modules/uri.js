Jappsy.__URI__ = true;

Jappsy.URI = function(str, strictMode) {
	var o = Jappsy.URI.options;
		mode = strictMode || true,
		m = o.parser[mode ? "strict": "loose"].exec(str);
		i = 14;

	while (i--) this[o.keys[i]] = m[i] || "";
	
	var q = this[o.keys[12]];
	this[o.parser.queryKey] = {};
	
	var THIS = this;
	q.replace(o.parser.query, function ($0, $1, $2) {
		if ($1) THIS[o.parser.queryKey][$1] = $2;
	});
	
	var e = o.parser.ext.exec(this[o.keys[11]]);
	this[o.parser.extKey] = (e[1] || "").toLowerCase();
};

Jappsy.URI.options = {
	keys: ["source","protocol","authority","userInfo","user","password","host","port","relative","path","directory","file","query","anchor"],
	parser: {
		strict: /^(?:([^:\/?#]+):)?(?:\/\/((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?))?((((?:[^?#\/]*\/)*)([^?#]*))(?:\?([^#]*))?(?:#(.*))?)/,
		loose: /^(?:(?![^:@]+:[^:@\/]*@)([^:\/?#.]+):)?(?:\/\/)?((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?)(((\/(?:[^?#](?![^?#\/]*\.[^?#\/.]+(?:[?#]|$)))*\/?)?([^?#\/]*))(?:\?([^#]*))?(?:#(.*))?)/,
		query: /(?:^|&)([^&=]*)=?([^&]*)/g,
		queryKey: "queryKeys",
		ext: /(\.[^\.]*)$/,
		extKey: "ext"
	}
};

Jappsy.URI.prototype.scheme = function() {
	if (this._scheme === undefined)
		this._scheme = (this.protocol != "" ? this.protocol + "://" : "");
	return this._scheme;
};

Jappsy.URI.prototype.server = function() {
	if (this._server === undefined)
		this._server = ((this.user != "" || this.password != "") ? this.user + ":" + this.password + "@" : "") +
			this.host +
			(this.port != "" ? ":" + this.port : "");
	return this._server;
};

Jappsy.URI.prototype.fullPath = function() {
	if (this._fullPath === undefined)
		this._fullPath = this.scheme() + this.server() + this.directory.replace(/\/$/, "");
	return this._fullPath;
};

Jappsy.URI.prototype.absolutePath = function(basePath) {
	if (this._absolutePath === undefined) {
		if (this.scheme() == "" || this.server() == "") {
			var path = this.directory.replace(/^\//, "").replace(/\/$/, "");
			if (basePath !== undefined) {
				this._absolutePath = basePath.replace(/\/$/, "") + (path != "" ? "/" + path : "");
			} else {
				this._absolutePath = Jappsy.basePath + (path != "" ? "/" + path : "");
			}
		} else {
			this._absolutePath = this.fullPath();
		}
	}
	return this._absolutePath;
};

Jappsy.URI.prototype.uri = function() {
	if (this._uri === undefined) {
		this._uri = this.absolutePath() + "/" + this.file + (this.query != "" ? "?" + this.query : "");
	}
	return this._uri;
};
