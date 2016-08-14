/*
Jappsy.prototype.load = function(config) {
	var loader = new Jappsy.Loader(this, config);

	loader.onfile = (config.onfile !== undefined) ? config.onfile.bind(this) : this.nop;
	loader.onstatus = (config.onstatus !== undefined) ? config.onstatus.bind(this) : this.nop;
	loader.onload = (config.onload !== undefined) ? config.onload.bind(this) : this.nop;
	loader.onerror = (config.onerror !== undefined) ? config.onerror.bind(this) : this.nop;
	loader.run();
};

Jappsy.prototype.createLoader = function(files) {
	return new Jappsy.Loader(this, files);
};
*/

Jappsy.Loader = function(context, config) {
	this.context = context;
	this.result = {};
	this.list = [];
	this.onUpdate = this.update.bind(this);
	this.running = false;
	this.shutdown = false;

	this.status = {
		update: false,
		count: 0,
		total: 0,
		left: 0,
		error: 0
	};

	this.load(config);
	this.onfile = context.engine.nop;
	this.onstatus = context.engine.nop;
	this.onload = context.engine.nop;
	this.onerror = context.engine.nop;
};

Jappsy.Loader.fileTypes = {
	image: [ ".png", ".jpg", ".jpeg", ".gif" ],
	sound: [ ".mp3", ".ogg" ],
	json: [ ".json" ],
	text: [ ".txt" ],
	jappsyImage: [ ".sdff", ".sdfi", ".jimg" ],
	jappsyShader: [ ".jsh" ],
	jappsyVertexShader: [ ".vsh" ],
	jappsyFragmentShader: [ ".fsh" ],
};

Jappsy.Loader.typeOf = function(ext) {
	for (var i in Jappsy.Loader.fileTypes) {
		// FIXED: IE
		//if (Jappsy.Loader.fileTypes[i].includes(ext))
		if (Jappsy.Loader.fileTypes[i].indexOf(ext) != -1)
			return i;
	}
	return "data";
};

Jappsy.Loader.responseTypes = {
	arraybuffer: [ ".sdff", ".sdfi", ".jimg", ".jsh" ],
	text: [ ".txt", ".vsh", ".fsh" ],
	json: [ ".json" ],
};

Jappsy.Loader.responseTypeOf = function(ext) {
	for (var i in Jappsy.Loader.responseTypes) {
		// FIXED: IE
		//if (Jappsy.Loader.responseTypes[i].includes(ext))
		if (Jappsy.Loader.responseTypes[i].indexOf(ext) != -1)
			return i;
	}
	return "text";
};

Jappsy.Loader.prototype.release = function() {
	if (this.running) {
		this.shutdown = true;
	}
	this.context = null;
	this.result = {};
	this.list = [];
	this.onUpdate = null;
};

Jappsy.Loader.prototype.config = {
	loadSpeed: 5,
	soundEnable: true
};

Jappsy.Loader.prototype.load = function(config) {
	if (config === undefined) return false;

	if (config.files !== undefined) {
		this.loadGroup(null, config.files);
		
		return true;
	} else if (config.groups !== undefined) {
		for (var groupkey in config.groups) {
			this.loadGroup(groupkey, config.groups[groupkey]);
		}
		
		return true;
	}
};

Jappsy.Loader.prototype.loadGroup = function(groupkey, group) {
	if ( typeof group === 'string' ) {
		var uri = new Jappsy.URI(group);
		this.list.push({ file: uri.uri(), ext: uri.ext, key: uri.file, group: groupkey });
	} else if ( Array.isArray(group) ) {
		for (var i = 0; i < group.length; i++) {
			var uri = new Jappsy.URI(group[i]);
			this.list.push({ file: uri.uri(), ext: uri.ext, key: uri.file, group: groupkey });
		}
	} else {
		for (var key in group) {
			var uri = new Jappsy.URI(group[key]);
			this.list.push({ file: uri.uri(), ext: uri.ext, key: key, group: groupkey });
		}
	}

	this.status.total = this.list.length;
};

