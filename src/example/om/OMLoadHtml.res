R"JAPPSYRAWSTRING(
<!DOCTYPE html>
<html>
<head>
<meta content="text/xml; charset=utf-8" http-equiv="Content-Type" />
<meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no" />
<meta name="apple-mobile-web-app-capable" content="yes" />
<meta name="apple-mobile-web-app-status-bar-style" content="black" />
<meta name="mobile-web-app-capable" content="yes" />

<style>

	body {
		background: #000;
		width: 100%;
		height: 100%;
		padding: 0;
		margin: 0;
		position: absolute;
		top: 0;
		left: 0;
		
		font: normal 0.18in sans-serif;
		color: #fff;

		-webkit-touch-callout: none; 	/* iOS Safari */
		-webkit-user-select: none;   	/* Chrome/Safari/Opera */
		-khtml-user-select: none;		/* Konqueror */
		-moz-user-select: none;	  		/* Firefox */
		-ms-user-select: none;	   		/* Internet Explorer/Edge */
		user-select: none;		   		/* Non-prefixed version, currently not supported by any browser */

		cursor: default;
	}

	body > table {
		background: #000;
		width: 100%;
		height: 100%;
	}

	.navigation {
		background: #000  url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAEAAABkCAYAAABHLFpgAAAACXBIWXMAAAsTAAALEwEAmpwYAAABNmlDQ1BQaG90b3Nob3AgSUNDIHByb2ZpbGUAAHjarY6xSsNQFEDPi6LiUCsEcXB4kygotupgxqQtRRCs1SHJ1qShSmkSXl7VfoSjWwcXd7/AyVFwUPwC/0Bx6uAQIYODCJ7p3MPlcsGo2HWnYZRhEGvVbjrS9Xw5+8QMUwDQCbPUbrUOAOIkjvjB5ysC4HnTrjsN/sZ8mCoNTIDtbpSFICpA/0KnGsQYMIN+qkHcAaY6addAPAClXu4vQCnI/Q0oKdfzQXwAZs/1fDDmADPIfQUwdXSpAWpJOlJnvVMtq5ZlSbubBJE8HmU6GmRyPw4TlSaqo6MukP8HwGK+2G46cq1qWXvr/DOu58vc3o8QgFh6LFpBOFTn3yqMnd/n4sZ4GQ5vYXpStN0ruNmAheuirVahvAX34y/Axk/96FpPYgAAACBjSFJNAAB6JQAAgIMAAPn/AACA6AAAUggAARVYAAA6lwAAF2/XWh+QAAAAHElEQVR42mJgYGD4z8TAwMAwSgxLwl01Gl0MMAC/aAKN62kIcAAAAABJRU5ErkJggg==') no-repeat scroll 0 -0.5px / 100% 0.5in;
		height: 0.48in;
	}
	
	.arrow-left {
		position: absolute;
		left: 0;
		top: 0;
		width: 0.48in;
		height: 0.48in;
		background: transparent url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAMAAAGD56FvAAAACXBIWXMAAAsTAAALEwEAmpwYAAABNmlDQ1BQaG90b3Nob3AgSUNDIHByb2ZpbGUAAHjarY6xSsNQFEDPi6LiUCsEcXB4kygotupgxqQtRRCs1SHJ1qShSmkSXl7VfoSjWwcXd7/AyVFwUPwC/0Bx6uAQIYODCJ7p3MPlcsGo2HWnYZRhEGvVbjrS9Xw5+8QMUwDQCbPUbrUOAOIkjvjB5ysC4HnTrjsN/sZ8mCoNTIDtbpSFICpA/0KnGsQYMIN+qkHcAaY6addAPAClXu4vQCnI/Q0oKdfzQXwAZs/1fDDmADPIfQUwdXSpAWpJOlJnvVMtq5ZlSbubBJE8HmU6GmRyPw4TlSaqo6MukP8HwGK+2G46cq1qWXvr/DOu58vc3o8QgFh6LFpBOFTn3yqMnd/n4sZ4GQ5vYXpStN0ruNmAheuirVahvAX34y/Axk/96FpPYgAAACBjSFJNAAB6JQAAgIMAAPn/AACA6AAAUggAARVYAAA6lwAAF2/XWh+QAAAAeFBMVEX/zAD/////zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAB/UgzsAAAAJ3RSTlMAABIeISQnKi0wMzY5PD9CRUhLVFdahJmcn8PP0tjh5Ofq7fDz9vkTwM98AAACUUlEQVR42u2YW0PjIBCFj2va2Etqq7uuq67WtOn5///QB7QCacAAIV6GRzLzzYEMwwVnVkO6DgC8NztIGB0kjA4ScFu0GQAfUksfbD6sDi7NDq5hdPAGsCwu4WHYUdKO5dMbAABI/nIYkFwBnQYkN0CMAUCychq4RX6JqRaDpEk7J50GM7oNLvhWGk8blDwW05MGE76X31MGhfrebfD6PYKAqUeDfxT+efDPpPtfSNJGGAhAAFEAaG1hr/R+gDlJchsKmJEkuQxUUCr3yip3HwVMlfuqVXE/Bpgo983rWPoCzpX71XEu+wIKM37EENbBgOhJTPAbEyRSglQOXkxSkQQgAAEIQAAC6NxbUZO3MYAdrStrT0DNOEDtPF/4Ac8kyR1CAcp/j1CA8m8QCtge/cMAmn8Q4EnzDwEY/gEA5X9AKODR9O8NuFNHZAQD7BEEz0ETDoj+C/F5EJ+J+lqKW437cEB0PYivSPE1Mb4qx+8LQE3+k+1dAAIQwA8AiAARIAJEgAgQASJABPw4Aehsxe3/5u4PMJKAxU698fB6FAHzt/BkM4KAWc339je7ACP8bpE7By6etfD7JTInYWmEr5xvYwMIKLda+KbyPA8mFzA1wq+8L6SJBUyf9PCXQF4BEyP82siLDAIKPfxhY62LwQUUj3r4q1ZdGFjAuSf84AKKsQX4MiBLEpprYJNfgF0F1vkFdNfBjKW4HLcUt/bC/JtR+zRQ5RcAzMY9kHyCIxkAzMc9lFrH8sPYF5Pf3/1qJrdjESACRIAIEAEiQAR8PwEvxjufVhc6kdsAAAAASUVORK5CYII=') no-repeat scroll 0 0 / 100% 100%;
	}
	
	#canvas {
		width: 1.5in;
		height: 1.5in;
	}
		
