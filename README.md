# Thor Janitor

<p align="center">
  <img width="600" height="auto" src="https://github.com/thorvg/thorvg.janitor/blob/main/title.png">
</p>

**“Clean the Galaxy, One Sweep at a Time!"**

By 2080, Earth's orbit has become a cosmic graveyard, a massive junkyard of space debris threatening the survival of humanity's spacefaring future. This is where you come in: the brave (if slightly underpaid) space janitor. Your enemies aren't fearsome alien warriors—they're just mountains of cosmic trash!<br />
<br />
You pilot a Thor Cleaning Ship, sweeping the orbit clean by blasting away these junk invaders. The more you clean, the safer and shinier Earth becomes. With your trusty ship, you'll blast through debris, protect humanity's future, and prove that even trash duty can make you a hero.<br />

<p align="center">
  <img width="800" height="auto" src="https://github.com/user-attachments/assets/8a4bd16a-bb72-4b41-b007-eadc2220d1eb"/>
</p>

<p align="center">
  <strong><a href="https://youtu.be/jdnnzmtHy9k">Watch the full video!</a></strong>
</p>

## Build & Run

### Native Build (Desktop)
This program is built on the ThorVG engine. Please refer to the [ThorVG repo](https://github.com/thorvg/thorvg) to install it first. The recommended build option is
```
-Dloaders="svg,ttf,jpg"
```
Afer that, try build and execute the ThorVG Janitor!
```
$ make
$ tvggame
```

### Web Build (WASM)
To build and run the game in a web browser:

1. **Setup Emscripten SDK** (if not already installed):
We use Emscripten to compile. Please refer to the [ThorVG wiki](https://github.com/thorvg/thorvg/wiki/Web-Development#1-setup-emscripten) to setup Emscripten SDK.


2. **Install WASM Static Library**
To use ThorVG Janitor in the web environment, you need a static library of ThorVG built for WebAssembly. Follow the steps in [Generate WebAssembly Code](https://github.com/thorvg/thorvg/wiki/Web-Development#2-build-thorvg) to compile ThorVG as a WASM static library. Once the build is complete, make sure to install the generated WASM static library before building the game.

```bash
$ ninja -C build_wasm install
```

2. **Build the game**:
```bash
export EMSDK=~/path/to/emsdk
./build_wasm.sh
```

3. **Run a local server**:
```bash
emrun tvggame.html
```

## Key Instruction

* **Arrow Key**: Movement
* **A** : Shoot
* **Esc** : Exit

## Combo System

You earn cleaning points for every piece of space junk you clear away.
Sweep away the same type of trash consecutively to trigger combo bonuses, multiplying your score for an even shinier cleanup!

## Features

- Designed as a demo app to showcase the performance of the ThorVG engine.
- Each enemy is composed of 86 particles, with up to ~300 enemies appearing on screen simultaneously.
- Includes a full-size background image (subtle halo glow effect, not distracting) and 4 layers of 100 star objects.
- At peak load, around 25,000 paint objects are rendered together.
- The player’s ship, missiles, and GUI texts feature real-time DropShadow effects, while zone outlines include real-time BlurEffects.
- Runs fully stable at 120+ FPS with the Software Renderer on 2K resolution.

## Authors

* **[Hermet Park](https://github.com/hermet)**