Jappsy.Loader.prototype.hasDownloads = function() {
	return this.list.length;
};

Jappsy.Loader.prototype.lastDownload = function() {
	return this.list[this.list.length - 1];
};
	
Jappsy.Loader.prototype.doneDownload = function() {
	this.list.pop();
};

Jappsy.Loader.prototype.updating = false;
Jappsy.Loader.prototype.checkUpdate = function(time) {
	if ((!this.updating) && (!this.shutdown)) {
		this.updating = true;
		setTimeout( this.onUpdate, time);
	}
};
Jappsy.Loader.prototype.update = function() {
	if (!this.shutdown) {
		this.updating = false;
		this.cacheid = new Date().getTime();
			
		var count = this.hasDownloads();
		if (this.status.error > 0) count = 0;
		if (count > this.config.loadSpeed) count = this.config.loadSpeed;
		if (count == 0) count = -1;
		if (this.status.left > this.config.loadSpeed) {
			count = 0;
		} else if (this.status.update) {
			this.onstatus(this.status);
			this.status.update = false;
		}
		while (count > 0) {
			var info = this.lastDownload();
			var type = Jappsy.Loader.typeOf(info.ext);
			info.type = type;
			if (type == "image") {
				this.status.update = true;
				this.status.left++;
				this.createImageLoader(info);
			} else if (type == "sound") {
				this.status.update = true;
				this.status.left++;
				this.createSoundLoader(info);
			} else {
				this.status.update = true;
				this.status.left++;
				this.createDataLoader(info);
			}
			this.doneDownload();
			count--;
		}
		if (this.status.left > 0) {
			this.checkUpdate(15);
		} else if (count == 0) {
			this.checkUpdate(15);
		} else if (this.status.error > 0) {
			this.onerror(this.lastError);
		} else {
			try {
				this.onload(this.result);
			} catch (e) {
				this.onerror(e);
			}
		}
	}
};

Jappsy.Loader.prototype.run = function() {
	if ((!this.running) && (!this.shutdown)) {
		this.running = true;
		this.updating = false;
		this.checkUpdate(15);
	}
};

Jappsy.Loader.prototype.createInfo = function(info) {
	return {
		"loader": this,
		"debug": this.context.engine.log("Loading: " + info.file, true),
		"file": info.file,
		"ext": info.ext,
		"key": info.key,
		"group": info.group,
		"type": info.type,
		"onload": function(data) {
			if (!this.loader.shutdown) {
				this.loader.status.count++;
				this.loader.status.left--;
				this.loader.status.update = true;
				if (this.group !== null) {
					if (this.loader.result[this.group] === undefined)
						this.loader.result[this.group] = {};
					this.loader.result[this.group][this.key] = data;
				} else {
					this.loader.result[this.key] = data;
				}
				this.loader.onfile(this, data);
				this.loader.context.engine.log("Done; ", true, this.debug);
				this.loader.context.engine.logRemove(this.debug);
			}
		},
		"onerror": function(data) {
			if (!this.loader.shutdown) {
				this.loader.status.left--;
				this.loader.status.error++;
				this.loader.lastError = this.file;
				this.loader.context.engine.log("Error; ", true, this.debug);
			}
		},
		"release": function() {
			this.loader = null;
			this.debug = null;
			this.file = null;
			this.ext = null;
			this.key = null;
			this.group = null;
			this.type = null;
		}
	};
};

