// Выполняется после прогрузки всех необходимых скриптов для движка
Jappsy.startup = function() {
	// TODO: Временно делаем автозапуск
	StartGame();
};

// Выполняется для остановки движка
Jappsy.stop = function(engine) {
	if ((engine !== null) && (Jappsy.engineReady)) {
		engine.release();
	}
	return null;
};

// Выполняется для запуска движка
Jappsy.start = function(onerror) {
	if (!Jappsy.engineReady) {
		return null;
	}
	
	var engine = new Jappsy(onerror);
	engine.extension = new Jappsy.Game(engine);
	
	return engine;
};

///////////////////////////

Jappsy.Effect = function(key, x, y, d, start, pause, duration, mobile) {
	this.key = key;
	this.x1 = x - d;
	this.y1 = y - d;
	this.x2 = x + d;
	this.y2 = y + d;
	this.start = start || 0;
	this.pause = pause || -1;
	this.duration = duration || -1;
	this.mobile = mobile || true;
};

Jappsy.Effect.prototype.render = function(context, worldTime) {
	var localTime = 0;
	if (this.start < worldTime) {
		localTime = worldTime - this.start;
		if (this.duration >= 0) {
			if (localTime > this.duration) {
				localTime = 0;
				return false;
			} else if ((this.pause >= 0) && (localTime > (this.duration - this.pause))) {
				localTime = this.duration - localTime;
			}
		}
		if ((this.pause >= 0) && (localTime > this.pause)) {
			localTime = this.pause;
		}
	}
	context.drawEffectMobile(this.x1, this.y1, this.x2, this.y2, this.key, localTime / 86400000.0, worldTime / 86400000.0);
	return true;
};

Jappsy.Bars = function() {
	var rot1 = new Jappsy.Rotation(null);
	var rot2 = new Jappsy.Rotation(null);//rot1);
	var rot3 = new Jappsy.Rotation(null);//rot2);
	this.rotation = [rot1, rot2, rot3];
	this.mask = new Array(3);
	this.effects = [];
};

Jappsy.Bars.prototype.update = function() {
	var currentTime = (new Date).getTime();
	return Jappsy.Rotation.updateBars(this.rotation, currentTime);
};

Jappsy.Bars.prototype.stopped = function() {
	return Jappsy.Rotation.stoppedBars(this.rotation);
};

Jappsy.Bars.prototype.start = function() {
	this.rotation[0].start();
	this.rotation[1].start();
	this.rotation[2].start();
	this.effects = [];
};

Jappsy.Bars.prototype.stopRandom = function() {
	var sum = Jappsy.Rotation.generateGrace(this.rotation);
	this.effects = [];
	return sum;
};

Jappsy.Bars.prototype.calculate = function() {
	var sum = Jappsy.Rotation.calculateBars(this.rotation, this.mask);

	var mobile = true;
	var currentTime = (new Date).getTime();
	
	for (var i = 0; i < 3; i++) {
		if (this.mask[i] != 0) {
			var ofs = this.rotation[i].offset();
			var key = null;
			if ((ofs == 0) || (ofs == 2) || (ofs == 6)) {
				key = mobile ? "mobile_effect_moon1" : "effect_moon1";
			} else if ((ofs == 1) || (ofs == 3) || (ofs == 5)) {
				key = mobile ? "mobile_effect_sun1" : "effect_sun1";
			}
			var x = 960+(i-1)*456;
			var y = 572;
			switch (ofs) {
				case 1:
				case 6:
					y -= 242;
					break;
				case 2:
				case 5:
					y += 236;
					break;
			}
			var effect = new Jappsy.Effect(key, x, y, 150, currentTime, -1, -1, mobile);
			this.effects.push( effect );
		}
	}
	
	return sum;
};

Jappsy.Bars.prototype.renderEffects = function(context) {
	if (this.effects.length > 0) {
		var worldTime = (new Date).getTime();
		for (var i = this.effects.length - 1; i >= 0; i--) {
			if (!this.effects[i].render(context, worldTime)) {
				this.effects.splice(i, 1); // remove
			}
		}
	}
};

Jappsy.Game = function(engine) {
	this.engine = engine;
	
	var oGLBody = document.getElementById("openglBody");
	var oGLCanvas = document.getElementById("openglCanvas");
	var oGLMixer = document.getElementById("openglMixer");
	var oGLGalery = document.getElementById("openglGalery");

	// Создаем контекст OpenGL
	this.context = engine.contexts.createContext("game", this.onFrame.bind(this), this.onTouch.bind(this), oGLCanvas);
	oGLCanvas.focus();
	
	// Задаем переменне
	Jappsy.assign(this, {
		scene: null,			// У нас всего одна сцена игровая с разными активными слоями
	
		ready: false,			// Готовность всех ресурсов после загрузки
		
		stage: Jappsy.Game.STAGE_UNKNOWN,
		nextTime: 0,
		nextTimeout: 0,
		nextStage: Jappsy.Game.STAGE_BARS,
		nextCallback: null,
		
		prizeTimeout: 0,		// Таймаут показа приза
		takeTimeout: 0,			// Таймаут сбора очков
		freespinTime: 0,		// Время начала выпадения фриспина
		takeTime: 0,			// Время начала забирания приза
		
		rollTimeout: 0,			// Таймаут кручения барабана (ожидания от сервера)
		selectTimeout: 0,		// Таймаут выбора затмения (ожидания от сервера)
		doubleShowTimeout: 0,	// Таймаут показа затмения
		doubleHideTimeout: 0,	// Таймаут скрытия затмения
		localTime: 0,			// Локальное время для эффектов
		
		conf: {
			points: 1000,		// Current Points
			bet: 99,			// Current Bet
			prize: 0,			// Current Spin Prize
			prizefree: 0,		// Current Free Spin Prize
			total: 0,			// Total Free Spins Prize
			doublein: 10,		// Double In Prize
			doubleout: 20,		// Double Out Prize
			spin: 0,			// Current Spin
			free: 0,			// Total Free Spins
			auto: false,		// Auto Spin
			sequence: [1,1,0,0,0],	// Double Sequence (0 sun / 1 moon)
			dblselect: 0,
			dblindex: 0,
		},
		
		barlist: null,
		
		// Строка в логе с фпс
		debugfps: null,
	});
	
	this.barlist = this.barlist1.concat(this.barlist2, this.barlist3);
	
	this.engine.oGalery = oGLGalery;
	this.engine.oMixer = oGLMixer;
	
	this.paint = new Jappsy.JPaint();
	this.paint.setColor(0x80FF0000).setStroke(3, 0x800000FF);
	
	this.paintShadow = new Jappsy.JPaint().setColor(0x80000000);
	this.paintSwitch = new Jappsy.JPaint().setColor(0x00000000);
	this.paintSpriteCenter = new Jappsy.JPaint()
		.setColor(0xFFFFFFFF)
		.setAlignX(Jappsy.JAlignX.CENTER)
		.setAlignY(Jappsy.JAlignY.MIDDLE);
	this.paintSpriteLeftCenter = new Jappsy.JPaint()
		.setColor(0xFFFFFFFF)
		.setAlignX(Jappsy.JAlignX.LEFT)
		.setAlignY(Jappsy.JAlignY.MIDDLE);
		
	this.load();
};

