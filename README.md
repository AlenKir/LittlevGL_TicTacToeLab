# LittlevGL_TicTacToeLab
Working with LittlevGL - a graphic library - to create a simple tic tac toe game for STM32.
(Ideally for STM32 but - COVID-19 and no access to it, so just in Eclipse.)

## Used this instructions and documentation

MSYS2: https://www.msys2.org/
SDL2.dll: google the name and download from some dll website
To test msys2 from command line: https://gist.github.com/thales17/fb2e4cff60890a51d9dddd4c6e832ad2
And LittlevGL documentation: https://docs.littlevgl.com/en/html/index.html

### Eclipse project properties

- C/C++ Build - > Enviroment
check MINGWHOME

- C/C++ General -> Path and Symbols
Includes: the path(s) to your .h files
Symbols: LV_CONF_INCLUDE_SIMPLE
Libraries (the order is important): mingw32 (yes, even if you are using mingw64, apparently), SDL2_main, SDL2
Sourse location: don't forget lvgl/src
Root dir of my project looks like
$ ls
Debug/  lv_conf.h  lv_ex_conf.h  lvgl/  SDL2.dll*  src/

## Notes
The code is far from clean and has some duplicate code that it definitely could do without. 
I hope I will fix at least some of it after the lab gets accepted.
