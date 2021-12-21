from pynput.keyboard import Key, Listener
import serial

keymap = {
    '\'a\'' : 'released',
    '\'w\'' : 'released',
    '\'s\'' : 'released',
    '\'e\'' : 'released',
    '\'d\'' : 'released',
    '\'f\'' : 'released',
    '\'t\'' : 'released',
    '\'g\'' : 'released',
    '\'y\'' : 'released',
    '\'h\'' : 'released',
    '\'u\'' : 'released',
    '\'j\'' : 'released',
}   

def on_press(key):
    if keymap.get(str(key)) == 'released':
        print("\nPRESSED: " + str(key))
        keymap[str(key)] = 'pressed'
        ser.write(b'p');
        char = str(key).encode('utf_8');
        print(char);
        ser.write(char);


def on_release(key):
    if keymap.get(str(key)) == 'pressed':
        print("\nRELEASED: " + str(key))
        keymap[str(key)] = 'released'
        ser.write(b'r');
        ser.write(str(key).encode('utf_8'));

ser = serial.Serial('/dev/ttyUSB0', 115200)

# Collect events until released
with Listener(
        on_press=on_press,
        on_release=on_release) as listener:
    listener.join()
