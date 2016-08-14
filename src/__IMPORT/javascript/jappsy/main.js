function Jappsy(onerror) {
	this.contexts = new Jappsy.GLContexts(this);
	this.extension = null;
	this.onerror = onerror || null;
}

Jappsy.prototype.release = function() {
	if (this.extension !== null) {
		this.extension.release();
		this.extension = null;
	}
	this.contexts.release();
	this.contexts = null;
	this.onerror = null;
};

///////////////////////////

Jappsy.GLContexts = function(engine) {
	this.engine = engine;
	this.list = {};
};

Jappsy.GLContexts.prototype.release = function() {
	this.engine = null;
	for (var key in this.list)
		this.list[key].release();
	this.list = {};
	this.main = null;
};

Jappsy.GLContexts.prototype.createContext = function(key, onframe, ontouch, canvas) {
	if (this.list[key] !== undefined)
		this.list[key].release();
	
	var context = new Jappsy.GLContext(this.engine, onframe, ontouch, canvas);
	
	this.list[key] = context;
	
	return context;
};

///////////////////////////

Jappsy.check = function() {
	// Typed arrays check
	if (
		(typeof Uint8Array !== 'undefined') &&
		(typeof Uint16Array !== 'undefined') &&
		(typeof Int32Array !== 'undefined') &&
		(typeof Float32Array !== 'undefined')
	) {
		// Class extend check
		var obj = Object.create(Float32Array.prototype);
		obj = Float32Array.apply(obj, [16]);
		obj["customMethod"] = function() { return true; };
		try {
			if (obj.customMethod()) {
				return true;
			}
		} catch (e) {
		}
	}
	return false;
};

Jappsy.ZLib = {};

Jappsy.modules = [
	"modules/core.js",
	"modules/lang.js",
	"modules/uri.js",
	"modules/loader.js",
	"modules/touch.js",
	"modules/matrix.js",
	
	"modules/lib/utils/common.js",
	"modules/lib/utils/strings.js",
	"modules/lib/zlib/adler32.js",
	"modules/lib/zlib/crc32.js",
	"modules/lib/zlib/inffast.js",
	"modules/lib/zlib/inftrees.js",
	"modules/lib/zlib/inflate.js",
	"modules/lib/zlib/constants.js",
	"modules/lib/zlib/messages.js",
	"modules/lib/zlib/zstream.js",
	"modules/lib/zlib/gzheader.js",
	"modules/lib/inflate.js",
	"modules/stream.js",
	"modules/reader.js",

	"modules/gui/JTypeface.js",
	"modules/gui/JAlignX.js",
	"modules/gui/JAlignY.js",
	"modules/gui/JPaint.js",

	"modules/opengl/GLContext.js",
	"modules/opengl/GLShader.js",
	"modules/opengl/GLTexture.js",
	"modules/opengl/GLSprite.js",
	"modules/opengl/GLScene.js",
	"modules/opengl/GLCamera.js",
	"modules/opengl/GLLight.js",
	"modules/opengl/GLModel.js",
	"modules/opengl/GLObject.js",
	"modules/opengl/GLParticle.js",
	"modules/opengl/GLDrawing.js",
	"modules/opengl/GLFunc.js",
];

Jappsy.styles = [
	"main.css",
];

Jappsy.includes = [];

Jappsy.assign = function (obj /*from1, from2, from3, ...*/) {
	var sources = Array.prototype.slice.call(arguments, 1);
	while (sources.length) {
		var source = sources.shift();
		if (!source) continue;

		if (typeof source !== 'object')
			throw new TypeError(source + 'must be non-object');

		for (var p in source)
			if (source.hasOwnProperty(p))
				obj[p] = source[p];
	}

	return obj;
};

Jappsy.mod = function(value, divider) {
	return value - (Math.floor(value / divider) * divider);
};

Jappsy.random = function(max) {
	return Math.floor(Math.random()*max);
};

