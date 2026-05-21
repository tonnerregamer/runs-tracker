# Runs Tracker

<img src="logo.png" width="150" alt="Runs Tracker Logo" />

Runs Tracker is a Geometry Dash utility built with Geode that records attempts in Normal/Practice modes and calculates optimal segment sequences.

## Features
* **Absolute Progress**: Bypasses native relative Start Pos tracking to ensure accurate percentage recording.
* **Optimal Sequence**: Sorts attempts and finds the theoretical minimum runs needed to cover the level.
* **Editor Integration**: Link online levels to local copies for unified, merged tracking.
* **Paginated Popups**: Clean, lag-free UI limiting local levels to 6 per page (newest first).

## Installation
Install directly via the in-game **Geode Mod Index** by searching for **"Runs Tracker"**, or manually drop the `.geode` file from the [Releases](https://github.com/tonnerregamer/runs-tracker/releases) page into your `geode/mods` folder.

## Building
Requires [Geode CLI](https://github.com/geode-sdk/cli):

```sh
geode package
geode build
```

# Resources
* [Geode SDK Documentation](https://docs.geode-sdk.org/)
* [Geode SDK Source Code](https://github.com/geode-sdk/geode/)
* [Geode CLI](https://github.com/geode-sdk/cli)