Jappsy.Loader.prototype.createImageLoader = function(info) {
	var image = new Image();
	image.user = this.createInfo(info);
	image.user._onload = function(e) {
		var user = this.user;
		this.removeEventListener('error', user._onerror, false);
		this.removeEventListener('load', user._onload, false);
		delete this.user;
		
		if (!user.loader.shutdown) {
			var texture = user.loader.context.textures.createTextureFromImg( user.key, this );
			if (texture != null) {
				user.onload(this);
			} else {
				user.onerror('Unexpected content of file');
			}
			// Remove texture from galery, it is already loaded into OpenGL
			user.loader.context.engine.oGalery.removeChild(this);
		}
		user.release();
	};
	image.user._onerror = function(e) {
		var user = this.user;
		this.removeEventListener('error', user._onerror, false);
		this.removeEventListener('load', user._onload, false);
		delete this.user;
		
		if (!user.loader.shutdown) {
			user.loader.context.engine.oGalery.removeChild(this);
			user.onerror(this);
		}
		user.release();
	};
	
	image.addEventListener('error', image.user._onerror, false);
	image.addEventListener('load', image.user._onload, false);
	this.context.engine.oGalery.appendChild(image);
	
	var nocache = "?_" + new Date().getTime();
	image.src = info.file + nocache;
};

Jappsy.Loader.prototype.createSoundLoader = function(info) {
	var audio = document.createElement('audio');
	audio.autoplay = false;
	audio.volume = 1;//0.00000001;
	audio.preload = true;
	audio.loaded = false;
	
	var nocache = "?_" + new Date().getTime();
	if (/\.ogg$/i.test(info.file)) {
		var src = document.createElement('source');
		src.src = info.file + nocache;
		src.type = 'audio/ogg';
		src.preload = 'auto';
		audio.appendChild(src);
	}
	if (/\.mp3$/i.test(info.file)) {
		var src = document.createElement('source');
		src.src = info.file + nocache;
		src.type = 'audio/mpeg';
		src.preload = 'auto';
		audio.appendChild(src);
	}
	
	audio.user = this.createInfo(info);
	
	var onload = function(e) {
		var user = this.user;
		this.removeEventListener('error', user._onerror, false);
		this.removeEventListener('pause', user._onpause, false);
		this.removeEventListener('durationchange', user._ondurationchange, false);
		this.removeEventListener('ended', user._onended, false);
		this.removeEventListener('canplaythrough', user._oncanplaythrough, false);
		delete this.user;
		
		if (!user.loader.shutdown) {
			user.onload(this);
		}
		user.release();
	};
	
	audio.user._onload = onload.bind(audio);
	
	audio.user._onerror = function(e) {
		var user = this.user;
		this.removeEventListener('error', user._onerror, false);
		this.removeEventListener('pause', user._onpause, false);
		this.removeEventListener('durationchange', user._ondurationchange, false);
		this.removeEventListener('ended', user._onended, false);
		this.removeEventListener('canplaythrough', user._oncanplaythrough, false);
		delete this.user;
		
		if (!user.loader.shutdown) {
			user.loader.context.engine.oMixer.removeChild(this);
			user.onerror(this);
		}
		user.release();
	};
	
	audio.user._onpause = function(e) {
		if (!this.user.loader.shutdown) {
			this.user.loader.context.engine.log("Pause; ", true, this.user.debug);
			//this.loaded = true; // На Windows Phone помогает врубить звуки но не все
			if (this.loaded)
				this.user._onload(this);
		}
	};
	
	audio.user._ondurationchange = function(e) {
		if (!this.user.loader.shutdown) {
			if (!this.loaded) {
				this.loaded = true;
				this.pause();
				this.currentTime = 0;
				this.volume = 1;
				this.user.loader.context.engine.log("Playing; ", true, this.user.debug);
			} else {
				this.user.loader.context.engine.log("Playing; ", true, this.user.debug);
			}
		}
	};
	
	audio.user._onended = function(e) {
		if (!this.user.loader.shutdown) {
			this.user.loader.context.engine.log("End; ", true, this.user.debug);
		}
	};
	
	audio.user._oncanplaythrough = function(e) {
		if (!this.user.loader.shutdown) {
			if (!this.loaded) {
				this.loaded = true;
				this.pause();
				this.currentTime = 0;
				this.volume = 1;
				this.user.loader.context.engine.log("Buffered; ", true, this.user.debug);
			} else {
				this.user.loader.context.engine.log("Buffered; ", true, this.user.debug);
			}
		}
	};

	audio.addEventListener('error', audio.user._onerror, false);
	audio.addEventListener('pause', audio.user._onpause, false);
	audio.addEventListener('durationchange', audio.user._ondurationchange, false);
	audio.addEventListener('ended', audio.user._onended, false);
	audio.addEventListener('canplaythrough', audio.user._oncanplaythrough, false);
	this.context.engine.oMixer.appendChild(audio);

	audio.play();
};

