R"JAPPSYRAWSTRING(
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

    text_sequence: [ "text_double", [216, 178+152/2], 2 ],

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

    dbl_sun: [ "sunmoon", [960 - 520, 606], 0 ],
    dbl_moon: [ "sunmoon", [960 + 520, 606], 1 ],
    question: [ "question", [960, 606], 0 ],
)JAPPSYRAWSTRING"