</style>

<script language="javascript" type="text/javascript">

	var LANG = "{LANG}";

	var canvas = null;
	var percent = 0;
	
	var lang = {
		"RU": {
			prayerWheel: "Молитвенный барабан",
		},
		
		"EN": {
			prayerWheel: "Prayer wheel",
		},
		
		get: function(lang, id) {
			if (this[lang] === undefined) {
				lang = "EN";
			}
			return this[lang][id];
		}
	}
	
	function setPercent(value) {
		percent = parseInt(value);
	}
	
	function onFrame() {
		var context = canvas.getContext('2d');
		
		context.clearRect(0, 0, canvas.width, canvas.height);
		context.save();
		
		var now = new Date().getTime();
		for (var i = 0; i < 24; i++) {
			context.globalAlpha = ((now + (i * 1000 / 24)) % 1000) / 1000;
			var x = Math.sin(i * Math.PI / 12) * 100;
			var y = Math.cos(i * Math.PI / 12) * 100;
			context.beginPath();
			if ((i % 2) == 0) {
				context.arc(Math.floor(canvas.width / 2) + x, Math.floor(canvas.height / 2) + y, 8, 0, 2 * Math.PI, false);
			} else {
				context.arc(Math.floor(canvas.width / 2) + x, Math.floor(canvas.height / 2) + y, 3, 0, 2 * Math.PI, false);
			}
			context.fillStyle = '#FFFFFF';
			context.fill();
		}
		context.restore();

		context.fillStyle = '#FFFFFF';
		context.font = 'bold 50px Arial';
		context.textAlign = 'center';
		context.textBaseline = 'middle';
		context.fillText(percent.toFixed(0) + '%', Math.floor(canvas.width / 2), Math.floor(canvas.height / 2));
		
		requestAnimFrame(onFrame);
	}

	function unlimitFrame(callback) {
		window.setTimeout(callback, 1000 / 30);
	}

	window.requestAnimFrame = ( function(callback){
		return window.requestAnimationFrame ||
			window.webkitRequestAnimationFrame ||
			window.mozRequestAnimationFrame ||
			window.oRequestAnimationFrame ||
			window.msRequestAnimationFrame ||
			unlimitFrame;
	} )();
	
	window.onload = function() {
		canvas = document.getElementById('canvas');
		
		var navi = document.getElementById('navigation');
		navi.innerHTML = lang.get(LANG, 'prayerWheel');
		
		requestAnimFrame(onFrame);
	}

    function onClose() {
        window.location = "ios:close";
    }

</script>

</head>
<body>
	<table cellspacing=0 cellpadding=0 border=0>
		<tr class="navigation">
			<td>
				<center id="navigation">Молитвенный барабан</center>
				<div class="arrow-left" onclick="onClose();"></div>
			</td>
		</tr>
		<tr style="height: 100%; background: #111;">
			<td>
				<center><canvas id="canvas" width="300px" height="300px"></canvas></center>
			</td>
		</tr>
	</table>
</body>
)JAPPSYRAWSTRING"