Jappsy.Loader.prototype.createDataLoader = function(info) {
	var object = this.createInfo(info);
	var nocache = "?_" + new Date().getTime();
	
	var resType = Jappsy.Loader.responseTypeOf(object.ext);
	var resLoad = function(object, data) {
		if (!object.loader.shutdown) {
			if (object.type == "json") {
				object.onload(data);
			} else if (object.type == "jappsyImage") {
				try {
					var o = JappsyReader.createTexture(object.loader.context, object.key, data);
					object.onload(o);
				} catch (err) {
					object.onerror(err);
				}
			} else if (object.type == "jappsyShader") {
				try {
					var o = JappsyReader.createShader(object.loader.context, object.key, data);
					object.onload(o);
				} catch (err) {
					object.onerror(err);
				}
			} else if (object.type == "jappsyVertexShader") {
				var sh = object.loader.context.shaders.createVertexShader(data);
				if (sh != null) {
					var vsh = new Jappsy.Data( sh, Jappsy.DataStyleObject );
					var o = object.loader.context.shaders.createShader(object.key, vsh, null, null, []);
					object.onload(o);
				} else {
					object.onerror('Unexpected content of file');
				}
			} else if (object.type == "jappsyFragmentShader") {
				var sh = object.loader.context.shaders.createFragmentShader(data);
				if (sh != null) {
					var fsh = new Jappsy.Data( sh, Jappsy.DataStyleObject );
					var o = object.loader.context.shaders.createShader(object.key, null, fsh, null, []);
					object.onload(o);
				} else {
					object.onerror('Unexpected content of file');
				}
			} else {
				object.onload(data);
			}
		}
	};
	
	if (Jappsy.__IOS__) {
		$.ajax({
			context: object,
			type: 'GET',
			url: object.file + nocache,
			dataType: resType,
			success: function(text) {
				if (!this.loader.shutdown) {
					resLoad(this, text);
				}
				this.release();
			},
			error: function(err, text) {
				if (!this.loader.shutdown) {
					this.onerror(err.responseText + ': ' + text);
				}
				this.release();
			}
		});
	} else {
		var loader = new XMLHttpRequest();
		if (loader) {
			loader.parent = object;
			loader.open('GET', object.file + nocache, true);
			loader.responseType = resType;
			loader.onreadystatechange = function (event) {
				if (this.readyState === 4) {
					if (this.status === 200) {
						if (!this.parent.loader.shutdown) {
							if (this.responseType == "") {
								if (Jappsy.Loader.responseTypeOf(this.parent.ext) == "json") {
									var json = JSON.parse(this.responseText);
									resLoad(this.parent, json);
								} else {
									resLoad(this.parent, this.responseText);
								}
							} else {
								resLoad(this.parent, this.response);
							}
						}
						this.parent.release();
						delete this.parent;
					} else {
						if (!this.parent.loader.shutdown) {
							this.parent.onerror(this.statusText);
						}
						this.parent.release();
						delete this.parent;
					}
				}
			};
			loader.send(null);
		} else if (window.XDomainRequest) {
			loader = new XDomainRequest();
			loader.parent = object;
			loader.open('GET', object.file + nocache, true);
			loader.onload = function() {
				if (!this.parent.loader.shutdown) {
					if (this.parent.type == "json") {
						var json = JSON.parse(this.responseText);
						resLoad(this.parent, json);
					} else {
						resLoad(this.parent, this.responseText);
					}
				}
				this.parent.release();
				delete this.parent;
			};
			loader.onerror = function() {
				if (!this.parent.loader.shutdown) {
					this.parent.onerror(this.status);
				}
				this.parent.release();
				delete this.parent;
			};
			loader.send(null);
		}
	}
};
