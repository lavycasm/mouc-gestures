# mouc-gestures
![preview](https://github.com/lavycasm/mouc-gestures/blob/main/preview.gif)  

a mouse gesture recognition program written in c and working alongside ahk  
works by reading the contents of config.ini and executing a temporary ahk  
script when a gesture combo is matched.

## Usage

Download [AutoHotkey](https://www.autohotkey.com)

Holding MMB - starts a gesture  
Holding MMB and pressing 'r' - reloads config 


## Building
```
gcc main.c ini.c -o main.exe
main.exe
```

## Credits
[inih](https://github.com/benhoyt/inih) - processing ini files



