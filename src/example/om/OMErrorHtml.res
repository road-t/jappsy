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
        background-color: rgba(0,0,0,0.5);
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
		background: transparent url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAIAAAACACAMAAAGD56FvAAAACXBIWXMAAAsTAAALEwEAmpwYAAABNmlDQ1BQaG90b3Nob3AgSUNDIHByb2ZpbGUAAHjarY6xSsNQFEDPi6LiUCsEcXB4kygotupgxqQtRRCs1SHJ1qShSmkSXl7VfoSjWwcXd7/AyVFwUPwC/0Bx6uAQIYODCJ7p3MPlcsGo2HWnYZRhEGvVbjrS9Xw5+8QMUwDQCbPUbrUOAOIkjvjB5ysC4HnTrjsN/sZ8mCoNTIDtbpSFICpA/0KnGsQYMIN+qkHcAaY6addAPAClXu4vQCnI/Q0oKdfzQXwAZs/1fDDmADPIfQUwdXSpAWpJOlJnvVMtq5ZlSbubBJE8HmU6GmRyPw4TlSaqo6MukP8HwGK+2G46cq1qWXvr/DOu58vc3o8QgFh6LFpBOFTn3yqMnd/n4sZ4GQ5vYXpStN0ruNmAheuirVahvAX34y/Axk/96FpPYgAAACBjSFJNAAB6JQAAgIMAAPn/AACA6AAAUggAARVYAAA6lwAAF2/XWh+QAAAAeFBMVEX/zAD/////zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAD/zAB/UgzsAAAAJ3RSTlMAABIeISQnKi0wMzY5PD9CRUhLVFdahJmcn8PP0tjh5Ofq7fDz9vkTwM98AAACUUlEQVR42u2YW0PjIBCFj2va2Etqq7uuq67WtOn5///QB7QCacAAIV6GRzLzzYEMwwVnVkO6DgC8NztIGB0kjA4ScFu0GQAfUksfbD6sDi7NDq5hdPAGsCwu4WHYUdKO5dMbAABI/nIYkFwBnQYkN0CMAUCychq4RX6JqRaDpEk7J50GM7oNLvhWGk8blDwW05MGE76X31MGhfrebfD6PYKAqUeDfxT+efDPpPtfSNJGGAhAAFEAaG1hr/R+gDlJchsKmJEkuQxUUCr3yip3HwVMlfuqVXE/Bpgo983rWPoCzpX71XEu+wIKM37EENbBgOhJTPAbEyRSglQOXkxSkQQgAAEIQAAC6NxbUZO3MYAdrStrT0DNOEDtPF/4Ac8kyR1CAcp/j1CA8m8QCtge/cMAmn8Q4EnzDwEY/gEA5X9AKODR9O8NuFNHZAQD7BEEz0ETDoj+C/F5EJ+J+lqKW437cEB0PYivSPE1Mb4qx+8LQE3+k+1dAAIQwA8AiAARIAJEgAgQASJABPw4Aehsxe3/5u4PMJKAxU698fB6FAHzt/BkM4KAWc339je7ACP8bpE7By6etfD7JTInYWmEr5xvYwMIKLda+KbyPA8mFzA1wq+8L6SJBUyf9PCXQF4BEyP82siLDAIKPfxhY62LwQUUj3r4q1ZdGFjAuSf84AKKsQX4MiBLEpprYJNfgF0F1vkFdNfBjKW4HLcUt/bC/JtR+zRQ5RcAzMY9kHyCIxkAzMc9lFrH8sPYF5Pf3/1qJrdjESACRIAIEAEiQAR8PwEvxjufVhc6kdsAAAAASUVORK5CYII=') no-repeat scroll 0 0 / 100% 100%;
	}

    .error {
        background-color: rgba(255,0,0,0.5);
        width: 100%;
        height: 1in;
    }

    .error td {
        width: 100%;
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
				<center id="navigation">Молитвенный барабан asdasd</center>
				<div class="arrow-left" onclick="onClose();"></div>
			</td>
		</tr>
		<tr style="height: 100%;">
			<td>
				<table class="error" cellspacing=0 cellpadding=0 border=0><tr><td id="error"><center>Ошибка подключения</center>AASD</td></tr></table>
			</td>
		</tr>
	</table>
</body>
)JAPPSYRAWSTRING"