// Объявляем константы
Jappsy.assign(Jappsy.Game, {
	STAGE_UNKNOWN: -1,
	STAGE_BARS_WAIT: 0,			// Барабаны во время кручения (ожидаем ответ сервера)
	STAGE_BARS_STOP: 1,			// Барабаны останавливаются
	STAGE_BARS_PRIZE: 2,		// Барабаны остановились (показываем выигрыш)
	STAGE_BARS_FREE: 3,			// Барабаны остановились (показываем бонус фриспин)
	STAGE_BARS_USER: 4,			// Барабаны остановились (ожидаем выбор пользователя)
	STAGE_BARS_TAKE: 5,			// Барабаны забираем накопления в общий счет
	STAGE_BARS: 6,				// Барабаны до кручения
	STAGE_DOUBLE_WAIT: 7,		// Удвоение (ожидаем ответ сервера)
	STAGE_DOUBLE_SHOW: 8,		// Удвоение показ результата
	STAGE_DOUBLE_HIDE: 9,		// Удвоение скрытие результата
	STAGE_DOUBLE_TAKE: 10,		// Удвоение возврат в барабаны
	STAGE_DOUBLE: 11,			// Удвоение (ожидаем выбор пользователя)
});

// Обьявляем класс
Jappsy.Game.prototype = {
	// Список 3д объектов на первом барабане
	barlist1: [
		"ball_0", "cap_0", "bar_00", "bar_01", "bar_02", "separator_00", "separator_01", "separator_02",
	],
	
	// Список 3д объектов на втором барабане
	barlist2: [
		"ball_1", "cap_1", "bar_10", "bar_11", "bar_12", "separator_10", "separator_11", "separator_12",
	],
	
	// Список 3д объектов на третьем барабане
	barlist3: [
		"ball_2", "cap_2", "bar_20", "bar_21", "bar_22", "separator_20", "separator_21", "separator_22",
	],

	startBars: function(freespin) {
		this.bars.start();
		this.updateStage(Jappsy.Game.STAGE_BARS_WAIT);
		this.rollTimeout = new Date().getTime() + 3000;

		if (freespin) {
			this.log("Фриспин - Ускорение всех барабанов");
		} else if (this.conf.auto) {
			this.log("Автоспин - Ускорение всех барабанов");
			this.conf.points -= this.conf.bet;
		} else {
			this.log("Ускорение всех барабанов");
			this.conf.points -= this.conf.bet;
		}
	},
	
	// Обработчики переходов
	updateStage: function(stage, callback, timeout, fade) {
		timeout = timeout || 0;
		var nextTimeout = new Date().getTime() + timeout;
		this.nextTime = (fade === true) ? nextTimeout : 0;

		if (timeout == 0) {
			this.stage = stage;
			this.nextTimeout = 0;
			this.nextStage = stage;
			this.nextCallback = null;
			
			this.updateStageVisibility();
			
			if (callback)
				callback();
		} else {
			this.nextTimeout = nextTimeout;
			this.nextStage = stage;
			this.nextCallback = callback || null;
		}
	},
	
	updateStageVisibility: function() {
		var scene = this.scene;
		
		if (this.stage <= Jappsy.Game.STAGE_BARS) {
			scene.visibleGroup(this.barlist, true);
			scene.visibleGroup(["effects_bar"], true);
			scene.visibleGroup(["effects_torsion", "effects_double", "dbl_sun", "dbl_moon"], false);
		} else {
			scene.visibleGroup(this.barlist, false);
			scene.visibleGroup(["effects_bar"], false);
			scene.visibleGroup(["effects_torsion", "effects_double", "dbl_sun", "dbl_moon"], true);
		}
		
		scene.visibleGroup(this.btnlist, false);
		
		var buttons = ["btn_rewards", "btn_grace", "btn_donate"];
		if (this.stage <= Jappsy.Game.STAGE_BARS) {
			if (this.conf.auto) {
				buttons.push("btn_double");
				buttons.push("btn_stop");
			} else {
				if (this.stage == Jappsy.Game.STAGE_BARS) {
					buttons.push("btn_auto");
					buttons.push("btn_info");
					buttons.push("btn_start");
				} else {
					buttons.push("btn_double");
					buttons.push("btn_take");
				}
			}
		} else if (this.stage <= Jappsy.Game.STAGE_DOUBLE) {
			buttons.push("btn_sun");
			buttons.push("btn_moon");
			buttons.push("btn_take");
		}
		scene.visibleGroup(buttons, true);
	},
	
	// Обработчик кадров и обновление состояние сцены
	onFrame: function(context, canvas, gl) {
		if (this.ready) {
			var scene = this.scene; //this.context.scenes.get("main");
			
			/*
			scene.camera.invalidate()
				.position
					.subtract(scene.camera.target)
					.transform(new Mat4().rotate(new Vec3([0,-1,0]).normalize(), 0.2))
					.add(scene.camera.target);
			*/
			//scene.rotateGroup(this.coinslist, new Vec3([-1,0,0]).normalize(), 5, true);
			
			var currentTime = (new Date).getTime();

			// Переключение сцены по таймауту
			if ((this.nextTimeout != 0) && (currentTime >= this.nextTimeout)) {
				this.nextTimeout = 0;
				this.stage = this.nextStage;
				
				this.updateStageVisibility();
				
				if (this.nextCallback)
					this.nextCallback();
			}
			else if (this.nextTimeout == 0) {
				if (this.stage <= Jappsy.Game.STAGE_BARS) {
					// FIXIT: Временное торможение без ответа от сервера (не забыть про фриспины)
					if ((this.rollTimeout != 0) && (currentTime >= this.rollTimeout)) {
						this.rollTimeout = 0;
						this.log("Торможение всех барабанов");
						var check = this.bars.stopRandom();
						//this.engine.log("Grace: " + check);
						this.updateStage(Jappsy.Game.STAGE_BARS_STOP);
					}
					
					if (this.bars.update()) {
						scene.rotateGroup(this.barlist1, new Vec3([0,-1,0]).normalize(), this.bars.rotation[0].rotate);
						scene.rotateGroup(this.barlist2, new Vec3([0,-1,0]).normalize(), this.bars.rotation[1].rotate);
						scene.rotateGroup(this.barlist3, new Vec3([0,-1,0]).normalize(), this.bars.rotation[2].rotate);
					}
					
					if (this.bars.stopped()) {
						var sum = this.bars.calculate();
						//this.engine.log("Check: " + sum);
						this.log("Барабаны остановились");
						if (sum == 0) {
							if (this.conf.free != 0) {
								this.prizeTimeout = currentTime - 1;
							} else {
								this.updateStage(Jappsy.Game.STAGE_BARS);
							}
						} else if (sum < 0) {
							this.log("Фриспин +" + (-sum));
							this.prizeTimeout = currentTime + 8000; // Время отображения бонус фриспин
							this.updateStage(Jappsy.Game.STAGE_BARS_FREE);
							
							this.conf.prizefree = -sum;
							this.conf.free -= sum;
							this.freespinTime = currentTime;
							
							this.log("Запуск феерверков");
							scene.startParticlesGroup(this.rocketslist, 1);
						} else if (sum > 0) {
							this.log("Приз +" + sum);
							this.prizeTimeout = currentTime + 4000; // Время отображения приза
							this.updateStage(Jappsy.Game.STAGE_BARS_PRIZE);
							
							this.conf.prize = sum * this.conf.bet;
							this.conf.total += this.conf.prize;
						}
					}
					
					if ((this.prizeTimeout != 0) && (this.prizeTimeout < currentTime)) {
						this.prizeTimeout = 0;
						
						if ((this.conf.free > 0) && (this.conf.spin < this.conf.free)) {
							this.conf.spin++;
							
							this.startBars(true);
						} else if ((this.conf.auto) || ((this.conf.free != 0) && (this.conf.total == 0))) {
							if (this.conf.total != 0) {
								this.log("Автокручение - Забрать приз");
							}
							this.updateStage(Jappsy.Game.STAGE_BARS_TAKE);
						} else {
							this.log("Войти в удвоение или забрать приз?");
							this.updateStage(Jappsy.Game.STAGE_BARS_USER);

							this.takeTime = currentTime;
						}
					}
				}
				
				if (this.stage <= Jappsy.Game.STAGE_DOUBLE) {
					// FIXIT: Временный показ дабла без ответа от сервера
					if ((this.selectTimeout != 0) && (currentTime >= this.selectTimeout)) {
						this.log("Удвоение - Открытие затмения");
						this.selectTimeout = 0;
						this.localTime = currentTime;
						this.conf.dblindex = Jappsy.Rotation.generateDouble();
						this.doubleShowTimeout = currentTime + 3000; // Время открытия затмения
						this.updateStage(Jappsy.Game.STAGE_DOUBLE_SHOW);

						var drawing;
						if (this.conf.dblindex == 0) {
							drawing = this.context.drawings.get("bkg_sun");
						} else {
							drawing = this.context.drawings.get("bkg_moon");
						}
						Jappsy.Animation.createFadeIn(drawing, 0.0, 1.0, 2000);
					}

					if ((this.doubleShowTimeout != 0) && (this.doubleShowTimeout < currentTime)) {
						this.doubleShowTimeout = 0;

						this.conf.sequence.splice(0, 1);
						this.conf.sequence.push(this.conf.dblindex);

						if (this.conf.dblselect != this.conf.dblindex) {
							this.log("Удвоение - Проиграно");
							this.conf.doubleout = 0;

							this.updateStage(Jappsy.Game.STAGE_DOUBLE_TAKE);
						} else {
							this.log("Удвоение - Угадано");
							this.conf.doublein = this.conf.doubleout;
							this.conf.doubleout = this.conf.doublein * 2;

							this.log("Удвоение - Закрытие затмения");
							
							this.localTime = currentTime + 3000;
							this.doubleHideTimeout = currentTime + 3000; // Время закрытия затмения
							this.updateStage(Jappsy.Game.STAGE_DOUBLE_HIDE);
							
							var drawing;
							if (this.conf.dblindex == 0) {
								drawing = this.context.drawings.get("bkg_sun");
							} else {
								drawing = this.context.drawings.get("bkg_moon");
							}
							Jappsy.Animation.createFadeOut(drawing, 0.0, 1.0, 2000);
							
							var dbl_sun = this.context.drawings.get("dbl_sun");
							var dbl_moon = this.context.drawings.get("dbl_moon");
							Jappsy.Animation.createLightOut(dbl_sun, 0.3, 1.0, 500);
							Jappsy.Animation.createLightOut(dbl_moon, 0.3, 1.0, 500);
						}
					}
					
					if ((this.doubleHideTimeout != 0) && (this.doubleHideTimeout < currentTime)) {
						this.doubleHideTimeout = 0;
						if (this.conf.dblselect == this.conf.dblindex) {
							this.log("Удвоение - Удвоить или забрать приз?");
							this.updateStage(Jappsy.Game.STAGE_DOUBLE);
						}
						
						var dbl_sun = this.context.drawings.get("dbl_sun");
						var dbl_moon = this.context.drawings.get("dbl_moon");
						var question = this.context.drawings.get("question");

						Jappsy.Animation.createLightOut(dbl_sun, 0.3, 1.0, 500);
						Jappsy.Animation.createLightOut(dbl_moon, 0.3, 1.0, 500);
						Jappsy.Animation.createPingPong(question, 0.75, 1.0, 500);
					}
				}
				
				if (this.stage == Jappsy.Game.STAGE_BARS_TAKE) {
					if (this.conf.total == 0) {
						this.updateStage(Jappsy.Game.STAGE_BARS);
						
						this.conf.prize = 0;
						this.conf.free = 0;
						this.conf.spin = 0;
					} else if (this.takeTimeout < currentTime) {
						var values = [1, 3, 5, 8, 13, 33, 108, 144, 369 ];
						var count = 0;
						if (this.conf.total < 3) {
							count = 1;
						} else if (this.conf.total < 5) {
							count = 2;
						} else if (this.conf.total < 8) {
							count = 3;
						} else if (this.conf.total < 13) {
							count = 4;
						} else if (this.conf.total < 33) {
							count = 5;
						} else if (this.conf.total < 108) {
							count = 6;
						} else if (this.conf.total < 144) {
							count = 7;
						} else if (this.conf.total < 369) {
							count = 8;
						} else {
							count = 9;
						}
						var idx = Jappsy.random(count);
						this.conf.total -= values[idx];
						this.conf.points += values[idx];
						
						var speed = this.conf.total / 5000.0 + 1;
						this.takeTimeout = currentTime + 50 / speed;
					}
				}

				else if (this.stage == Jappsy.Game.STAGE_DOUBLE_TAKE) {
					if ((this.conf.doublein == 0) || (this.conf.doubleout == 0)) {
						this.log("Удвоение - Выход");
						this.updateStage(Jappsy.Game.STAGE_BARS, this.doubleReset, (this.conf.doubleout == 0) ? 3000 : 500, true);
						
						this.conf.total = 0;
						this.conf.prize = 0;
						this.conf.free = 0;
						this.conf.spin = 0;
					} else if (this.takeTimeout < currentTime) {
						var values = [1, 3, 5, 8, 13, 33, 108, 144, 369 ];
						var count = 0;
						if (this.conf.doublein < 3) {
							count = 1;
						} else if (this.conf.doublein < 5) {
							count = 2;
						} else if (this.conf.doublein < 8) {
							count = 3;
						} else if (this.conf.doublein < 13) {
							count = 4;
						} else if (this.conf.doublein < 33) {
							count = 5;
						} else if (this.conf.doublein < 108) {
							count = 6;
						} else if (this.conf.doublein < 144) {
							count = 7;
						} else if (this.conf.doublein < 369) {
							count = 8;
						} else {
							count = 9;
						}
						var idx = Jappsy.random(count);
						this.conf.doublein -= values[idx];
						this.conf.points += values[idx];
						
						var speed = this.conf.doublein / 5000.0 + 1;
						this.takeTimeout = currentTime + 50 / speed;
					}
				}
				
				if ((this.conf.auto) && (this.stage == Jappsy.Game.STAGE_BARS)) {
					this.startBars(false);
				}
			}
			
			scene.render();
		}
		
		if (this.debugfps != null)
			this.engine.logRemove(this.debugfps);
		this.debugfps = this.engine.log(context.frame.fps);
	},
	
	// Обработчик нажатия кнопок на клавиатуре
	onKey: function(e) {
		this.engine.log(e.keyCode);
	},
	
	log: function(e) {
		this.engine.log("<span style='color:#000000'>" + e + "</span>");
	},
	
	onTouch: function(e) {
		if (this.nextTimeout == 0) {
			if (e.indexOf("click ") < 0) {
				this.engine.log(e);
			}

			if (e == "swipe long left") {
				if (this.stage == Jappsy.Game.STAGE_BARS) {
					this.startBars(false);
				}
			}
		}
	},
	
	onTrackBar: function(index, event, cur, delta, speed) {
		if (this.nextTimeout == 0) {
			if (this.stage == Jappsy.Game.STAGE_BARS) {
				var scene = this.scene; //this.context.scenes.get("main");

				this.bars.effects = [];
				if (event == "move") {
					this.bars.rotation[index].userMove(-delta.x / 4);
				} else if (event == "leave") {
					this.bars.rotation[index].userRotate(-delta.x / 4, speed.x * 250);
				}
				return true;
			}
		}
		
		return false;
	},
	
	onButtonEvent: function(event, drawing) {
		if (this.nextTimeout == 0) {
			if (event.indexOf("enter ") == 0) {
				Jappsy.Animation.createBlink(drawing, 1.0, 1.5, 500);
				this.log("Нажата кнопка " + event);
			} else if (event.indexOf("move ") == 0) {
			} else if (event.indexOf("leave ") == 0) {
			} else {
				this.log("Действие по кнопке " + event);
				if (event == "click btn_auto") {
					if (this.stage == Jappsy.Game.STAGE_BARS) {
						this.log("Включен автоспин");
						this.conf.auto = true;
					}
				} else if (event == "click btn_start") {
					if (this.stage == Jappsy.Game.STAGE_BARS) {
						this.startBars(false);
					}
				} else if (event == "click btn_take") {
					if (this.stage == Jappsy.Game.STAGE_BARS_USER) {
						this.log("Забрть приз");
						this.updateStage(Jappsy.Game.STAGE_BARS_TAKE);
					} else if (this.stage == Jappsy.Game.STAGE_BARS_TAKE) {
						this.log("Забран весь приз без ожидания перетекания");
						this.conf.points += this.conf.total;
						this.conf.total = 0;
					} else if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
						this.log("Удвоение - Забрать приз");
						this.updateStage(Jappsy.Game.STAGE_DOUBLE_TAKE);
						
						var question = this.context.drawings.get("question");
						Jappsy.Animation.createFadeOut(question, 0.0, 1.0, 500);
					} else if (this.stage == Jappsy.Game.STAGE_DOUBLE_TAKE) {
						this.log("Забран весь приз без ожидания перетекания");
						if (this.conf.doubleout != 0) {
							this.conf.points += this.conf.doublein;
							this.conf.doublein = 0;
						}
						this.conf.total = 0;
					}
				} else if (event == "click btn_stop") {
					if (this.conf.auto) {
						this.log("Отключен автоспин");
						this.conf.auto = false;
						this.updateStage(this.stage);
					}
				} else if (event == "click btn_double") {
					if (this.stage == Jappsy.Game.STAGE_BARS_USER) {
						this.log("Вход в удвоение");
						this.conf.doublein = this.conf.total;
						this.conf.doubleout = this.conf.total * 2;
						this.updateStage(Jappsy.Game.STAGE_DOUBLE, null, 500, true);
					}
				} else if (event == "click btn_sun") {
					if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
						this.doubleSelect(0);
					}
				} else if (event == "click btn_moon") {
					if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
						this.doubleSelect(1);
					}
				} else {
					this.engine.log(event);
				}
			}
			return true;
		}
		return false;
	},
	
	doubleSelect: function(index) {
		var dbl_sun = this.context.drawings.get("dbl_sun");
		var dbl_moon = this.context.drawings.get("dbl_moon");
		var question = this.context.drawings.get("question");
		
		var drawing1 = (index == 0) ? dbl_sun : dbl_moon;
		var drawing2 = (index != 0) ? dbl_sun : dbl_moon;

		Jappsy.Animation.createLightIn(drawing1, 0.3, 1.0, 500);
		Jappsy.Animation.createLightOut(drawing2, 0.3, 1.0, 500);
		Jappsy.Animation.createFadeOut(question, 0.0, 1.0, 500);
		
		this.updateStage(Jappsy.Game.STAGE_DOUBLE_WAIT);
		this.selectTimeout = new Date().getTime() + 1000;
		
		this.doubleShowTimeout = 0;
		this.doubleHideTimeout = 0;
		this.conf.dblselect = index;

		this.log("Удвоение - Выбор сделан");
	},
	
	doubleReset: function() {
		this.context.drawings.get("dbl_sun").paint.setLight(0.3, 1.0);
		this.context.drawings.get("dbl_moon").paint.setLight(0.3, 1.0);
		this.context.drawings.get("bkg_sun").paint.setLight(0.0, 0.0);
		this.context.drawings.get("bkg_moon").paint.setLight(0.0, 0.0);

		var dbl_sun = this.context.drawings.get("dbl_sun");
		var dbl_moon = this.context.drawings.get("dbl_moon");
		var question = this.context.drawings.get("question");

		Jappsy.Animation.createLightOut(dbl_sun, 0.3, 1.0, 500);
		Jappsy.Animation.createLightOut(dbl_moon, 0.3, 1.0, 500);
		Jappsy.Animation.createPingPong(question, 0.75, 1.0, 500);
	},
	
	onDoubleEvent: function(event, drawing) {
		if (event.indexOf("enter ") == 0) {
			if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
				Jappsy.Animation.createPingPong(drawing, 0.3, 1.0, 500);
				this.log("Нажата кнопка " + event);
			}
		} else if (event.indexOf("move ") == 0) {
		} else if (event.indexOf("leave ") == 0) {
			if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
				Jappsy.Animation.createLightOut(drawing, 0.3, 1.0, 500);
			}
		} else {
			this.log("Действие по кнопке " + event);
			if (event == "click dbl_sun") {
				if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
					this.doubleSelect(0);
				}
			} else if (event == "click dbl_moon") {
				if (this.stage == Jappsy.Game.STAGE_DOUBLE) {
					this.doubleSelect(1);
				}
			} else {
				this.engine.log(event);
			}
		}
		return true;
	},

	// Вывод спрайтов
	renderButtonGrace: function(drawing) {
		this.context.sprites.renderSpriteNumber("num_sm", [344+7, 928+66], 30, this.conf.points);
	},

	renderButtonDonate: function(drawing) {
		this.context.sprites.renderSpriteNumber("num_sm", [760+7, 928+66], 30, this.conf.bet);
	},
	
	renderButtonDouble: function(drawing) {
		var context = this.context;
		
		var state = 0;
		if (this.conf.auto) {
			if ((this.stage == Jappsy.Game.STAGE_BARS) || (this.stage <= Jappsy.Game.STAGE_BARS_STOP)) {
				state = 2;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_PRIZE) {
				state = 1;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_FREE) {
				state = 2;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_USER) {
				state = 0;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_TAKE) {
				state = 2;
			}
		} else {
			if (this.stage == Jappsy.Game.STAGE_BARS) {
			} else if (this.stage <= Jappsy.Game.STAGE_BARS_STOP) {
				state = 2;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_PRIZE) {
				state = 1;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_FREE) {
				state = 2;
			} else if (this.stage == Jappsy.Game.STAGE_BARS_USER) {
				var currentTime = (new Date).getTime();
				var doubleBlink = Math.floor((currentTime - this.takeTime) / 500);
				if ((doubleBlink & 1) == 0) {
					state = 0;
				} else {
					state = 2;
				}
			} else if (this.stage == Jappsy.Game.STAGE_BARS_TAKE) {
				state = 2;
			}
		}
		
		if (state == 1) {
			drawing.foreground = 5;
			if (this.conf.spin != 0) {
				context.sprites.renderSpriteNumber("num_sm", [1576-7, 928+7], -30, this.conf.spin.toString(10) + ";" + this.conf.free.toString(10));
			}
			if (this.conf.prize > 0) {
				context.sprites.renderSpriteNumber("num_sm", [1576-7, 928+66], -30, ":" + this.conf.prize.toString(10));
			}
		} else if (state == 2) {
			drawing.foreground = 5;
			if (this.conf.free != 0) {
				var currentTime = (new Date).getTime();
				var freeBlink = Math.floor((currentTime - this.freespinTime) / 500);
				if ((freeBlink >= 8) || ((freeBlink & 1) == 0)) {
					if (this.conf.spin == 0) {
						context.sprites.renderSpriteNumber("num_sm", [1576-7, 928+7], -30, this.conf.free.toString(10));
					} else {
						context.sprites.renderSpriteNumber("num_sm", [1576-7, 928+7], -30, this.conf.spin.toString(10) + ";" + this.conf.free.toString(10));
					}
				} else {
					if (this.conf.spin != 0) {
						var len = this.conf.free.toString(10).length;
						context.sprites.renderSpriteNumber("num_sm", [1576-7-30*len, 928+7], -30, this.conf.spin.toString(10) + ";");
					}
				}
			}
			context.sprites.renderSpriteNumber("num_sm", [1176+7, 928+66], 30, this.conf.total.toString(10));
		} else {
			drawing.foreground = 6;
			context.sprites.renderSpriteNumber("num_sm", [1176+7, 928+66], 30, this.conf.total);
		}
	},
};

