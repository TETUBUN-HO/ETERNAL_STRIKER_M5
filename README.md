ETERNAL STRIKER Rr M5
T.KATAKOTO (TETUBUN-HO) TEAM Redherring(STUDIO Sequence)

for M5 Stack (Not CORE2)

![TITLE](https://www.team-rh.com/wp-content/uploads/2019/05/D45o0SkU4AEMAmt.png)![GAME SCREEN](https://www.team-rh.com/wp-content/uploads/2019/05/D45o0SmUUAA67y8.png)

*Overview

-Works that ported STG(shooting game) [ETERNAL STRIKER Rr], which was operating in MIDP environment.

-SPIFFS is not used for anything other than score saving.

-For [M5Stack-SD-Menu] system.

+[M5Stack-SD-Menu] > Copyright 2018 tobozo http://github.com/tobozo

 Released under the MIT license
 
 It can be disabled by commenting out [#define SDC_SYSTEM_ON].

I adjust the vertical STG to force only horizontal movement, but personally I prefer to make it under such restrictions.

As long as there is a time-recovery barrier, [ENE], we have decided that the system is safe to be hit.
You can also use the [B] button or [A] and [C] simultaneously to consume a little [ENE] and enter the invincible mode spontaneously.
By pressing the [A] and [C] buttons at the same time on the title screen, you can switch the LED off and on.
You can mute the audio by pressing the [C] button at startup.

-This is for button operation models such as NORMAL and FIRE.

-Please refer to here for the touch panel and vibration compatible version for M5Stack CORE2.

https://github.com/TETUBUN-HO/ETERNAL_STRIKER_M5_C2/

+It corresponds to the stick of [M5 unit]. Connect with port A (I2C (G21 / G22)).

+It also supports the dual button of [M5 unit]. Connect to port B (G26 / G36).

-In the bonus, corresponding to the light emission of [GO BOTTOM]’s NEOPIXEL.

 Uses NeoPixelBus library.

https://github.com/Makuna/NeoPixelBus

It can be disabled by commenting out [#define NEOPIXEL_ON].
 By pressing the [A] and [C] buttons at the same time on the title screen, you can switch the LED off and on.
 However, at present, the malfunction of the 0 light is confirmed.
 under investigation.
 
＊説明

ガラケー前期、Doja2.1-3.0、あるいはMIDP時代に作った作品 [ETERNAL STRIKER Rr]の移植作です。
SPIFFSは、スコアセーブ以外に使っていません。

元ゲーの最小構成版は、リソース含めて総計30kbとかで動いていました(一応、有料配信していたタイトルです)。
縦STGを、無理やり横移動だけに調整していますが、こういう制限の元に作るのも個人的には好みです。
時間回復する防壁、[ENE]が有るうちは、被弾しても大丈夫なシステムとしました。
なお、[B]ボタンあるいは[A][C]同時押しで、[ENE]を少し消費して、自発的に無敵モードに入る事も出来ます。

こちらは、無印版、FIREなど、ボタン操作機種用となります。

M5Stack CORE2用の、タッチパネルと振動に対応した版は此方を参照ください。

https://github.com/TETUBUN-HO/ETERNAL_STRIKER_M5_C2/


[M5Stack-SD-Menu] システムに対応しています。

Copyright 2018 tobozo http://github.com/tobozo
Released under the MIT license

[ #define SDC_SYSTEM_ON ]のコメントアウトで不使用化も可能です。

[M5 unit]のスティックに対応しています。ポートＡ（I2C(G21/G22)）で接続してください。

[M5 unit]のデュアルボタンにも対応しています。ポートＢ（G26/G36)に接続してください。

[GO BOTTOM]のNEOPIXELにも対応しています。タイトル画面で[A] [C]ボタンを同時押しする事で消灯する事ができます。

NeoPixelBusライブラリを使用。

https://github.com/Makuna/NeoPixelBus

[ #define NEOPIXEL_ON ]のコメントアウトで不使用化も可能です。

プレイ動画 / PLAYMOVIE

http://www.team-rh.com/wp-content/uploads/2019/05/D5CeQYKU0AE2ylZ.mp4

![M5STACK+UNIT](https://www.team-rh.com/wp-content/uploads/2019/03/D2ZCS7-U8AAhDyx-large-300x300.jpg) ![M5STACK](https://www.team-rh.com/wp-content/uploads/2018/10/DqFidU2UwAA3hu7-large-300x300.jpg)