Jappsy.now = function() {
	return new Date().getTime();
};
if (typeof(window.preformance) != 'undefined') {
	if (typeof(window.performance.now) != 'undefined') {
		Jappsy.now = function() { return window.performance.now(); };
	} else if (typeof(window.performance.webkitNow) != 'undefined') {
		Jappsy.now = function() { return window.performance.webkitNow(); };
	}
}

Jappsy.clamp = function(value, min, max) {
	if (value < min) return min;
	if (value > max) return max;
	return value;
};

Jappsy.engineReady = false;
Jappsy.run = function() {
	if (Jappsy.runOnce === undefined) {
		Jappsy.runOnce = false;
		try {
			throw new Error();
		} catch(e) {
			var stackLines = e.stack.split('\n');
			
			var find = /(\b(https?|ftp|file):\/\/[-A-Z0-9+&@#\/%?=~_|!:,.;]*[-A-Z0-9+&@#\/%=~_|])/ig;
			var parse = /^(?:([^:\/?#]+):)?(?:\/\/((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?))?((((?:[^?#\/]*\/)*)([^?#]*))(?:\?([^#]*))?(?:#(.*))?)/;
			for (var i in stackLines) {
				var line = find.exec(stackLines[i]);
				if (line && line[1]) {
					var uri = parse.exec(line[1]);
					var j = 14;
					while (j--) uri[j] = uri[j] || "";
					var path = (uri[1] != "" ? uri[1] + "://" : "") +
						((uri[4] != "" || uri[5] != "") ? uri[4] + ":" + uri[5] + "@" : "") +
						uri[6] +
						(uri[7] != "" ? ":" + uri[7] : "") +
						uri[10].replace(/\/$/, "");
					Jappsy.basePath = path;
					break;
				}
			}
		}

		try {
			var nocache = "?_" + new Date().getTime();
		
			var filesLoaded = 0;
			var filesCount = Jappsy.modules.length + Jappsy.styles.length;
			function checkLoaded() {
				if (filesLoaded === filesCount) {
					if (Jappsy.startup === undefined)
						alert("Create Jappsy.startup() function, it will run after all modules loaded");
					else {
						Jappsy.engineReady = true;
						Jappsy.startup();
						Jappsy.runOnce = true;
					}
				}
			}
			for (var i = 0; i < Jappsy.modules.length; i++) {
				var script = document.createElement('script');
				script.type = 'text/javascript';
				script.src = Jappsy.basePath + "/" + Jappsy.modules[i] + nocache;
				script.module = Jappsy.modules[i];
				script.onload = function() {
					filesLoaded++;
					Jappsy.includes.push(this.module);
					if (Jappsy.requires.querys[this.module] !== undefined) {
						for (var j = 0; j < Jappsy.requires.querys[this.module].length; j++) {
							var query = Jappsy.requires.querys[this.module][j];
							query.ready++;
							query.check();
						}
						delete Jappsy.requires.querys[this.module];
					}
					checkLoaded();
				}
				document.head.appendChild(script);
			}
			for (var i = 0; i < Jappsy.styles.length; i++) {
				var style = document.createElement('link');
				style.rel = 'stylesheet';
				style.href = Jappsy.basePath + "/" + Jappsy.styles[i] + nocache;
				style.type = 'text/css';
				style.onload = function() { filesLoaded++; checkLoaded(); }
				document.head.appendChild(style);
			}
		} catch(e) {
		}
	}
};

Jappsy.Require = function(onready) {
	this.total = 0;
	this.ready = 0;
	this.onready = onready;
};

Jappsy.Require.prototype.check = function() {
	if (this.total == this.ready) {
		this.onready();
		return true;
	}
	return false;
};

Jappsy.requires = function(modules, onready) {
	var query = new Jappsy.Require(onready);
	for (var i = 0; i < modules.length; i++) {
		query.total++;
		var module = modules[i];
		if (Jappsy.includes.indexOf(module) !== -1)
			query.ready++;
		else {
			if (Jappsy.requires.querys[module] === undefined)
				Jappsy.requires.querys[module] = [];
			Jappsy.requires.querys[module].push(query);
		}
	}
	query.check();
};

Jappsy.requires.querys = {};