Jappsy.Game.prototype.release = function() {
	this.engine = null;
	this.context = null;
};

Jappsy.Game.prototype.load = function() {
	this.context.loader.load({
		"groups": {
			"shaders": {
				"vsh_main":					"shaders/vsh_main.jsh",
				"vsh_texture":				"shaders/vsh_texture.jsh",
				"square_fill":				"shaders/square_fill.jsh",
				"square_stroke":			"shaders/square_stroke.jsh",
				"square_texture":			"shaders/square_texture.jsh",
				"sprite":					"shaders/sprite.jsh",
				"particle":					"shaders/particle.jsh",
				"model":					"shaders/model.jsh",
				
				"vsh_effect":				"shaders/vsh_effect.jsh",
				"effect_torsion":			"shaders/effect_torsion.jsh",
				"effect_sun1":				"shaders/effect_sun1.jsh",
				"effect_moon1":				"shaders/effect_moon1.jsh",
				"effect_unknown":			"shaders/effect_unknown.jsh",
				"effect_unknown_star":		"shaders/effect_unknown_star.jsh",
				"effect_sun2":				"shaders/effect_sun2.jsh",
				"effect_moon2":				"shaders/effect_moon2.jsh",
				"effect_star1":				"shaders/effect_star1.jsh",
				"effect_star2":				"shaders/effect_star2.jsh",
				
				"fsh_effect_test":			"shaders/fsh_effect_test.fsh",
				
				"mobile_effect_torsion":				"shaders/mobile_effect_torsion.jsh",
				"mobile_effect_sun1":					"shaders/mobile_effect_sun1.jsh",
				"mobile_effect_moon1":					"shaders/mobile_effect_moon1.jsh",
				"mobile_effect_unknown":				"shaders/mobile_effect_unknown.jsh",
				"mobile_effect_unknown_star":			"shaders/mobile_effect_unknown_star.jsh",
				"mobile_effect_sun2":					"shaders/mobile_effect_sun2.jsh",
				"mobile_effect_moon2":					"shaders/mobile_effect_moon2.jsh",
				"mobile_effect_star1":					"shaders/mobile_effect_star1.jsh",
				"mobile_effect_star2":					"shaders/mobile_effect_star2.jsh",
			},
			
			"textures": {
				"null":						"models/null.jimg",
				"mramor.png":				"models/mramor.jimg",
				"metal.png":				"models/metal.jimg",
				"gold.png":					"models/gold.jimg",
				"cox":						"models/cox.jimg",

				"background1":				"images/background1.jimg",
				"background2":				"images/background2.jimg",
				"background3":				"images/background3.jimg",
				"background4":				"images/background4.jimg",
				"buttons":					"images/buttons.jimg",
				"sprites":					"images/sprites.jimg",
				
				"mobile_tex_effect_star":				"shaders/mobile_tex_effect_star.jimg",
				"mobile_tex_random":					"shaders/mobile_tex_random.jimg",
			},
			
			"models": {
				"ball":						"models/ball.json",
				"cap":						"models/cap.json",
				"separator":				"models/separator.json",
				"bar":						"models/bar.json",
			},
		}
	});
	
	this.context.loader.onfile = this.onLoadFile.bind(this);
	this.context.loader.onstatus = this.onLoadStatus.bind(this);
	this.context.loader.onload = this.onLoadReady.bind(this);
	this.context.loader.onerror = this.onLoadError.bind(this);
	
	this.context.loader.run();
};

