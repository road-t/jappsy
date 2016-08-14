Jappsy.__LANG__ = "en";

Jappsy.lang = function(type, name) {
	return Jappsy.lang[Jappsy.__LANG__][type][name];
}

Jappsy.lang.en = {
	label: {
		back: 'Back',
	},

	hint: {
		loading: 'Loading...',
	}
}

Jappsy.lang.ru = {	
	label: {
		back: 'Назад',
	},
	
	hint: {
		loading: 'Загрузка...',
	}
}
