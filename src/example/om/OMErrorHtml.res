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
        background-color: rgba(0,0,0,0.8);
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
		background: transparent url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAYAAAG0OVFdAAAACXBIWXMAAAsTAAALEwEAmpwYAAABNmlDQ1BQaG90b3Nob3AgSUNDIHByb2ZpbGUAAHjarY6xSsNQFEDPi6LiUCsEcXB4kygotupgxqQtRRCs1SHJ1qShSmkSXl7VfoSjWwcXd7/AyVFwUPwC/0Bx6uAQIYODCJ7p3MPlcsGo2HWnYZRhEGvVbjrS9Xw5+8QMUwDQCbPUbrUOAOIkjvjB5ysC4HnTrjsN/sZ8mCoNTIDtbpSFICpA/0KnGsQYMIN+qkHcAaY6addAPAClXu4vQCnI/Q0oKdfzQXwAZs/1fDDmADPIfQUwdXSpAWpJOlJnvVMtq5ZlSbubBJE8HmU6GmRyPw4TlSaqo6MukP8HwGK+2G46cq1qWXvr/DOu58vc3o8QgFh6LFpBOFTn3yqMnd/n4sZ4GQ5vYXpStN0ruNmAheuirVahvAX34y/Axk/96FpPYgAAACBjSFJNAAB6JQAAgIMAAPn/AACA6AAAUggAARVYAAA6lwAAF2/XWh+QAAACXElEQVR42mL6//8/AyWYiYFCMGoALgPOMv7HiYkywPg/IwMDAyo2/s8IFSfSBQwMqHgIugAXHk2JGAAAAAD//2IaLQ+GXHlAtAG4ciNJLsCWG4eYC4goC0ZTIgQAAAAA//+iuDwY8PJkwINw1AEj0AFnGfHj0SgY/g4w/o8fj0bBqAOGmgMAAAAA///s2DEKACAQA8H///pK4YoQ8MoR7KfYQuM9AAAAAADQA9KSWS4af4D3y883rAEauGlgrzfluqsBDdw10F4NAAAAAJRnAAAA///s2ckJACAMBMD+u7YAHx5BomaEbWAwBDbp/UD5fqL8FwQAAAAAAAAAAADwDcBqj7yTqwH6fiqWQOdlBAAAAAAgfQ2Ozimz55Zn1+CJGAEAAAAAAAAAAAAAAKKvAQAA///s27ENADAIA7D/v+4DTFUGIswJVhAdUv0AKwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAD8TVlPWALiAHOjK1uWlgAAAAAAaHoHpMvSzqAVAAAAAABnUAJq3gGLP0xJAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABcmwcAAP//7NzRCQAhDETB/ru2BcGsIWQGroOHh37s+v0A+wk4AhEAAkAACAABIAAEgAAQAAJAAAgAASAABIAAEAACQAAIAAEggJAfe8QDN4/3BHC3n9T7NWw8+QUgAASAABAAy6+Br7u81bu+roE4AbwDeAfACYAAEAACQADeAQQQugZO+ASAABAAAkAACAABIAAEgAAQAAJAAAgAASAABIAAEAACQAAIAAGQcQAAAP//AwCi7CgqtTYIHwAAAABJRU5ErkJggg==') no-repeat scroll 0 0 / 100% 100%;
	}

    .error {
        background-color: rgba(255,0,0,0.5);
        width: 100%;
        height: 1in;
    }

</style>

<script language="javascript" type="text/javascript">

	var LANG = "{LANG}";

	var canvas = null;
	var percent = 0;
	
	var lang = {
		"RU": {
			prayerWheel: "Молитвенный барабан",
            connectionError: "Ошибка подключения",
		},
		
		"EN": {
			prayerWheel: "Prayer wheel",
            connectionError: "Connection error",
		},

		get: function(lang, id) {
			if (this[lang] === undefined) {
				lang = "EN";
			}
			return this[lang][id];
		}
	}
	
	window.onload = function() {
		var navi = document.getElementById('navigation');
		navi.innerHTML = lang.get(LANG, 'prayerWheel');

        var error = document.getElementById('error');
        error.innerHTML = lang.get(LANG, 'connectionError');
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
		<tr style="height: 100%;">
			<td>
				<table class="error" cellspacing=0 cellpadding=0 border=0><tr><td><center id="error">Ошибка подключения</center></td></tr></table>
			</td>
		</tr>
	</table>
</body>
)JAPPSYRAWSTRING"