Jappsy.Game.prototype.onLoadFile = function(info, data) {
	//this.engine.log("file " + info.file);
};

Jappsy.Game.prototype.onLoadStatus = function(status) {
	//this.engine.log("status: " + status.count + " / " + status.total);
};

Jappsy.Game.prototype.onLoadReady = function(result) {
	// Подготавливаем тестовые шейдеры
	if (
		!this.context.createShaders({
			effect_test: [ "vsh_effect", "fsh_effect_test" ],
		})
	) {
		throw 'Create Shaders Failed';
	}
	
	// Подготавливаем модели
	this.context.createModels(result['models']);
	
	// Подготавливаем барабаны
	this.bars = new Jappsy.Bars();
	
	// Подготавливаем спрайты из текстур
	var sprites = {
		background1: [ "background1", [1980, 1080] ],
		background2: [ "background2", [1980, 1080] ],
		background3: [ "background3", [1980, 1080] ],
		background4: [ "background4", [1980, 1080] ],
	
		buttons1: [ "buttons", [312,136], [16,16], 6, [0, 1] ],		// Rewards / Start / Take / Stop
		buttons2: [ "buttons", [400,136], [344,16], 7, [0, 1] ],	// Grace / Donate / Grace In / Double
		buttons3: [ "buttons", [192,136], [760,16], 6, [0, 1] ],	// Auto / Info / Sun / Moon
		
		anchors: [ "sprites", [128,128], [768,0], 2, [1, 0] ],
		text_om: [ "sprites", [144,64], [0,960], 1 ],
		text_mani: [ "sprites", [216,64], [144,960], 1],
		text_padme: [ "sprites", [296,64], [360,960], 1],
		text_hum: [ "sprites", [192,64], [656,960], 1],
		
		num_sm: [ "sprites", [64,64], [0,0], 12, [1, 0] ],			// Small Numbers
		num: [ "sprites", [64,64], [0,64], 10, [1, 0] ],			// Numbers
		
		text_double: [ "sprites", [464,48], [0, 128], 3, [0, 1] ],			// Grace In / Grace Out / Recent
		text_free: [ "sprites", [560,64], [0,272], 2, [0, 1] ],		// 4/8 Free Spins
		
		sequence: [ "sprites", [176,144], [848,128], 2, [0, 1] ],	// Sequence Sun / Moon
		question: [ "sprites", [160,176], [864,416], 1],			// Question
		sunmoon: [ "sprites", [328,320], [0,640], 2, [1, 0] ],		// Sun / Moon
	};
	this.context.createSprites(sprites);

	// Подготавливаем картинки из спрайтов
	var drawings = {
		bkg_bars: [ "background1", [0, 0] ],
		bkg_double: [ "background2", [0, 0] ],
		bkg_sun: [ "background3", [0, 0] ],
		bkg_moon: [ "background4", [0, 0] ],
	
		anchor1: [ "anchors", [40,38], 0 ],
		anchor2: [ "anchors", [1920-40-128,38], 1 ],

		text_om: [ "text_om", [245,72], 0 ],
		text_mani: [ "text_mani", [625,72], 0 ],
		text_padme: [ "text_padme", [1029,72], 0 ],
		text_hum: [ "text_hum", [1500,72], 0 ],
		
		text_free4: [ "text_free", [(1920 - 560)/2, (1080 - 64)/2 + 32], 0 ],
		text_free8: [ "text_free", [(1920 - 560)/2, (1080 - 64)/2 + 32], 1 ],

		text_doublein: [ "text_double", [216, 40], 0 ],
		text_doubleout: [ "text_double", [970, 40], 1 ],

		text_sequence: [ "text_double", [216, 178+152/2], 2, this.paintSpriteLeftCenter ],

		btn_rewards: [ "buttons1", [16, 928], [0, 0, 0, 2] ],
		btn_grace: [ "buttons2", [344, 928], [0, 0, 0, 3] ],
		btn_donate: [ "buttons2", [760, 928], [1, 1, 1, 4] ],
		btn_auto: [ "buttons3", [1176, 928], [0, 0, 0, 2] ],
		btn_info: [ "buttons3", [1384, 928], [1, 1, 1, 3] ],
		//btn_spin: [ "buttons2", [1176, 928], [2, 2, 2, 5] ], //INFO: в renderButtonDouble подставляется номер
		btn_double: [ "buttons2", [1176, 928], [2, 2, 2, 6] ], //INFO: в renderButtonDouble подставляется номер
		btn_start: [ "buttons1", [1592, 928], [1, 1, 1, 3] ],
		btn_take: [ "buttons1", [1592, 928], [1, 1, 1, 4] ],
		btn_stop: [ "buttons1", [1592, 928], [1, 1, 1, 5] ],
		btn_sun: [ "buttons3", [1176, 928], [0, 0, 0, 4] ],
		btn_moon: [ "buttons3", [1384, 928], [1, 1, 1, 5] ],
		
		dbl_sun: [ "sunmoon", [960 - 520, 606], 0, this.paintSpriteCenter ],
		dbl_moon: [ "sunmoon", [960 + 520, 606], 1, this.paintSpriteCenter ],
		question: [ "question", [960, 606], 0, this.paintSpriteCenter ],
	};
	this.context.createDrawings(drawings);
	
	// Добавляем наложения на кнопки
	this.context.drawings.get("btn_grace").onrender = this.renderButtonGrace.bind(this);
	this.context.drawings.get("btn_donate").onrender = this.renderButtonDonate.bind(this);
	this.context.drawings.get("btn_double").onrender = this.renderButtonDouble.bind(this);

	// Задаем свет для спрайтов по умолчанию
	this.context.light = new Vec4([1,1,1,1.1]);
	
	var scene = this.scene = this.context.scenes.createScene("main");
	scene.ambient.set([0,0,0]);
	
	var cam = this.context.cameras.createCamera("cam")
		.size(1920, 1080)
		.perspective(45, 0.1, 100)
		.lookAt([317.207, 347.124, 1351.136], [317.207, 347.124, -249.692], [0,1,0]);
		//.lookAt([0,0,500], [0,0,0], [0,1,0]);
	scene.setCamera(cam);
	
	{ // Освещение сцены
		scene.lights.createLight("Omni001").omni({
			position: [-554.5, 701.5, -246],
			color: [1, 1, 1],
			intensivity: 0.54,
			radius: 0,
			falloff: 0,
			fixed: false
		});
		scene.lights.createLight("Omni002").omni({
			position: [1183.5, 378.5, 109.5],
			color: [1, 1, 1],
			intensivity: 0.3,
			radius: 0,
			falloff: 0,
			fixed: false
		});
		scene.lights.createLight("Spot001").spot({ // Shadows :(
			position: [783.9, 359.2, 619.6],
			target: [47.2, 256.3, -536.5],
			color: [1, 1, 1],
			intensivity: 1,
			radius: 111.7,
			falloff: 2,
			fixed: false
		});
		scene.lights.createLight("Spot002").spot({ // Shadows :(
			position: [-182, 578.2, 488],
			target: [256.4, 352.5, -830.5],
			color: [1, 1, 1],
			intensivity: 0.63,
			radius: 77.2,
			falloff: 2,
			fixed: false
		});
	}
	
	var backLayer = scene.createLayer();
	var modelLayer = scene.createLayer();
	var effectLayer = scene.createLayer();
	var rocketLayer = scene.createLayer();
	var splashLayer = scene.createLayer();
	var buttonLayer = scene.createLayer();
	var blackLayer = scene.createLayer();
	
	/*
	this.createCoins = function() {
		var w = 1.0/3.0;
		var h = 1.0/4.0;
		var r = 24;
		var s = 2;
		var c = 16;
		var p = [317.207 - 60 * 4.5, 327.124, 0];
		
		this.coinslist = [];
		
		var names = [1, 3, 5, 8, 13, 33, 108, 144, 369 ];
		for (var i = 0; i < 9; i++) {
			var x = Math.floor(i / 3);
			var y = i - x * 3 + 1;
			var name = "coin" + names[i];
			var modelName = name + ".mdl";
			var model = this.context.models.createModel(modelName);
			var mat1 = model.createMaterial("cox", w*x, 0, w, h, Jappsy.UVMAP_CYLINDER);
			var mat2 = model.createMaterial("cox", w*x, h*y, w, h, Jappsy.UVMAP_CYLINDER);
			model.createCoin(r, s, c, [mat1, mat2]);
			scene.createLayerObject(modelLayer, name).setModel(modelName).objectMatrix.translate(p);
			this.coinslist.push(name);
			p[0] += 60;
		}
	}
	
	this.createCoins();
	scene.rotateGroup(this.coinslist, new Vec3([0,1,0]).normalize(), 180, true);
	*/

	{ // Феерверки
		this.rocketslist = [];
		
		for (var i = 0; i < 10; i++) {
			var name = "rocket" + i;
			var s = this.context.particles.createParticleSystem(name);
			s.createRocket(0, new Vec3([0,0,0]));
			s.generate();
			this.rocketslist.push(name);
			
			var p = new Vec3().subtract(scene.camera.target, [0,250,0]);
			scene.createLayerObject(rocketLayer, name).setParticleSystem(name).objectMatrix.translate(p);
		}
		
		//scene.startParticlesGroup(this.rocketslist, 2);
		scene.visibleGroup(this.rocketslist, true);
	}

	{ // Модели
		scene.createLayerObject(modelLayer, "ball_0").setModel("ball").objectMatrix.translate([1.947, 620.468, -252.468]);
		scene.createLayerObject(modelLayer, "cap_0").setModel("cap").objectMatrix.translate([4.12, 572.537, -249.301]);
		scene.createLayerObject(modelLayer, "bar_00").setModel("bar").objectMatrix.translate([3.397, 477.718, -249.692])
			.multiply(new Mat4().rotate([0,-1,0], 180));
		scene.createLayerObject(modelLayer, "separator_00").setModel("separator").objectMatrix.translate([3.049, 401.208, -248.473]);
		scene.createLayerObject(modelLayer, "bar_01").setModel("bar").objectMatrix.translate([3.397, 327.124, -249.692])
			.multiply(new Mat4().rotate([0,-1,0], 90));
		scene.createLayerObject(modelLayer, "separator_01").setModel("separator").objectMatrix.translate([3.049, 252.739, -248.473]);
		scene.createLayerObject(modelLayer, "bar_02").setModel("bar").objectMatrix.translate([3.397, 179.488, -249.692]);
		scene.createLayerObject(modelLayer, "separator_02").setModel("separator").objectMatrix.translate([3.049, 106.063, -248.473]);
		
		scene.createLayerObject(modelLayer, "ball_1").setModel("ball").objectMatrix.translate([315.811, 620.468, -252.468]);
		scene.createLayerObject(modelLayer, "cap_1").setModel("cap").objectMatrix.translate([317.984, 572.537, -249.301]);
		scene.createLayerObject(modelLayer, "bar_10").setModel("bar").objectMatrix.translate([317.207, 477.718, -249.692])
			.multiply(new Mat4().rotate([0,-1,0], 180));
		scene.createLayerObject(modelLayer, "separator_10").setModel("separator").objectMatrix.translate([316.914, 401.208, -248.473]);
		scene.createLayerObject(modelLayer, "bar_11").setModel("bar").objectMatrix.translate([317.207, 327.124, -249.692])
			.multiply(new Mat4().rotate([0,-1,0], 90));
		scene.createLayerObject(modelLayer, "separator_11").setModel("separator").objectMatrix.translate([316.914, 252.739, -248.473]);
		scene.createLayerObject(modelLayer, "bar_12").setModel("bar").objectMatrix.translate([317.207, 179.488, -249.692]);
		scene.createLayerObject(modelLayer, "separator_12").setModel("separator").objectMatrix.translate([316.914, 106.063, -248.473]);

		scene.createLayerObject(modelLayer, "ball_2").setModel("ball").objectMatrix.translate([626.263, 620.468, -252.468]);
		scene.createLayerObject(modelLayer, "cap_2").setModel("cap").objectMatrix.translate([628.436, 572.537, -249.301]);
		scene.createLayerObject(modelLayer, "bar_20").setModel("bar").objectMatrix.translate([632.114, 477.718, -249.692])
			.multiply(new Mat4().rotate([0,-1,0], 180));
		scene.createLayerObject(modelLayer, "separator_20").setModel("separator").objectMatrix.translate([631.676, 401.208, -248.473]);
		scene.createLayerObject(modelLayer, "bar_21").setModel("bar").objectMatrix.translate([632.114, 327.124, -249.692])
			.multiply(new Mat4().rotate([0,-1,0], 90));
		scene.createLayerObject(modelLayer, "separator_21").setModel("separator").objectMatrix.translate([631.676, 252.739, -248.473]);
		scene.createLayerObject(modelLayer, "bar_22").setModel("bar").objectMatrix.translate([632.114, 179.488, -249.692]);
		scene.createLayerObject(modelLayer, "separator_22").setModel("separator").objectMatrix.translate([631.676, 106.063, -248.473]);

		scene.rotateGroup(this.barlist1, new Vec3([0,-1,0]).normalize(), -11, true);
		scene.rotateGroup(this.barlist3, new Vec3([0,-1,0]).normalize(), 11, true);
		//scene.rotateGroup(this.barlist, new Vec3([0,-1,0]).normalize(), 45*3);
	}
	
	{ // Фон
		scene.createLayerObject(backLayer, "background").setFunc(function(object){
			var context = object.scene.context;
			var game = context.engine.extension;
			//context.drawTexture(0, 0, 1920, 1080, "background1");
			
			if (game.stage <= Jappsy.Game.STAGE_BARS) {
				context.drawings.renderDrawing("bkg_bars", 0);
			} else if (game.stage <= Jappsy.Game.STAGE_DOUBLE) {
				context.drawings.renderDrawing("bkg_double", 0);
				if ((game.stage == Jappsy.Game.STAGE_DOUBLE_SHOW) || (game.stage == Jappsy.Game.STAGE_DOUBLE_HIDE) || (game.stage == Jappsy.Game.STAGE_DOUBLE_TAKE)) {
					if (game.conf.dblindex == 0) {
						context.drawings.renderDrawing("bkg_sun", 0);
					} else {
						context.drawings.renderDrawing("bkg_moon", 0);
					}
				}
			}
		});
	}
	
	{ // Тексты
		scene.createLayerObject(splashLayer, "splash").setFunc(function(object){
			var context = object.scene.context;
			var game = context.engine.extension;
			
			if (game.stage <= Jappsy.Game.STAGE_BARS) {
				context.drawings.renderDrawing("text_om");
				context.drawings.renderDrawing("text_mani");
				context.drawings.renderDrawing("text_padme");
				context.drawings.renderDrawing("text_hum");
			}
			
			if (game.stage == Jappsy.Game.STAGE_BARS_FREE) {
				if (game.conf.prizefree == 4)
					context.drawings.renderDrawing("text_free4");
				else
					context.drawings.renderDrawing("text_free8");
			}

			if (game.stage <= Jappsy.Game.STAGE_BARS) {
			} else if (game.stage <= Jappsy.Game.STAGE_DOUBLE) {
				context.drawings.renderDrawing("text_doublein");
				if ((game.stage != Jappsy.Game.STAGE_DOUBLE_TAKE) || (game.conf.doubleout == 0)) {
					context.drawings.renderDrawing("text_doubleout");
				}
				
				var currentTime = (new Date).getTime();
				var state = 0;
				if (game.stage == Jappsy.Game.STAGE_DOUBLE_SHOW) {
					var doubleBlink = Math.floor((currentTime - game.doubleShowTimeout) / 500);
					if ((doubleBlink & 1) != 0) {
						state = 1;
					}
				}
				if (state == 0) {
					context.sprites.renderSpriteNumber("num", [216-4, 100], 42, game.conf.doublein);
					if ((game.stage != Jappsy.Game.STAGE_DOUBLE_TAKE) || (game.conf.doubleout == 0)) {
						context.sprites.renderSpriteNumber("num", [970-4, 100], 42, game.conf.doubleout);
					}
				}

				context.drawRect(0, 178, 1920, 178+152, game.paintShadow);
				context.drawings.renderDrawing("text_sequence", 0);
				var x = 900;
				if (game.stage == Jappsy.Game.STAGE_DOUBLE_SHOW) {
					var ofs = (currentTime - game.doubleShowTimeout + 3000) / 2000;
					if (ofs > 1) ofs = 1;
					game.paintSpriteLeftCenter.setLight(ofs, ofs);
					context.sprites.renderSprite("sequence", [x - 176 + ofs * 176, 178+152/2], game.conf.dblindex, game.paintSpriteLeftCenter, 0);
					game.paintSpriteLeftCenter.setLight(1.0 - ofs, 1.0 - ofs);
					context.sprites.renderSprite("sequence", [x + 176 * 4 + ofs * 176, 178+152/2], game.conf.sequence[0], game.paintSpriteLeftCenter, 0);
					game.paintSpriteLeftCenter.setLight(1.0, 1.0);
					for (var i = 0; i < 4; i++) {
						context.sprites.renderSprite("sequence", [x + 176 * i + ofs * 176, 178+152/2], game.conf.sequence[4-i], game.paintSpriteLeftCenter, 0);
					}
				} else {
					for (var i = 0; i < 5; i++) {
						context.sprites.renderSprite("sequence", [x + 176 * i, 178+152/2], game.conf.sequence[4-i], game.paintSpriteLeftCenter, 0);
					}
				}
			}
		});
	}
	
	{ // Анимация на барабанах
		scene.createLayerObject(effectLayer, "effects_bar").setFunc(function(object){
			var context = object.scene.context;
			var game = context.engine.extension;
			
			game.bars.renderEffects(context);
		});
	}
	
	{ // Торсион
		scene.createLayerObject(effectLayer, "effects_torsion").setFunc(function(object){
			var context = object.scene.context;
			var game = context.engine.extension;
			
			var mobile = true;
			var worldTime = (new Date).getTime() / 86400000.0;
			
			if (mobile) {
				context.drawEffectMobile(960-700, 606-700, 960+700, 606+700, "mobile_effect_torsion", worldTime, worldTime);
			} else {
				context.drawEffect(960-700, 606-700, 960+700, 606+700, "effect_torsion", worldTime, worldTime);
			}
		});
	}

	{ // Картинки и Кнопки
		scene.createLayerDrawing(splashLayer, "anchor1");
		scene.createLayerDrawing(splashLayer, "anchor2");

		this.btnlist = [ "btn_rewards", "btn_grace", "btn_donate", "btn_auto", "btn_info", "btn_double", "btn_start", "btn_take", "btn_stop", "btn_sun", "btn_moon" ];
		scene.createLayerDrawing(buttonLayer, this.btnlist);
		scene.objects.trackEvents(this.btnlist, this.onButtonEvent.bind(this));
		
		this.dbllist = [ "dbl_sun", "dbl_moon" ];
		scene.createLayerDrawing(effectLayer, this.dbllist, 0);
		scene.objects.trackEvents(this.dbllist, this.onDoubleEvent.bind(this));
	}
	
	{ // Анимация в удвоении
		scene.createLayerObject(effectLayer, "effects_double").setFunc(function(object){
			var context = object.scene.context;
			var game = context.engine.extension;
			
			//game.paint.setColor(0x80FF0000);
			//context.drawRect(10, 10, 100, 100, game.paint);
			//game.paint.setColor(0x8000FF00);
			//context.drawRect(50, 50, 140, 140, game.paint);
			
			
			var mobile = false;
			var index = game.conf.dblindex;
			
			var currentTime = new Date().getTime();
			var worldTime = currentTime / 86400000.0;

			var localTime;
			if (game.stage != Jappsy.Game.STAGE_DOUBLE_HIDE) {
				localTime = (currentTime - game.localTime) / 86400000.0;
			} else {
				localTime = (game.localTime - currentTime) / 86400000.0;
			}
			
			if ((game.stage == Jappsy.Game.STAGE_DOUBLE) || (game.stage == Jappsy.Game.STAGE_DOUBLE_WAIT) || ((game.stage == Jappsy.Game.STAGE_DOUBLE_TAKE) && (game.conf.doubleout != 0))) {
				if (mobile) {
					context.drawEffectMobile(960-320, 606-320, 960+320, 606+320, "mobile_effect_unknown", 0, worldTime);
					context.drawEffectMobile(1050-120, 490-120, 1050+120, 490+120, "mobile_effect_unknown_star", 0, worldTime);
				} else {
					context.drawEffect(960-320, 606-320, 960+320, 606+320, "effect_unknown", 0, worldTime);
					context.drawEffect(1050-120, 490-120, 1050+120, 490+120, "effect_unknown_star", 0, worldTime);
				}
				
				context.drawings.renderDrawing("question", 0);
			} else if ((game.stage == Jappsy.Game.STAGE_DOUBLE_SHOW) || (game.stage == Jappsy.Game.STAGE_DOUBLE_HIDE) || (game.stage == Jappsy.Game.STAGE_DOUBLE_TAKE)) {
				if (mobile) {
					if (index == 0) {
						context.drawEffectMobile(960-320, 606-320, 960+320, 606+320, "mobile_effect_sun2", localTime, worldTime);
					} else {
						context.drawEffectMobile(960-320, 606-320, 960+320, 606+320, "mobile_effect_moon2", localTime, worldTime);
						context.drawEffectMobile(220-200, 450-200, 220+200, 450+200, "mobile_effect_star1", localTime, worldTime);
						context.drawEffectMobile(450-150, 820-150, 450+150, 820+150, "mobile_effect_star2", localTime, worldTime + 0.3/86400);
						context.drawEffectMobile(1700-200, 750-200, 1700+200, 750+200, "mobile_effect_star1", localTime, worldTime + 0.6/86400);
					}

					context.drawEffectMobile(1050-120, 490-120, 1050+120, 490+120, "mobile_effect_unknown_star", localTime, worldTime);
				} else {
					if (index == 0) {
						context.drawEffect(960-320, 606-320, 960+320, 606+320, "effect_sun2", localTime, worldTime);
					} else {
						context.drawEffect(960-320, 606-320, 960+320, 606+320, "effect_moon2", localTime, worldTime);
						context.drawEffect(220-200, 450-200, 220+200, 450+200, "effect_star1", localTime, worldTime);
						context.drawEffect(450-150, 820-150, 450+150, 820+150, "effect_star2", localTime, worldTime + 0.3/86400);
						context.drawEffect(1700-200, 750-200, 1700+200, 750+200, "effect_star1", localTime, worldTime + 0.6/86400);
					}

					context.drawEffect(1050-120, 490-120, 1050+120, 490+120, "effect_unknown_star", localTime, worldTime);
				}
			}
		});
	}
	
	{ // Затемнение экрана при переключении сцены
		scene.createLayerObject(blackLayer, "effects_switch").setFunc(function(object){
			var context = object.scene.context;
			var game = context.engine.extension;
			
			if (this.nextTime != 0) {
				var currentTime = new Date().getTime();
				var ofs = (currentTime - game.nextTime) / 500;
				if (ofs >= 1) {
					game.nextTime = 0;
				} else {
					game.paintSwitch.setLight(0, 1.0 - Jappsy.clamp(Math.abs(ofs), 0, 1));
					context.drawRect(0, 0, 1920, 1080, game.paintSwitch);
				}
			}
		});
	}

	this.updateStage(Jappsy.Game.STAGE_BARS, null, 0, true);
	//this.updateStage(Jappsy.Game.STAGE_DOUBLE, null, 0, true);
	this.doubleReset();

	this.context.touchScreen.trackEvent(null, 276, 192, 456, 720, this.onTrackBar.bind(this, 0));
	this.context.touchScreen.trackEvent(null, 732, 192, 456, 720, this.onTrackBar.bind(this, 1));
	this.context.touchScreen.trackEvent(null, 1188, 192, 456, 720, this.onTrackBar.bind(this, 2));

	window.onkeyup = this.onKey.bind(this);

	this.ready = true;
};

Jappsy.Game.prototype.onLoadError = function(e) {
	//this.engine.log("error " + e);
	if (this.engine.onerror) {
		this.engine.onerror(e);
	}
};

